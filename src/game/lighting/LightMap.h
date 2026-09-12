#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <glm/vec3.hpp>
#include <memory>

namespace gm {

enum class LightChannel { R, G, B, S, Count };

class LightMap {
 public:
  LightMap(size_t width, size_t height, size_t depth)
      : width_(width),
        height_(height),
        depth_(depth),
        size_(width * height * depth),
        map_(std::make_unique<uint16_t[]>(size_)) {
    std::memset(map_.get(), 0, size_ * sizeof(uint16_t));
  }

  bool Contains(const glm::ivec3& pos) const {
    return pos.x >= 0 && pos.y >= 0 && pos.z >= 0 && static_cast<size_t>(pos.x) < width_ &&
           static_cast<size_t>(pos.y) < height_ && static_cast<size_t>(pos.z) < depth_;
  }

  size_t index(const glm::ivec3& pos) const {
    return static_cast<size_t>(pos.x) +
           width_ * (static_cast<size_t>(pos.y) + height_ * static_cast<size_t>(pos.z));
  }

  uint16_t get(size_t index) const { return map_[index]; }

  uint16_t get(const glm::ivec3& pos) const {
    if (!Contains(pos)) return 0;
    return map_[index(pos)];
  }

  void set(size_t index, uint16_t light) { map_[index] = light; }

  void set(const glm::ivec3& pos, uint16_t light) {
    if (!Contains(pos)) return;
    map_[index(pos)] = light;
  }

  uint8_t getR(size_t index) const { return map_[index] & 0xF; }

  uint8_t getR(const glm::ivec3& pos) const {
    if (!Contains(pos)) return 0;
    return getR(index(pos));
  }

  uint8_t getG(size_t index) const { return (map_[index] >> 4) & 0xF; }

  uint8_t getG(const glm::ivec3& pos) const {
    if (!Contains(pos)) return 0;
    return getG(index(pos));
  }

  uint8_t getB(size_t index) const { return (map_[index] >> 8) & 0xF; }

  uint8_t getB(const glm::ivec3& pos) const {
    if (!Contains(pos)) return 0;
    return getB(index(pos));
  }

  uint8_t getS(size_t index) const { return (map_[index] >> 12) & 0xF; }

  uint8_t getS(const glm::ivec3& pos) const {
    if (!Contains(pos)) return 0;
    return getS(index(pos));
  }

  void setR(size_t index, uint8_t value) { map_[index] = (map_[index] & 0xFFF0) | (value & 0xF); }

  void setR(const glm::ivec3& pos, uint8_t value) {
    if (!Contains(pos)) return;
    setR(index(pos), value);
  }

  void setG(size_t index, uint8_t value) {
    map_[index] = (map_[index] & 0xFF0F) | ((value & 0xF) << 4);
  }

  void setG(const glm::ivec3& pos, uint8_t value) {
    if (!Contains(pos)) return;
    setG(index(pos), value);
  }

  void setB(size_t index, uint8_t value) {
    map_[index] = (map_[index] & 0xF0FF) | ((value & 0xF) << 8);
  }

  void setB(const glm::ivec3& pos, uint8_t value) {
    if (!Contains(pos)) return;
    setB(index(pos), value);
  }

  void setS(size_t index, uint8_t value) {
    map_[index] = (map_[index] & 0x0FFF) | ((value & 0xF) << 12);
  }

  void setS(const glm::ivec3& pos, uint8_t value) {
    if (!Contains(pos)) return;
    setS(index(pos), value);
  }

  size_t width() const { return width_; }
  size_t height() const { return height_; }
  size_t depth() const { return depth_; }

 private:
  size_t width_;
  size_t height_;
  size_t depth_;
  size_t size_;

  std::unique_ptr<uint16_t[]> map_;
};

}  // namespace gm