#include "ShaderIncluder.h"

#include <filesystem>

#include "../../../util/files.h"
#include "../../../util/pathUtils.h"

namespace gfx {
shaderc_include_result* ShaderIncluder::GetInclude(const char* requestedSource,
                                                   shaderc_include_type type,
                                                   const char* requestingSource,
                                                   size_t includeDepth) {
  std::filesystem::path resolved =
      (type == shaderc_include_type_relative)
          ? std::filesystem::path(requestingSource).parent_path() / requestedSource
          : std::filesystem::path(basePath_) / requestedSource;

  auto* data = new IncludeData{resolved.string(), util::ReadFile(resolved.string())};

  auto* result = new shaderc_include_result();
  result->source_name = data->path.c_str();
  result->source_name_length = data->path.size();
  result->content = data->content.c_str();
  result->content_length = data->content.size();
  result->user_data = data;
  return result;
}

void ShaderIncluder::ReleaseInclude(shaderc_include_result* data) {
  delete reinterpret_cast<IncludeData*>(data->user_data);
}

}  // namespace gfx