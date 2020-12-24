#pragma once

#include <fluid_simulation/render/render_system.hpp>

#include <vermillon/gfx/data_types.hpp>
#include <vermillon/gfx/index_buffer.hpp>
#include <vermillon/gfx/vertex_buffer.hpp>

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
   auto vertices_res = system.create_vertex_buffer(data.vertices);
   auto indices_res = system.create_index_buffer(data.indices);

   if (!(vertices_res && indices_res))
   {
      std::exit(EXIT_FAILURE);
   }

   renderable r;
   r.m_vertex_buffer = std::move(vertices_res).value().value();
   r.m_index_buffer = std::move(indices_res).value().value();
   r.m_model = data.model;

   return r;
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

   crl::dynamic_array<cacao::vertex> vertices;
   crl::dynamic_array<std::uint32_t> indices;

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
            vertices.append(vertex);
         }

         indices.append(unique_vertices[vertex]);
      }
   }

   return {.vertices = vertices, .indices = indices, .model = glm::mat4{1}};
}

