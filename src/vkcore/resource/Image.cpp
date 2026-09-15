#include "Image.h"

#include "MemoryBindHelper.h"

namespace vkcore {

UniqueImage Image::CreateImage(const Device& device, const VkImageCreateInfo& imageCI) {
  const auto& dispatchTable = device.dispatchTable();

  VkImage imageRaw = VK_NULL_HANDLE;
  VkResult result = dispatchTable.vkCreateImage(device.handle(), &imageCI, nullptr, &imageRaw);
  SystemError::Check(result, "Failed to create image");

  return UniqueImage(imageRaw, {device.handle(), dispatchTable.vkDestroyImage});
}

VkMemoryRequirements Image::GetImageMemoryRequirements(const Device& device, VkImage image) {
  VkMemoryRequirements req = {};
  device.dispatchTable().vkGetImageMemoryRequirements(device.handle(), image, &req);
  return req;
}

void Image::BindImageMemory(const Device& device, VkImage image, const MemorySlice& memorySlice) {
  const auto& dispatchTable = device.dispatchTable();

  VkResult result = dispatchTable.vkBindImageMemory(device.handle(), image, memorySlice.memory(),
                                                    memorySlice.offset());

  SystemError::Check(result, "Failed to bind image memory");
}

Image::Image(const Device& device, const VkImageCreateInfo& imageCI, VkMemoryPropertyFlags flags)
    : device_(&device),
      image_(CreateImage(device, imageCI)),
      usage_(imageCI.usage),
      extent_(imageCI.extent),
      memoryRequirements_(GetImageMemoryRequirements(device, image_.get())),
      samples_(imageCI.samples),
      mipLevels_(imageCI.mipLevels),
      layers_(imageCI.arrayLayers),
      memorySlice_(detail::reserveDedicated(*this, device, flags)) {
  BindImageMemory(device, image_.get(), memorySlice_);
}

Image::Image(const Device& device, const VkImageCreateInfo& imageCI, MemoryAllocator& allocator,
             VkMemoryPropertyFlags flags)
    : device_(&device),
      image_(CreateImage(device, imageCI)),
      usage_(imageCI.usage),
      extent_(imageCI.extent),
      memoryRequirements_(GetImageMemoryRequirements(device, image_.get())),
      samples_(imageCI.samples),
      mipLevels_(imageCI.mipLevels),
      layers_(imageCI.arrayLayers),
      memorySlice_(detail::reservePooled(*this, allocator, flags)) {
  BindImageMemory(device, image_.get(), memorySlice_);
}

}  // namespace vkcore