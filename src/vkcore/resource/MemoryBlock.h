#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <optional>

#include "../devices/Device.h"
#include "DescriptorSet.h"
#include "DeviceMemory.h"
#include "FreeListAllocator.h"

namespace vkcore {

struct MemoryBlockStorage {
  DeviceMemory memory = DeviceMemory({}, 0, 0);
  FreeListAllocator allocator;

  MemoryBlockStorage(DeviceMemory&& memory, FreeListAllocator&& allocator)
      : memory(std::move(memory)), allocator(std::move(allocator)) {}
};

class MemorySlice {
 public:
  MemorySlice(std::shared_ptr<MemoryBlockStorage> storage, const FreeListAllocator::Region& region)
      : storage_(std::move(storage)), region_(region) {}

  ~MemorySlice() { release(); }

  MemorySlice(const MemorySlice&) = delete;
  MemorySlice& operator=(const MemorySlice&) = delete;

  MemorySlice(MemorySlice&& other) noexcept
      : storage_(std::move(other.storage_)), region_(other.region_) {
    other.storage_.reset();
  }

  MemorySlice& operator=(MemorySlice&& other) noexcept {
    if (this != &other) {
      release();

      storage_ = std::move(other.storage_);
      region_ = other.region_;

      other.storage_.reset();
    }

    return *this;
  }

  [[nodiscard]] VkDeviceMemory memory() const {
    return storage_ ? storage_->memory.handle() : VK_NULL_HANDLE;
  }

  [[nodiscard]] void* map(VkDeviceSize offset = 0) const {
    if (!storage_) {
      return nullptr;
    }

    return storage_->memory.map(region_.offset + offset);
  }

  [[nodiscard]] VkDeviceSize offset() const { return region_.offset; }

  [[nodiscard]] VkDeviceSize size() const { return region_.size; }

 private:
  void release() {
    if (storage_) {
      storage_->allocator.free(region_);
      storage_.reset();
    }
  }

  std::shared_ptr<MemoryBlockStorage> storage_;
  FreeListAllocator::Region region_;
};

class MemoryBlock {
 public:
  MemoryBlock(const Device& device, VkDeviceSize size, VkMemoryPropertyFlags flags,
              uint32_t memoryTypeIndex);

  [[nodiscard]] std::optional<MemorySlice> reserve(VkDeviceSize size, VkDeviceSize alignment = 1);

  [[nodiscard]] std::optional<MemorySlice> reserveFull();

  [[nodiscard]] DeviceMemory& deviceMemory() const { return storage_->memory; }

 private:
  const Device* device_;
  std::shared_ptr<MemoryBlockStorage> storage_;
};

}  // namespace vkcore