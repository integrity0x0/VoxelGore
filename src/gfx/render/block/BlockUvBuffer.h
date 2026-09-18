#pragma once

#include <vector>

#include "../../../vkcore/resource/Buffer.h"
#include "../../UvRegion.h"

namespace gfx {
struct alignas(16) UniformUv {
  glm::vec4 uvRect;
  uint32_t arrayLayer;
};
static_assert(sizeof(UniformUv) == 32, "must match std140 stride");
static_assert(alignof(UniformUv) == 16, "must match std140 base alignment");

class BlockUvBuffer {
 public:
  BlockUvBuffer(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
           uint32_t maxSurfaces);

  [[nodiscard]] UniformUv* mapped() { return mapped_; };

  [[nodiscard]] VkBuffer handle() const { return buffer_.handle(); }

  [[nodiscard]] VkDeviceSize capacityBytes() const {
    return static_cast<VkDeviceSize>(maxSurfaces_) * sizeof(UvRegion);
  }

  [[nodiscard]] uint32_t maxSurfaces() const { return maxSurfaces_; }

 private:
  vkcore::Buffer buffer_;
  UniformUv* mapped_;
  uint32_t maxSurfaces_;
};

}  // namespace gfx