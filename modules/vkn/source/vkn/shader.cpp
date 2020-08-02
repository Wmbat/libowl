#include "vkn/shader.hpp"

#include <monads/try.hpp>

#include <fstream>

namespace vkn
{
   namespace detail
   {
      auto to_string(shader::error err) -> std::string
      {
         using error = shader::error;

         switch (err)
         {
            case error::no_filepath:
               return "NO_FILEPATH";
            case error::invalid_filepath:
               return "INVALID_FILEPATH";
            case error::filepath_not_a_file:
               return "FILEPATH_NOT_A_FILE";
            case error::failed_to_open_file:
               return "FAILED_TO_OPEN_FILE";
            case error::failed_to_preprocess_shader:
               return "FAILED_TO_PREPROCESS_SHADER";
            case error::failed_to_parse_shader:
               return "FAILED_TO_PARSE_SHADER";
            case error::failed_to_link_shader:
               return "FAILED_TO_LINK_SHADER";
            case error::failed_to_create_shader_module:
               return "FAILED_TO_CREATE_SHADER_MODULE";
            default:
               return "UNKNOWN";
         }
      };

      constexpr auto get_shader_stage(std::string_view stage) -> EShLanguage
      {
         if (stage == "vert")
         {
            return EShLangVertex;
         }
         else if (stage == "tesc")
         {
            return EShLangTessControl;
         }
         else if (stage == "tese")
         {
            return EShLangTessEvaluation;
         }
         else if (stage == "geom")
         {
            return EShLangGeometry;
         }
         else if (stage == "frag")
         {
            return EShLangFragment;
         }
         else if (stage == "comp")
         {
            return EShLangCompute;
         }
         else
         {
            return EShLangCount;
         }
      }
      constexpr auto get_shader_type(std::string_view stage) -> shader::type
      {
         if (stage == "vert")
         {
            return shader::type::vertex;
         }
         else if (stage == "tesc")
         {
            return shader::type::tes_control;
         }
         else if (stage == "tese")
         {
            return shader::type::tes_eval;
         }
         else if (stage == "geom")
         {
            return shader::type::geometry;
         }
         else if (stage == "frag")
         {
            return shader::type::fragment;
         }
         else if (stage == "comp")
         {
            return shader::type::compute;
         }
         else
         {
            return shader::type::count;
         }
      }

      constexpr auto get_spirv_version(uint32_t version) -> glslang::EShTargetLanguageVersion
      {
         if (VK_VERSION_MAJOR(version) == 1)
         {
            if (VK_VERSION_MINOR(version) == 0)
            {
               return glslang::EShTargetSpv_1_0;
            }
            else if (VK_VERSION_MINOR(version) == 1)
            {
               return glslang::EShTargetSpv_1_3;
            }
            else if (VK_VERSION_MINOR(version) == 2)
            {
               return glslang::EShTargetSpv_1_5;
            }
            else
            {
               return glslang::EShTargetLanguageVersionCount;
            }
         }
         else
         {
            return glslang::EShTargetLanguageVersionCount;
         }
      }
      constexpr auto get_vulkan_version(uint32_t version) -> glslang::EshTargetClientVersion
      {
         if (VK_VERSION_MAJOR(version) == 1)
         {
            if (VK_VERSION_MINOR(version) == 0)
            {
               return glslang::EShTargetVulkan_1_0;
            }
            else if (VK_VERSION_MINOR(version) == 1)
            {
               return glslang::EShTargetVulkan_1_1;
            }
            else if (VK_VERSION_MINOR(version) == 2)
            {
               return glslang::EShTargetVulkan_1_2;
            }
            else
            {
               return glslang::EShTargetClientVersionCount;
            }
         }
         else
         {
            return glslang::EShTargetClientVersionCount;
         }
      }

      auto rectify_path(const std::filesystem::path& path) -> std::filesystem::path
      {
         return path.is_relative() ? std::filesystem::current_path() / path : path;
      }

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

