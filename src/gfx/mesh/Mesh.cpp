#include "Mesh.h"

#include <cassert>

namespace gfx {

namespace {
vkcore::BufferSlice createVertexBuffer(const vkcore::Device& device, VkDeviceSize size) {
  VkBufferCreateInfo bufferCI = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCI.size = size;
  bufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  return *vkcore::BufferBlock(device, bufferCI, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).Allocate(size);
}
}  // namespace

Mesh::Mesh(const vkcore::Device& device, vkcore::BufferSlice&& buffer, uint32_t vertexCount)
    : device_(&device), buffer_(std::move(buffer)), vertexCount_(vertexCount) {}

void Mesh::Bind(VkCommandBuffer cmd) const {
  const auto& dt = device_->dispatchTable();

  VkBuffer handle = buffer_.handle();
  VkDeviceSize offset = buffer_.offset();

  dt.vkCmdBindVertexBuffers(cmd, 0, 1, &handle, &offset);

  if (indexBuffer_.has_value())
    dt.vkCmdBindIndexBuffer(cmd, indexBuffer_->handle(), indexBuffer_->offset(), indexType_);
}

Mesh::Mesh(const vkcore::Device& device, vkcore::BufferSlice&& buffer, uint32_t vertexCount,
           vkcore::BufferSlice&& indexBuffer, uint32_t indexCount, VkIndexType indexType)
    : device_(&device),
      buffer_(std::move(buffer)),
      vertexCount_(vertexCount),
      indexBuffer_(std::move(indexBuffer)),
      indexCount_(indexCount),
      indexType_(indexType) {}

void Mesh::Draw(VkCommandBuffer cmd) const {
  const auto& dt = device_->dispatchTable();
  if (indexBuffer_)
    dt.vkCmdDrawIndexed(cmd, indexCount_, 1u, 0u, 0u, 0u);
  else
    dt.vkCmdDraw(cmd, static_cast<uint32_t>(vertexCount_), 1u, 0u, 0u);
}

}  // namespace gfx