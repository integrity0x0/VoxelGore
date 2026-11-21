#pragma once

#include <vector>
#include <stdint.h>
#include <glm/glm.hpp>

namespace util {
class RingIndexer {
 public:
  RingIndexer(const glm::uvec3& size, const glm::ivec3& origin = {});

  [[nodiscard]] const glm::ivec3& GetOrigin() const { return origin_; }
  [[nodiscard]] const glm::uvec3& GetSize() const { return size_; }
  [[nodiscard]] const glm::uvec3& GetCapacity() const { return capacity_; }
  [[nodiscard]] const glm::uvec3& GetMask() const { return mask_; }

 private:
  glm::uvec3 size_;
  glm::uvec3 capacity_;
  glm::uvec3 mask_;

  glm::ivec3 origin_;
};
}  // namespace util
