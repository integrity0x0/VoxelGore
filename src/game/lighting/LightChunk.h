#pragma once

#include <stdint.h>
#include <array>
#include <glm/glm.hpp>

namespace gm {

class LightChunk {
 public:
  static constexpr uint32_t kLength = 16u;
  static constexpr uint32_t kVolume = kLength * kLength * kLength;

  explicit LightChunk() { data_.fill(0); }

  [[nodiscard]] uint8_t Get(const glm::ivec3& localPos) const;

  void Set(const glm::ivec3& localPos, uint8_t value);

  const std::array<uint8_t, kVolume / 2ull>& data() const {
    return data_;
  }

  static [[nodiscard]] size_t GetIndex(const glm::ivec3& localPos);

 private:
  std::array<uint8_t, kVolume / 2ull> data_;
};
}  // namespace gm
