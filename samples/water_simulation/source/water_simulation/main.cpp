#include <water_simulation/particle_system.hpp>
#include <water_simulation/shader_codex.hpp>

#include <gfx/data_types.hpp>
#include <gfx/render_manager.hpp>
#include <gfx/window.hpp>

#include <ui/window.hpp>

#include <glm/ext/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

static constexpr float time_step = 0.0001F;
[[maybe_unused]] static constexpr float gravity = -9.81F;

auto load_obj(const filepath& path) -> gfx::renderable_data
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

   std::unordered_map<gfx::vertex, std::uint32_t> unique_vertices{};

   util::dynamic_array<gfx::vertex> vertices;
   util::dynamic_array<std::uint32_t> indices;

   for (const auto& shape : shapes)
   {
      for (const auto& index : shape.mesh.indices)
      {
         gfx::vertex vertex{.position = {attrib.vertices[3 * index.vertex_index + 0],
                                         attrib.vertices[3 * index.vertex_index + 1],
                                         attrib.vertices[3 * index.vertex_index + 2]},
                            .normal = {attrib.normals[3 * index.normal_index + 0],
                                       attrib.normals[3 * index.normal_index + 1],
                                       attrib.normals[3 * index.normal_index + 2]},
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

void integrate([[maybe_unused]] particle& p)
{
   p.velocity = p.velocity + time_step * p.force / p.mass;
   p.position = p.position + time_step * p.velocity;
};

auto main() -> int
{
   auto main_logger = std::make_shared<util::logger>("water_simulation");

   glfwInit();

   ui::window window{"Water Simulation", 1080, 720};
   gfx::render_manager rendering_manager{window, main_logger};

   shader_codex codex{rendering_manager, main_logger};
   auto vert_res = codex.insert("resources/shaders/test_vert.spv", vkn::shader_type::vertex);
   auto frag_res = codex.insert("resources/shaders/test_frag.spv", vkn::shader_type::fragment);

   if (auto err = vert_res.error())
   {
      util::log_error(main_logger, "error: {}", err->value().message());
   }

   if (auto err = frag_res.error())
   {
      util::log_error(main_logger, "error: {}", err->value().message());
   }

   const auto vert_shader_info = vert_res.value().value();
   const auto frag_shader_info = frag_res.value().value();

   rendering_manager.bake(vert_shader_info.value(), frag_shader_info.value());

   rendering_manager.subscribe_renderable("sphere", load_obj("resources/meshes/sphere.obj"));

   rendering_manager.update_model_matrix("sphere", glm::scale(glm::mat4{1}, {0.5F, 0.5F, 0.5F}));

   while (window.is_open())
   {
      window.poll_events();

      rendering_manager.render_frame();
   }

   rendering_manager.wait();

   return 0;
}
