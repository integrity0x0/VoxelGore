#include "RingIndexer.h"

#include <bit>

namespace util {

namespace {
static glm::vec3 Vec3BitCeil(const glm::uvec3& vec) {
  return glm::uvec3(std::bit_ceil(vec.x), std::bit_ceil(vec.y), std::bit_ceil(vec.z));
}
}  // namespace

RingIndexer::RingIndexer(const glm::uvec3& size, const glm::ivec3& origin)
    : size_(size), capacity_(Vec3BitCeil(size)), mask_(capacity_ - glm::uvec3(1)), origin_(origin) {
}

void RingIndexer::Recenter(const glm::ivec3& origin, std::vector<glm::ivec3>& removed,
                           std::vector<glm::ivec3>& added) {
  glm::ivec3 delta = origin - origin_;
  

}

}  // namespace util