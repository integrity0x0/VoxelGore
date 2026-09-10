#pragma once

#include <string>

namespace core {
#ifdef __ANDROID__
static const std::string kAssetsPrefix = "";
#else
static const std::string kAssetsPrefix = "assets/";
#endif
}  // namespace core