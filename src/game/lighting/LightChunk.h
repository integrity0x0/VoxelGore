#pragma once

#include <stdint.h>
#include <array>
#include <glm/glm.hpp>

namespace gm {

class LightChunk {
 public:
  explicit LightChunk(const glm::ivec3& pos) : pos_(pos) { data_.fill(0); }

  static constexpr uint32_t kLengthBits = 4u;
  static constexpr uint32_t kLength = 1u << kLengthBits;
  static constexpr uint32_t kVolume = kLength * kLength * kLength;

  [[nodiscard]] uint8_t Get(const glm::ivec3& localPos) const;

  void Set(const glm::ivec3& localPos, uint8_t value);

  const std::array<uint8_t, kVolume / 2ull>& data() const {
    return data_;
  }

  static [[nodiscard]] size_t GetIndex(const glm::ivec3& localPos);

  const glm::ivec3& pos() const { return pos_; }
 private:
  std::array<uint8_t, kVolume / 2ull> data_;
  glm::ivec3 pos_;
};
}  // namespace gm
