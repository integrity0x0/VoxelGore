#pragma once

#include <shaderc/shaderc.hpp>
#include <string>

namespace gfx {
class ShaderIncluder final : public shaderc::CompileOptions::IncluderInterface {
 public:
  explicit ShaderIncluder(std::string basePath) : basePath_(std::move(basePath)) {}
  explicit ShaderIncluder(std::string&& basePath) : basePath_(std::move(basePath)) {}

  [[nodiscard]] shaderc_include_result* GetInclude(const char* requested_source,
                                                   shaderc_include_type type,
                                                   const char* requesting_source,
                                                   size_t include_depth) override;

  void ReleaseInclude(shaderc_include_result* data) override;

 private:
  struct IncludeData {
    std::string path;
    std::string content;
  };

 private:
  std::string basePath_;
};

}  // namespace gfx