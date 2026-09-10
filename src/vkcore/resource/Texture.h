#pragma once

#include <string_view>

#include "../commands/CommandPool.h"
#include "Image.h"
#include "ImageView.h"
#include "imageUtils.h"

namespace vkcore {

class Texture {
 public:
  Texture(const Device& device, Image&& image, ImageView&& imageView);
  Texture(const Device& device, MemoryAllocator& memoryAllocator,
          VkMemoryPropertyFlags memoryProperties, const VkImageCreateInfo& imageCI,
          const VkImageViewCreateInfo& viewCI);
  Texture(const Device& device, VkMemoryPropertyFlags memoryProperties,
          const VkImageCreateInfo& imageCI, const VkImageViewCreateInfo& viewCI);

  const Image& getImage() const { return image_; }
  const ImageView& imageView() const { return imageView_; }

  VkExtent3D extent() const { return image_.extent(); }
  uint32_t width() const { return image_.extent().width; }
  uint32_t height() const { return image_.extent().height; }
  uint32_t mipLevels() const { return image_.mipLevels(); }

 private:
  Image image_;
  ImageView imageView_;
};

extern Texture ImportTexture(const Device& device, TransferContext& transferCtxt,
                             MemoryAllocator& allocator, std::string_view path,
                             uint32_t mipLevels = 1u);

extern Texture ImportCubemap(const Device& device, TransferContext& transferCtxt,
                             MemoryAllocator& allocator, const std::array<std::string, 6>& paths,
                             uint32_t mipLevels = 1u);

}  // namespace vkcore