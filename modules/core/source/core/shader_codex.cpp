#include "core/shader_codex.hpp"

#include <monads/try.hpp>

#include <util/logger.hpp>

#include <mpark/patterns/match.hpp>

#include <cstring>
#include <fstream>
#include <functional>
#include <iterator>

namespace fs = std::filesystem;

namespace core
{
   namespace detail
   {
      auto to_string(shader_codex::error_type err) -> std::string
      {
         using error = shader_codex::error_type;
         using namespace mpark::patterns;

         // clang-format off
         return match(err) (
            pattern(error::failed_to_open_file) = [] { return "failed_to_open_file"; },
            pattern(error::unknow_shader_type) = [] { return "unknow_shader_type"; },
            pattern(error::failed_to_preprocess_shader) = [] { 
               return "failed_to_preprocess_shader";
            },
            pattern(error::failed_to_parse_shader) = [] { return "failed_to_parse_shader"; },
            pattern(error::failed_to_link_shader) = [] { return "failed_to_link_shader"; },
            pattern(error::failed_to_cache_shader) = [] { return "failed_to_cache_shader"; },
            pattern(_) = [] { return "unknown_error"; }
         );
         // clang-format on
      };

      // clang-format off
      const TBuiltInResource default_built_in_resource
      {
         .maxLights = 32,
         .maxClipPlanes = 6,
         .maxTextureUnits = 32,
         .maxTextureCoords = 32,
         .maxVertexAttribs = 64,
         .maxVertexUniformComponents = 4096,
         .maxVaryingFloats = 64,
         .maxVertexTextureImageUnits = 32,
         .maxCombinedTextureImageUnits = 80,
         .maxTextureImageUnits = 32,
         .maxFragmentUniformComponents = 4096,
         .maxDrawBuffers = 32,
         .maxVertexUniformVectors = 128,
         .maxVaryingVectors = 8,
         .maxFragmentUniformVectors = 16,
         .maxVertexOutputVectors = 16,
         .maxFragmentInputVectors = 15,
         .minProgramTexelOffset = -8,
         .maxProgramTexelOffset = 7,
         .maxClipDistances = 8,
         .maxComputeWorkGroupCountX = 65535,
         .maxComputeWorkGroupCountY = 65535,
         .maxComputeWorkGroupCountZ = 65535,
         .maxComputeWorkGroupSizeX = 1024,
         .maxComputeWorkGroupSizeY = 1024,
         .maxComputeWorkGroupSizeZ = 64,
         .maxComputeUniformComponents = 1024,
         .maxComputeTextureImageUnits = 16,
         .maxComputeImageUniforms = 8,
         .maxComputeAtomicCounters = 8,
         .maxComputeAtomicCounterBuffers = 1,
         .maxVaryingComponents = 60,
         .maxVertexOutputComponents = 64,
         .maxGeometryInputComponents = 64,
         .maxGeometryOutputComponents = 128,
         .maxFragmentInputComponents = 128,
         .maxImageUnits = 8,
         .maxCombinedImageUnitsAndFragmentOutputs = 8,
         .maxCombinedShaderOutputResources = 8,
         .maxImageSamples = 0,
         .maxVertexImageUniforms = 0,
         .maxTessControlImageUniforms = 0,
         .maxTessEvaluationImageUniforms = 0,
         .maxGeometryImageUniforms = 0,
         .maxFragmentImageUniforms = 8,
         .maxCombinedImageUniforms = 8,
         .maxGeometryTextureImageUnits = 16,
         .maxGeometryOutputVertices = 256,
         .maxGeometryTotalOutputComponents = 1024,
         .maxGeometryUniformComponents = 1024,
         .maxGeometryVaryingComponents = 64,
         .maxTessControlInputComponents = 128,
         .maxTessControlOutputComponents = 128,
         .maxTessControlTextureImageUnits = 16,
         .maxTessControlUniformComponents = 1024,
         .maxTessControlTotalOutputComponents = 4096,
         .maxTessEvaluationInputComponents = 128,
         .maxTessEvaluationOutputComponents = 128,
         .maxTessEvaluationTextureImageUnits = 16,
         .maxTessEvaluationUniformComponents = 1024,
         .maxTessPatchComponents = 120,
         .maxPatchVertices = 32,
         .maxTessGenLevel = 64,
         .maxViewports = 16,
         .maxVertexAtomicCounters = 0,
         .maxTessControlAtomicCounters = 0,
         .maxTessEvaluationAtomicCounters = 0,
         .maxGeometryAtomicCounters = 0,
         .maxFragmentAtomicCounters = 8,
         .maxCombinedAtomicCounters = 8,
         .maxAtomicCounterBindings = 1,
         .maxVertexAtomicCounterBuffers = 0,
         .maxTessControlAtomicCounterBuffers = 0,
         .maxTessEvaluationAtomicCounterBuffers = 0,
         .maxGeometryAtomicCounterBuffers = 0,
         .maxFragmentAtomicCounterBuffers = 1,
         .maxCombinedAtomicCounterBuffers = 1,
         .maxAtomicCounterBufferSize = 16384,
         .maxTransformFeedbackBuffers = 4,
         .maxTransformFeedbackInterleavedComponents = 64,
         .maxCullDistances = 8,
         .maxCombinedClipAndCullDistances = 8,
         .maxSamples = 4,
         .maxMeshOutputVerticesNV = 256,
         .maxMeshOutputPrimitivesNV = 512,
         .maxMeshWorkGroupSizeX_NV = 32,
         .maxMeshWorkGroupSizeY_NV = 1,
         .maxMeshWorkGroupSizeZ_NV = 1,
         .maxTaskWorkGroupSizeX_NV = 32,
         .maxTaskWorkGroupSizeY_NV = 1,
         .maxTaskWorkGroupSizeZ_NV = 1,
         .maxMeshViewCountNV = 4,
         .limits = {
            .nonInductiveForLoops =  true,
            .whileLoops = true,
            .doWhileLoops = true,
            .generalUniformIndexing = true,
            .generalAttributeMatrixVectorIndexing = true,
            .generalVaryingIndexing = true,
            .generalSamplerIndexing = true,
            .generalVariableIndexing = true,
            .generalConstantMatrixVectorIndexing = true
         }
      };
      // clang-format on

   } // namespace detail

