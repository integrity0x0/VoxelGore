#pragma once

#include <array>
#include <span>
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

  [[nodiscard]] const Image& GetImage() const { return image_; }
  [[nodiscard]] const ImageView& GetImageView() const { return imageView_; }

  [[nodiscard]] VkExtent3D GetExtent() const { return image_.GetExtent(); }
  [[nodiscard]] uint32_t GetWidth() const { return image_.GetExtent().width; }
  [[nodiscard]] uint32_t GetHeight() const { return image_.GetExtent().height; }
  [[nodiscard]] uint32_t GetMipLevels() const { return image_.GetMipLevels(); }

 private:
  Image image_;
  ImageView imageView_;
};


[[nodiscard]] extern Texture MakeTexture(const Device& device, TransferContext& transferCtxt, MemoryAllocator& allocator,
                                         std::span<const std::byte> pixels, uint32_t width, uint32_t height,
                                         uint32_t mipLevels = 1u);

[[nodiscard]] extern Texture MakeCubemap(const Device& device, TransferContext& transferCtxt, MemoryAllocator& allocator,
                                         const std::array<std::span<const std::byte>, 6>& faces, uint32_t size,
                                         uint32_t mipLevels = 1u);


}  // namespace vkcore