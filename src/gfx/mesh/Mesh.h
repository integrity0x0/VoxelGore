#pragma once

#include "../../vkcore/commands/CommandPool.h"
#include "../../vkcore/resource/BufferAllocator.h"
#include "../../vkcore/resource/memoryUtils.h"

namespace gfx {

class Mesh {
 public:
  Mesh(const vkcore::Device& device, vkcore::BufferSlice&& buffer, uint32_t vertexCount);
  Mesh(const vkcore::Device& device, vkcore::BufferSlice&& buffer, uint32_t vertexCount,
       vkcore::BufferSlice&& indexBuffer, uint32_t indexCount,
       VkIndexType indexType = VK_INDEX_TYPE_UINT32);

  void Bind(VkCommandBuffer cmd) const;
  void Draw(VkCommandBuffer cmd) const;
  uint32_t vertexCount() const { return vertexCount_; }
  uint32_t indexCount() const { return indexCount_; }
  bool HasIndex() const { return indexBuffer_.has_value(); }
  VkIndexType indexType() const { return indexType_; }
  const std::optional<vkcore::BufferSlice>& indexBuffer() const { return indexBuffer_; }

  const vkcore::BufferSlice& vertexBuffer() const { return buffer_; }

 private:
  const vkcore::Device* device_;
  vkcore::BufferSlice buffer_;
  uint32_t vertexCount_;

  std::optional<vkcore::BufferSlice> indexBuffer_;
  uint32_t indexCount_ = 0;
  VkIndexType indexType_ = VK_INDEX_TYPE_UINT32;
};

}  // namespace gfx