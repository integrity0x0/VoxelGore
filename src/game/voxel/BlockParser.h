#pragma once

#include <nlohmann/json.hpp>
#include <string_view>

#include "../../util/pathUtils.h"
#include "Block.h"
#ifdef __ANDROID__
#include <android_native_app_glue.h>
extern AAssetManager* g_AAssetManager;
#endif

namespace gm {
class BlockParser {
 public:
  [[nodiscard]] static Block Parse(uint32_t id, std::string_view path);

 private:
  [[nodiscard]] static nlohmann::json loadJson(std::string_view path);
  [[nodiscard]] static Block parseBlockJson(uint32_t id, const nlohmann::json& j,
                                            const std::string& jsonPath);
  static std::unordered_map<std::string, Block::RenderLayer> kNameByRenderLayer;
};
}  // namespace gm