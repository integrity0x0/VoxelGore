#include "ShaderCompiler.h"

#include "../../../util/files.h"
#include "ShaderIncluder.h"

namespace gfx {

std::vector<uint32_t> ShaderCompiler::Compile(std::string_view source, std::string_view filename,
                                              shaderc_shader_kind kind,
                                              const ShaderDefinitions& localDefinitions) const {
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;

  for (const auto& [name, value] : globalDefinitions_) {
    options.AddMacroDefinition(name, value);
  }

  for (const auto& [name, value] : localDefinitions) {
    options.AddMacroDefinition(name, value);
  }

  options.SetIncluder(std::make_unique<ShaderIncluder>(basePath_));
  options.SetOptimizationLevel(shaderc_optimization_level_performance);
  options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);

  shaderc::SpvCompilationResult result =
      compiler.CompileGlslToSpv(source.data(), source.size(), kind, filename.data(), options);

  if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    throw std::runtime_error(result.GetErrorMessage());
  }

  return {result.cbegin(), result.cend()};
}
vkcore::ShaderModule CompileShaderModule(const ShaderCompiler& compiler,
                                         const vkcore::Device& device, std::string_view path,
                                         shaderc_shader_kind kind,
                                         const ShaderDefinitions& localDefinitions) {
  return vkcore::ShaderModule(device,
                              compiler.Compile(util::ReadFile(path), path, kind, localDefinitions));
}

}  // namespace gfx