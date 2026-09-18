#pragma once

#include <shaderc/shaderc.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../util/hashers.h"
#include "../../../vkcore/pipeline/ShaderModule.h"

namespace gfx {

using ShaderDefinitions =
    std::unordered_map<std::string, std::string, util::StringHash, std::equal_to<>>;

class ShaderCompiler {
 public:
  explicit ShaderCompiler(std::string basePath) : basePath_(std::move(basePath)) {}

  void AddDefinition(std::string_view name, std::string_view value) {
    globalDefinitions_.insert_or_assign(std::string(name), std::string(value));
  }

  void RemoveDefinition(std::string_view name) {
    if (auto it = globalDefinitions_.find(name); it != globalDefinitions_.end()) {
      globalDefinitions_.erase(it);
    }
  }

  [[nodiscard]] std::vector<uint32_t> Compile(std::string_view source, std::string_view filename,
                                              shaderc_shader_kind kind,
                                              const ShaderDefinitions& localDefinitions = {}) const;

 private:
  std::string basePath_;
  shaderc::Compiler compiler_;
  ShaderDefinitions globalDefinitions_;
};

[[nodiscard]] vkcore::ShaderModule CompileShaderModule(
    const ShaderCompiler& compiler, const vkcore::Device& device, std::string_view path,
    shaderc_shader_kind kind, const ShaderDefinitions& localDefinitions = {});

}  // namespace gfx