#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace util {
[[nodiscard]] extern std::vector<std::byte> ReadFileBytes(std::string_view path);
}  // namespace util