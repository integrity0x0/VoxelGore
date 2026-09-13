#pragma once

#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

namespace util {

template <typename T>
static void HashCombine(size_t& seed, T value) noexcept {
  std::hash<T> hasher;
  seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct IVec3Hash {
  size_t operator()(const glm::ivec3& v) const noexcept {
    size_t seed = 0;
    HashCombine(seed, v.x);
    HashCombine(seed, v.y);
    HashCombine(seed, v.z);
    return seed;
  }
};

struct StringHash {
  using is_transparent = void;
  size_t operator()(std::string_view sv) const { return std::hash<std::string_view>{}(sv); }
};

}  // namespace util