#include "TranslucentMesh.h"

#include <algorithm>
#include <cmath>

namespace gfx {

TranslucentMesh::TranslucentMesh(const vkcore::Device& device, Mesh&& mesh,
                                 vkcore::BufferAllocator& bufferAllocator, uint32_t framesCount,
                                 std::vector<QuadEntry> quads)
    : device_(&device), mesh_(std::move(mesh)) {
  assert(framesCount > 0);
  assert(!quads.empty());

  const size_t quadCount = quads.size();

  centers_.resize(quadCount);
  quadIndices_.resize(quadCount);
  order_.resize(quadCount);

  for (size_t i = 0; i < quadCount; ++i) {
    centers_[i] = quads[i].center;
    quadIndices_[i] = {
        quads[i].indices[0], quads[i].indices[1], quads[i].indices[2],
        quads[i].indices[3], quads[i].indices[4], quads[i].indices[5],
    };

    order_[i] = static_cast<uint32_t>(i);
  }

  constexpr VkDeviceSize kIndicesPerQuad = 6;
  const VkDeviceSize indexBufferSize =
      static_cast<VkDeviceSize>(quadCount) * kIndicesPerQuad * sizeof(uint32_t);

  frames_.reserve(framesCount);

  for (uint32_t i = 0; i < framesCount; ++i) {
    vkcore::BufferSlice indexBuffer = bufferAllocator.Allocate(
        indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint32_t* mapped = reinterpret_cast<uint32_t*>(indexBuffer.map());

    frames_.emplace_back(std::move(indexBuffer), mapped);
  }
}

void TranslucentMesh::Sort(uint32_t currentFrame, const glm::vec3& cameraPos, float threshold) {
  assert(currentFrame < frames_.size());

  auto& frame = frames_[currentFrame];

  if (frame.sortedOnce && glm::length(cameraPos - frame.lastSortPos) < threshold) {
    return;
  }

  std::sort(order_.begin(), order_.end(), [&](uint32_t a, uint32_t b) {
    const float da = glm::distance2(cameraPos, centers_[a]);
    const float db = glm::distance2(cameraPos, centers_[b]);
    return da > db;
  });

  for (size_t i = 0; i < order_.size(); ++i) {
    std::memcpy(frame.mapped + i * 6, quadIndices_[order_[i]].data(), 6 * sizeof(uint32_t));
  }

  frame.lastSortPos = cameraPos;
  frame.sortedOnce = true;
}

void TranslucentMesh::Draw(VkCommandBuffer cmd, uint32_t currentFrame) {
  assert(currentFrame < frames_.size());

  auto& frame = frames_[currentFrame];

  frame.indexBuffer.BindIndex(cmd);
  mesh_.Bind(cmd);
  device_->dispatchTable().vkCmdDrawIndexed(cmd, static_cast<uint32_t>(quadIndices_.size()) * 6u, 1,
                                            0, 0, 0);
}

}  // namespace gfx