   auto shader::error_category::name() const noexcept -> const char* { return "vk_shader"; }
   auto shader::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<shader::error>(err));
   }

   shader::shader(const create_info& info) :
      m_device{info.device}, m_shader_module{info.shader_module}, m_type{info.type}
   {}
   shader::shader(create_info&& info) :
      m_device{info.device}, m_shader_module{info.shader_module}, m_type{info.type}
   {}
   shader::shader(shader&& other) noexcept { *this = std::move(other); }
   shader::~shader()
   {
      if (m_device && m_shader_module)
      {
         m_device.destroyShaderModule(m_shader_module);

         m_shader_module = nullptr;
         m_device = nullptr;
      }
   }

   auto vkn::shader::operator=(shader&& rhs) noexcept -> shader&
   {
      if (this != &rhs)
      {
         m_device = rhs.m_device;
         rhs.m_device = nullptr;

         m_shader_module = rhs.m_shader_module;
         rhs.m_shader_module = nullptr;

         m_type = rhs.m_type;
         rhs.m_type = shader::type::count;
      }

      return *this;
   }

   auto shader::value() noexcept -> vk::ShaderModule& { return m_shader_module; }
   auto shader::value() const noexcept -> const vk::ShaderModule& { return m_shader_module; }
   auto shader::stage() const noexcept -> type { return m_type; }

   shader::builder::builder(const device& device, util::logger* const plogger) : m_plogger{plogger}
   {
      m_info.device = device.value();
      m_info.version = device.get_vulkan_version();
   }

   auto shader::builder::build() -> result<shader>
   {
      using err_t = vkn::error;

      if (m_info.path.empty())
      {
         // clang-format off
         return monad::make_left(err_t{
            .type = shader::make_error_code(shader::error::no_filepath),
            .result = {}
         });
         // clang-format on
      }

      const auto rectified_path = detail::rectify_path(m_info.path);
      if (!std::filesystem::exists(rectified_path))
      {
         // clang-format off
         return monad::make_left(err_t{
            .type = shader::make_error_code(shader::error::invalid_filepath),
            .result = {}
         });
         // clang-format on
      }

      if (!std::filesystem::is_regular_file(rectified_path))
      {
         // clang-format off
         return monad::make_left(err_t{
            .type = shader::make_error_code(shader::error::filepath_not_a_file),
            .result = {}
         });
         // clang-format on
      }

      std::ifstream file{rectified_path};
      if (!file.is_open())
      {
         // clang-format off
         return monad::make_left(err_t{
            .type = shader::make_error_code(shader::error::failed_to_open_file),
            .result = {}
         });
         // clang-format on
      }

      std::string spirv_version{};
      glslang::GetSpirvVersion(spirv_version);

      const std::string parent_path = rectified_path.parent_path();
      const std::string extension = rectified_path.extension();
      const auto shader_stage = detail::get_shader_stage({extension.begin() + 1, extension.end()});

      if (shader_stage != EShLangCount)
      {
         // if not compiled, compile it
      }
      else
      {
         // if already compiled
      }

      const int client_input_semantics_version = 100;
      const int default_version = 100;

      using file_it = std::istreambuf_iterator<char>;
      const std::string shader_data_str{file_it{file}, file_it{}};

      glslang::TShader tshader{shader_stage};
      {
         const char* shader_data_c_str = shader_data_str.c_str();
         tshader.setStrings(&shader_data_c_str, 1);
      }

      tshader.setEnvInput(glslang::EShSourceGlsl, shader_stage, glslang::EShClientVulkan,
                          client_input_semantics_version);
      tshader.setEnvClient(glslang::EShClientVulkan, detail::get_vulkan_version(m_info.version));
      tshader.setEnvTarget(glslang::EshTargetSpv, detail::get_spirv_version(m_info.version));

      const auto resources = detail::default_built_in_resource;
      const auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

      DirStackFileIncluder includer;
      includer.pushExternalLocalDirectory(parent_path);

      std::string preprocessed_glsl;
      if (!tshader.preprocess(&resources, default_version, ENoProfile, false, false, messages,
                              &preprocessed_glsl, includer))
      {
         util::log_error(m_plogger, "[vkn] {0}", tshader.getInfoLog());

         // clang-format off
         return monad::make_left(err_t{
            .type = shader::make_error_code(shader::error::failed_to_preprocess_shader),
            .result = {}
         });
         // clang-format on
      }

      const char* preprocessed_glsl_c_str = preprocessed_glsl.c_str();
      tshader.setStrings(&preprocessed_glsl_c_str, 1);

      if (!tshader.parse(&resources, default_version, false, messages))
      {
         util::log_error(m_plogger, "[vkn] {0}", tshader.getInfoLog());

         // clang-format off
         return monad::make_left(err_t{
            .type = shader::make_error_code(shader::error::failed_to_parse_shader),
            .result = {}
         });
         // clang-format on
      }

      glslang::TProgram program;
      program.addShader(&tshader);

      if (!program.link(messages))
      {
         util::log_error(m_plogger, "[vkn] {0}", tshader.getInfoLog());
         util::log_error(m_plogger, "[vkn] {0}", tshader.getInfoDebugLog());

         // clang-format off
         return monad::make_left(err_t{
            .type = shader::make_error_code(shader::error::failed_to_link_shader),
            .result = {}
         });
         // clang-format on  
      }

      std::vector<uint32_t> spirv;
      spv::SpvBuildLogger logger;
      glslang::SpvOptions spv_options;
      glslang::GlslangToSpv(*program.getIntermediate(shader_stage), spirv, &logger, &spv_options);

      const auto create_info = vk::ShaderModuleCreateInfo{}
         .setPNext(nullptr)
         .setFlags({})
         .setCodeSize(spirv.size() * 4)
         .setPCode(spirv.data());

      const auto module_res = monad::try_wrap<vk::SystemError>([&]{
         return m_info.device.createShaderModule(create_info);
      });

      if(!module_res.is_right())
      {
         // clang-format off
         return monad::make_left(err_t{
            .type = shader::make_error_code(error::failed_to_create_shader_module), 
            .result = static_cast<vk::Result>(module_res.left()->code().value())
         });
         // clang-format on
      }

      util::log_info(m_plogger, "[vkn] shader module created");

      // clang-format off
      return monad::make_right(shader{shader::create_info{
         .device = m_info.device,
         .shader_module = module_res.right().value(),
         .type = detail::get_shader_type({extension.begin() + 1, extension.end()})
      }});
      // clang-format on
   }

   auto shader::builder::set_filepath(const std::filesystem::path& path) -> builder&
   {
      m_info.path = path;
      return *this;
   }

   auto shader::builder::enable_caching(bool cache) noexcept -> builder&
   {
      m_info.is_caching_enabled = cache;
      return *this;
   }
} // namespace vkn
