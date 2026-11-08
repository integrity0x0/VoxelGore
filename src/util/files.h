#pragma once

#include <cstddef>
#include <string_view>
#include <vector>
#include <string>

namespace util {
[[nodiscard]] extern std::vector<std::byte> ReadFileBytes(std::string_view path);
[[nodiscard]] extern std::string ReadFile(std::string_view path);
[[nodiscard]] extern std::vector<std::string> ListFiles(std::string_view dir);
}  // namespace util