#pragma once

#include <sph-simulation/data_types/vertex.hpp>
#include <sph-simulation/render/core/index_buffer.hpp>
#include <sph-simulation/render/core/vertex_buffer.hpp>

#include <glm/mat4x4.hpp>

#include <tiny_obj_loader.h>

#include <filesystem>

struct renderable
{
   vertex_buffer vertex_buff;
   index_buffer index_buff;

   glm::mat4 model{};
};

inline auto create_renderable(const cacao::device& device, const cacao::command_pool& pool,
                              const renderable_data& data, mannele::log_ptr logger)
   -> renderable
{
   return renderable{
      .vertex_buff = vertex_buffer(
         {.device = device, .pool = pool, .vertices = data.vertices, .logger = logger}),
      .index_buff = index_buffer(
         {.device = device, .pool = pool, .indices = data.indices, .logger = logger}),
      .model = data.model};
}

inline auto load_obj(const std::filesystem::path& path) -> renderable_data
{
   tinyobj::attrib_t attrib;

   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;

   std::string err;
   if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str()))
   {
      throw std::runtime_error(err);
   }

   std::unordered_map<vertex, std::uint32_t> unique_vertices{};

   std::vector<vertex> vertices;
   std::vector<std::uint32_t> indices;

   for (const auto& shape : shapes)
   {
      for (const auto& index : shape.mesh.indices)
      {
         vertex vertex{
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

