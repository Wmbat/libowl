#include <water_simulation/particle_system.hpp>
#include <water_simulation/pipeline_codex.hpp>
#include <water_simulation/render_system.hpp>
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

template <typename Any>
auto handle_error(Any&& result, const std::shared_ptr<util::logger>& p_logger)
{
   if (auto err = result.error())
   {
      util::log_error(p_logger, "error: {}", err->value().message());

      std::exit(EXIT_FAILURE);
   }

   return std::forward<Any>(result).value().value();
}

auto main() -> int
{
   auto main_logger = std::make_shared<util::logger>("water_simulation");

   glfwInit();

   ui::window window{"Water Simulation", 1080, 720};

   render_system renderer = handle_error(
      render_system::make({.p_logger = main_logger, .p_window = &window}), main_logger);

   shader_codex shader_codex{renderer, main_logger};
   pipeline_codex pipeline_codex{renderer, main_logger};

   const auto vert_shader_info =
      handle_error(shader_codex.insert("resources/shaders/test_vert.spv", vkn::shader_type::vertex),
                   main_logger);
   const auto frag_shader_info = handle_error(
      shader_codex.insert("resources/shaders/test_frag.spv", vkn::shader_type::fragment),
      main_logger);

   while (window.is_open())
   {
      window.poll_events();

      renderer.begin_frame();

      renderer.record_draw_calls([&](vk::CommandBuffer /*buffer*/) {
         /*
      buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline.value());
      buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline.layout(),
                                0, {m_camera_descriptor_pool.sets()[image_index]}, {});

      for (const auto& [name, index] : m_renderables_to_index)
      {
         util::log_debug(mp_logger, R"([gfx] buffer calls for renderable "{}" at index "{}")",
                         name, index);

         buffer.pushConstants(
            m_graphics_pipeline.layout(),
            m_graphics_pipeline.get_push_constant_ranges("mesh_data").stageFlags, 0,
            sizeof(glm::mat4) * 1, &m_renderable_model_matrices[index]);
         buffer.bindVertexBuffers(0, {m_renderables[index].vertex_buffer->value()},
                                  {vk::DeviceSize{0}});
         buffer.bindIndexBuffer(m_renderables[index].index_buffer->value(), 0,
                                vk::IndexType::eUint32);
         buffer.drawIndexed(m_renderables[index].index_buffer.index_count(), 1, 0, 0, 0);
      }
      */
      });

      renderer.end_frame();
   }

   renderer.wait();

   return 0;
}

/*
rendering_manager.bake(vert_shader_info.value(), frag_shader_info.value());

rendering_manager.subscribe_renderable("sphere", load_obj("resources/meshes/sphere.obj"));

rendering_manager.update_model_matrix("sphere", glm::scale(glm::mat4{1}, {0.5F, 0.5F, 0.5F}));

while (window.is_open())
{
   window.poll_events();

   rendering_manager.render_frame([](auto) {});
}

rendering_manager.wait();
*/
