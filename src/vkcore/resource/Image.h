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

  [[nodiscard]] VkImage GetHandle() const { return image_.get(); }
  [[nodiscard]] const VkMemoryRequirements& memoryRequirements() const { return memoryRequirements_; }

  [[nodiscard]] VkExtent3D GetExtent() const { return extent_; }
  [[nodiscard]] const MemorySlice& GetMemorySlice() const { return memorySlice_; }

  [[nodiscard]] uint32_t GetMipLevels() const { return mipLevels_; }
  [[nodiscard]] uint32_t GetLayers() const { return layers_; }

 private:
  [[nodiscard]] static UniqueImage CreateImage(const Device& device,
                                               const VkImageCreateInfo& imageCI);
  [[nodiscard]] static VkMemoryRequirements GetImageMemoryRequirements(const Device& device,
                                                                       VkImage image);

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