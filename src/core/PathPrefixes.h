#pragma once

#include <string>

namespace core {
#ifdef __ANDROID__
static const std::string kAssetsPrefix = "";
#else
static const std::string kAssetsPrefix = "assets/";
#endif
static const std::string kShadersPrefix = kAssetsPrefix + "shaders/";
static const std::string kPagesPrefix = kAssetsPrefix + "pages/";
static const std::string kBlocksPrefix = kAssetsPrefix + "blocks/";
}  // namespace core