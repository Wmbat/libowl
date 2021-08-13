#pragma once

#include <sph-simulation/render/render_system.hpp>

#include <libcacao/gfx/data_types.hpp>
#include <libcacao/index_buffer.hpp>
#include <libcacao/vertex_buffer.hpp>

#include <glm/mat4x4.hpp>

#include <tiny_obj_loader.h>

#include <filesystem>

struct renderable
{
   cacao::vertex_buffer m_vertex_buffer;
   cacao::index_buffer m_index_buffer;

   glm::mat4 m_model{};
};

inline auto create_renderable(const render_system& system, const cacao::renderable_data& data)
   -> renderable
{
   return renderable{.m_vertex_buffer = system.create_vertex_buffer(data.vertices),
                     .m_index_buffer = system.create_index_buffer(data.indices),
                     .m_model = data.model};
}

inline auto load_obj(const std::filesystem::path& path) -> cacao::renderable_data
{
   tinyobj::attrib_t attrib;

   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;

   std::string warn;
   std::string err;

   if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
   {
      throw std::runtime_error(warn + err);
   }

   std::unordered_map<cacao::vertex, std::uint32_t> unique_vertices{};

   std::vector<cacao::vertex> vertices;
   std::vector<std::uint32_t> indices;

   for (const auto& shape : shapes)
   {
      for (const auto& index : shape.mesh.indices)
      {
         cacao::vertex vertex{
            .position = {attrib.vertices[3u * static_cast<std::size_t>(index.vertex_index) + 0u],
                         attrib.vertices[3u * static_cast<std::size_t>(index.vertex_index) + 1u],
                         attrib.vertices[3u * static_cast<std::size_t>(index.vertex_index) + 2u]},
            .normal = {attrib.normals[3u * static_cast<std::size_t>(index.normal_index) + 0u],
                       attrib.normals[3u * static_cast<std::size_t>(index.normal_index) + 1u],
                       attrib.normals[3u * static_cast<std::size_t>(index.normal_index) + 2u]},
            .colour = {1.0F, 1.0F, 1.0F}};

         if (unique_vertices.count(vertex) == 0)
         {
            unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
         }

         indices.push_back(unique_vertices[vertex]);
      }
   }

   return {.vertices = vertices, .indices = indices, .model = glm::mat4{1}};
}