   auto shader_codex::error_category::name() const noexcept -> const char*
   {
      return "shader_codex";
   }
   auto shader_codex::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<shader_codex::error_type>(err));
   }

   auto shader_codex::get_shader(const std::string& name) noexcept -> vkn::shader&
   {
      return m_shaders.at(name);
   }
   [[nodiscard]] auto shader_codex::get_shader(const std::string& name) const noexcept
      -> const vkn::shader&
   {
      return m_shaders.at(name);
   }

   using builder = shader_codex::builder;

   builder::builder(const vkn::device& device, util::logger* plogger) noexcept : m_plogger{plogger}
   {
      m_info.version = device.get_vulkan_version();
   }

   auto builder::build() -> core::result<shader_codex>
   {
      shader_codex codex{};

      if (!m_info.shader_directory_path.empty())
      {
         if (fs::exists(m_info.shader_directory_path))
         {
            if (fs::is_directory(m_info.shader_directory_path))
            {
               for (const auto& path : fs::directory_iterator(m_info.shader_directory_path))
               {
                  m_info.shader_paths.push_back(path);
               }
            }
            else
            {
               util::log_warn(m_plogger, "[core] provided path \"{0}\" is not a directory",
                              m_info.shader_directory_path.string());
            }
         }
         else
         {
            util::log_warn(m_plogger, "[core] provided path \"{0}\" does not exist",
                           m_info.shader_directory_path.string());
         }
      }
      else
      {
         util::log_info(m_plogger, "[core] no shader directory provided");
      }

      if (m_info.is_caching_allowed)
      {
         util::log_info(m_plogger, "[core] shader caching: ENABLED");
      }
      else
      {
         util::log_info(m_plogger, "[core] shader caching: DISABLED");
      }

      if (!m_info.shader_paths.empty())
      {
         for ([[maybe_unused]] const auto& path : m_info.shader_paths)
         {
            create_shader(path);
         }
      }
      else
      {
         util::log_warn(m_plogger, "[core] no shaders provided");
      }

      return monad::make_right(std::move(codex));
   }

   auto builder::set_cache_directory(const std::filesystem::path& path) -> builder&
   {
      m_info.cache_directory_path = path;
      return *this;
   }
   auto builder::set_shader_directory(const std::filesystem::path& path) -> builder&
   {
      m_info.shader_directory_path = path;
      return *this;
   }
   auto builder::add_shader_filepath(const std::filesystem::path& path) -> builder&
   {
      m_info.shader_paths.push_back(path);
      return *this;
   }
   auto builder::allow_caching(bool is_caching_allowed) noexcept -> builder&
   {
      m_info.is_caching_allowed = is_caching_allowed;
      return *this;
   }

   auto builder::create_shader(const fs::path& path) -> core::result<vkn::shader>
   {
      if (m_info.is_caching_allowed)
      {
         if (!fs::exists(m_info.cache_directory_path))
         {
            fs::create_directories(m_info.cache_directory_path);

            util::log_info(m_plogger, R"([core] no shader cache directory found. using "{0}")",
                           m_info.cache_directory_path.string());
         }

         std::hash<std::string> hasher;
         auto cache_dir = m_info.cache_directory_path;
         const fs::path hashed_path = cache_dir /= fs::path{std::to_string(hasher(path.string()))};

         bool in_cache = false;
         for (const fs::path& cache_path : fs::directory_iterator(m_info.cache_directory_path))
         {
            if (hashed_path == cache_path)
            {
               in_cache = true;
               break;
            }
         }

         if (in_cache)
         {
            const auto raw_last_write = fs::last_write_time(path);
            const auto cache_last_write = fs::last_write_time(hashed_path);

            if (raw_last_write >= cache_last_write) // NOLINT
            {
               util::log_info(m_plogger, R"([core] out-dated shader found in cache)");
               util::log_info(m_plogger, R"([core] recompiling shader: "{0}")", path.string());

               const auto compilation_result = compile_shader(path);
               if (!compilation_result.is_right())
               {
                  return monad::make_left(*compilation_result.left());
               }

               const auto shader_data = compilation_result.right().value();

               util::log_info(m_plogger, R"([core] caching shader "{0}")", path.string());

               auto cache_result = cache_shader(hashed_path, shader_data);
            }
            else
            {
               util::log_info(m_plogger, R"([core] up-to-date shader found in cache)");
               util::log_info(m_plogger, R"([core] loading shader "{0}" from cache)", path.string(),
                              hashed_path.string());

               const auto loading_result = load_shader(hashed_path);
               if (!loading_result.is_right())
               {
                  return monad::make_left(*loading_result.left());
               }
            }
         }
         else
         {
            util::log_info(m_plogger, R"([core] shader "{0}" not found in cache)", path.string(),
                           hashed_path.string());
            util::log_info(m_plogger, R"([core] compiling shader: "{0}")", path.string());

            const auto compilation_result = compile_shader(path);
            if (!compilation_result.is_right())
            {
               return monad::make_left(*compilation_result.left());
            }

            const auto shader_data = compilation_result.right().value();

            util::log_info(m_plogger, R"([core] caching shader "{0}")", path.string());

            auto cache_result = cache_shader(hashed_path, shader_data);
         }
      }
      else
      {
         util::log_info(m_plogger, "[core] compiling shader: \"{0}\"", path.string());

         compile_shader(path);
      }

      return monad::make_left(std::error_code{});
   }

   auto builder::compile_shader(const std::filesystem::path& path)
      -> core::result<util::dynamic_array<std::uint32_t>>
   {
      std::ifstream file{path};
      if (!file.is_open())
      {
         return monad::make_left(make_error_code(error_type::failed_to_open_file));
      }

      std::string spirv_version{};
      glslang::GetSpirvVersion(spirv_version);

      const std::string extension = path.extension();
      const auto shader_stage = get_shader_stage({extension.begin() + 1, extension.end()});

      if (shader_stage == EShLangCount)
      {
         return monad::make_left(make_error_code(error_type::unknow_shader_type));
      }

      using file_it = std::istreambuf_iterator<char>;
      const std::string shader_data{file_it{file}, file_it{}};

      glslang::TShader tshader{shader_stage};
      tshader.setEnvInput(glslang::EShSourceGlsl, shader_stage, glslang::EShClientVulkan,
                          client_input_semantics_version);
      tshader.setEnvClient(glslang::EShClientVulkan, get_vulkan_version(m_info.version));
      tshader.setEnvTarget(glslang::EshTargetSpv, get_spirv_version(m_info.version));

      {
         const char* shader_data_cstr = shader_data.c_str();
         tshader.setStrings(&shader_data_cstr, 1);
      }

      const auto resources = detail::default_built_in_resource;
      const auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

      DirStackFileIncluder includer;
      includer.pushExternalLocalDirectory(path.parent_path());

      std::string preprocessed_glsl;
      if (!tshader.preprocess(&resources, default_version, ENoProfile, false, false, messages,
                              &preprocessed_glsl, includer))
      {
         util::log_error(m_plogger, "[vkn] {0}", tshader.getInfoLog());

         return monad::make_left(make_error_code(error_type::failed_to_preprocess_shader));
      }

      const char* preprocessed_glsl_c_str = preprocessed_glsl.c_str();
      tshader.setStrings(&preprocessed_glsl_c_str, 1);

      if (!tshader.parse(&resources, default_version, false, messages))
      {
         util::log_error(m_plogger, "[vkn] {0}", tshader.getInfoLog());

         return monad::make_left(make_error_code(error_type::failed_to_parse_shader));
      }

      glslang::TProgram program;
      program.addShader(&tshader);

      if (!program.link(messages))
      {
         util::log_error(m_plogger, "[vkn] {0}", tshader.getInfoLog());
         util::log_error(m_plogger, "[vkn] {0}", tshader.getInfoDebugLog());

         return monad::make_left(make_error_code(error_type::failed_to_link_shader));
      }

      std::vector<uint32_t> spirv;
      spv::SpvBuildLogger logger;
      glslang::SpvOptions spv_options;
      glslang::GlslangToSpv(*program.getIntermediate(shader_stage), spirv, &logger, &spv_options);

      return monad::make_right(util::dynamic_array<uint32_t>{spirv.begin(), spirv.end()});
   }
   auto builder::load_shader(const std::filesystem::path& path)
      -> core::result<util::dynamic_array<std::uint32_t>>
   {
      std::ifstream file{path, std::ios::binary};
      if (!file.is_open())
      {
         return monad::make_left(make_error_code(error_type::failed_to_open_file));
      }

      using file_it = std::istreambuf_iterator<char>;
      const util::dynamic_array<uint8_t> raw_shader_data{file_it{file}, file_it{}};
      util::dynamic_array<std::uint32_t> data(raw_shader_data.size() / sizeof(std::uint32_t));
      std::memcpy(static_cast<void*>(data.data()), raw_shader_data.data(),
                  sizeof(std::uint32_t) * data.size());

      return monad::make_right(data);
   }

   auto builder::cache_shader(const std::filesystem::path& path,
                              const util::dynamic_array<std::uint32_t>& data) const
      -> monad::maybe<std::error_code>
   {
      std::ofstream cache_file{path, std::ios::trunc | std::ios::binary};
      if (!cache_file.is_open())
      {
         return monad::make_maybe(make_error_code(error_type::failed_to_open_file));
      }

      cache_file.write(reinterpret_cast<const char*>(data.data()), // NOLINT
                       data.size() * sizeof(std::uint32_t));
      cache_file.close();

      return monad::none;
   }

   auto builder::get_shader_stage(std::string_view stage_name) const -> EShLanguage
   {
      using namespace mpark::patterns;

      // clang-format off
      return match(stage_name)(
         pattern("vert") = [] { return EShLangVertex; },
         pattern("tesc") = [] { return EShLangTessControl; },
         pattern("tese") = [] { return EShLangTessEvaluation; },
         pattern("geom") = [] { return EShLangGeometry; },
         pattern("frag") = [] { return EShLangFragment; },
         pattern("comp") = [] { return EShLangCompute; },
         pattern(_) = [] { return EShLangCount; }
      );
      // clang-format on
   }
   auto builder::get_spirv_version(uint32_t version) const -> glslang::EShTargetLanguageVersion
   {
      using namespace mpark::patterns;

      uint32_t major = VK_VERSION_MAJOR(version);
      uint32_t minor = VK_VERSION_MINOR(version);

      // clang-format off
      return match(major, minor)(
         pattern(1u, 0u) = [] { return glslang::EShTargetSpv_1_0; },
         pattern(1u, 1u) = [] { return glslang::EShTargetSpv_1_3; },
         pattern(1u, 2u) = [] { return glslang::EShTargetSpv_1_5; },
         pattern(_, _) = [] { return glslang::EShTargetLanguageVersionCount; }
      );
      // clang-format on
   }
   auto builder::get_vulkan_version(uint32_t version) const -> glslang::EshTargetClientVersion
   {
      using namespace mpark::patterns;

      uint32_t major = VK_VERSION_MAJOR(version);
      uint32_t minor = VK_VERSION_MINOR(version);

      // clang-format off
      return match(major, minor)(
         pattern(1u, 0u) = [] { return glslang::EShTargetVulkan_1_0; },
         pattern(1u, 1u) = [] { return glslang::EShTargetVulkan_1_1; },
         pattern(1u, 2u) = [] { return glslang::EShTargetVulkan_1_2; },
         pattern(_, _) = [] { return glslang::EShTargetClientVersionCount; }
      );
      // clang-format on
   }
} // namespace core
