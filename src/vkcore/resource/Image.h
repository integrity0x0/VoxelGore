#pragma once

#include "../devices/Device.h"
#include "MemoryAllocator.h"
#include "MemoryBlock.h"

namespace vkcore {

class Image {
 public:
  Image(const Device& device, const VkImageCreateInfo& imageCI, VkMemoryPropertyFlags flags);

  Image(const Device& device, const VkImageCreateInfo& imageCI, MemoryAllocator& allocator,
        VkMemoryPropertyFlags flags);

  VkImage handle() const { return image_.get(); }
  const VkMemoryRequirements& memoryRequirements() const { return memoryRequirements_; }

  VkExtent3D extent() const { return extent_; }
  const MemorySlice& memorySlice() const { return memorySlice_; }

  uint32_t mipLevels() const { return mipLevels_; }
  uint32_t layers() const { return layers_; }

 private:
  static UniqueImage CreateImage(const Device& device, const VkImageCreateInfo& imageCI);
  static VkMemoryRequirements GetImageMemoryRequirements(const Device& device, VkImage image);
  static void BindImageMemory(const Device& device, VkImage image, const MemorySlice& memorySlice);

 private:
  const Device* device_;
  UniqueImage image_;
  VkImageUsageFlags usage_;
  VkExtent3D extent_;
  VkMemoryRequirements memoryRequirements_;
  VkSampleCountFlagBits samples_;
  uint32_t mipLevels_;
  uint32_t layers_;
  MemorySlice memorySlice_;
};

}  // namespace vkcore