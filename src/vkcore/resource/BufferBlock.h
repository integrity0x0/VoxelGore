#pragma once

#include <memory>

#include "Buffer.h"
#include "FreeListAllocator.h"
#include "MemoryBlock.h"

namespace vkcore {

struct BufferBlockStorage {
  Buffer buffer;
  FreeListAllocator allocator;

  BufferBlockStorage(Buffer&& buffer, FreeListAllocator allocator)
      : buffer(std::move(buffer)), allocator(std::move(allocator)) {}

  ~BufferBlockStorage() = default;

  BufferBlockStorage(const BufferBlockStorage&) = delete;
  BufferBlockStorage& operator=(const BufferBlockStorage&) = delete;

  BufferBlockStorage(BufferBlockStorage&&) noexcept = default;
  BufferBlockStorage& operator=(BufferBlockStorage&&) noexcept = default;
};

class BufferSlice {
 public:
  BufferSlice(std::shared_ptr<BufferBlockStorage> storage, const FreeListAllocator::Region& region)
      : storage_(std::move(storage)), region_(region) {}

  ~BufferSlice() { release(); }

  BufferSlice(const BufferSlice&) = delete;
  BufferSlice& operator=(const BufferSlice&) = delete;

  BufferSlice(BufferSlice&& other) noexcept
      : storage_(std::move(other.storage_)), region_(other.region_) {
    other.storage_.reset();
  }

  BufferSlice& operator=(BufferSlice&& other) noexcept {
    if (this != &other) {
      release();
      storage_ = std::move(other.storage_);
      region_ = other.region_;
      other.storage_.reset();
    }
    return *this;
  }

  [[nodiscard]] const vkcore::Buffer& buffer() const { return storage_->buffer; }

  [[nodiscard]] VkBuffer handle() const {
    return storage_ ? storage_->buffer.handle() : VK_NULL_HANDLE;
  }
  [[nodiscard]] VkDeviceSize offset() const { return region_.offset; }
  [[nodiscard]] VkDeviceSize size() const { return region_.size; }
  [[nodiscard]] void* map(VkDeviceSize offset = 0ull) const {
    return storage_ ? storage_->buffer.memorySlice().map(region_.offset + offset) : nullptr;
  }

  void BindVertex(VkCommandBuffer cmd, VkDeviceSize offset = 0ull,
                  uint32_t firstBinding = 0u) const {
    if (storage_) storage_->buffer.BindVertex(cmd, region_.offset + offset, firstBinding);
  }
  void BindIndex(VkCommandBuffer cmd, VkDeviceSize offset = 0ull) const {
    if (storage_) storage_->buffer.BindIndex(cmd, region_.offset + offset);
  }

 private:
  void release() {
    if (storage_) {
      storage_->allocator.free(region_);
      storage_.reset();
    }
  }

  std::shared_ptr<BufferBlockStorage> storage_;
  FreeListAllocator::Region region_;
};

class BufferBlock {
 public:
  BufferBlock(const Device& device, const VkBufferCreateInfo& bufferCI, MemoryAllocator& allocator,
              VkMemoryPropertyFlags memProperties);

  BufferBlock(const Device& device, const VkBufferCreateInfo& bufferCI,
              VkMemoryPropertyFlags memProperties);

  ~BufferBlock() = default;

  BufferBlock(const BufferBlock&) = delete;
  BufferBlock& operator=(const BufferBlock&) = delete;

  BufferBlock(BufferBlock&&) noexcept = default;
  BufferBlock& operator=(BufferBlock&&) noexcept = default;

  [[nodiscard]] const Buffer& buffer() const { return storage_->buffer; }

  std::optional<BufferSlice> Allocate(VkDeviceSize size, VkDeviceSize alignment = 0ull);

 private:
  std::shared_ptr<BufferBlockStorage> storage_;
};

}  // namespace vkcore