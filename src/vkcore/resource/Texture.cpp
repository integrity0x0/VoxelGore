#include "Texture.h"

#include <stdexcept>

namespace vkcore {

namespace {

ImageView CreateImageView(const Device& device, const Image& image, VkImageViewCreateInfo viewCI) {
  viewCI.image = image.GetHandle();
  return ImageView(device, viewCI);
}

VkImageCreateInfo MakeTextureImageCI(uint32_t width, uint32_t height, uint32_t mipLevels,
                                     uint32_t arrayLayers = 1u, VkImageCreateFlags flags = 0) {
  VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};

  imageCI.flags = flags;
  imageCI.imageType = VK_IMAGE_TYPE_2D;
  imageCI.format = VK_FORMAT_R8G8B8A8_SRGB;
  imageCI.extent = {width, height, 1u};
  imageCI.mipLevels = mipLevels;
  imageCI.arrayLayers = arrayLayers;
  imageCI.samples = VK_SAMPLE_COUNT_1_BIT;

  imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  return imageCI;
}

Texture FinishTexture(const Device& device, TransferContext& transferCtxt, Image&& image,
                      const VkImageCreateInfo& imageCI, std::span<const std::byte> data) {
  transferCtxt.Begin();

  ImageTransitionInfo src(VK_IMAGE_LAYOUT_UNDEFINED, 0);
  ImageTransitionInfo dst(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);

  ImageCopyRegion region({}, imageCI.extent, ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT));

  TransitionImage(device, transferCtxt.GetCmd(), image, src, dst,
                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  LoadDataToImage(device, transferCtxt, data, image, region);

  dst = ImageTransitionInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
  GenMipMaps(device, transferCtxt.GetCmd(), image, dst, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  transferCtxt.Flush();

  VkImageViewCreateInfo imageViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  imageViewCI.image = image.GetHandle();
  imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCI.format = imageCI.format;
  imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageViewCI.subresourceRange.baseMipLevel = 0;
  imageViewCI.subresourceRange.levelCount = imageCI.mipLevels;
  imageViewCI.subresourceRange.baseArrayLayer = 0;
  imageViewCI.subresourceRange.layerCount = 1u;

  ImageView imageView(device, imageViewCI);
  return Texture(device, std::move(image), std::move(imageView));
}

Texture FinishCubemap(const Device& device, TransferContext& transferCtxt, Image&& image,
                      const VkImageCreateInfo& imageCI,
                      const std::array<std::span<const std::byte>, 6>& faces) {
  transferCtxt.Begin();

  ImageTransitionInfo src(VK_IMAGE_LAYOUT_UNDEFINED, 0);
  ImageTransitionInfo dst(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);

  TransitionImage(device, transferCtxt.GetCmd(), image, src, dst,
                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  for (uint32_t face = 0; face < 6u; ++face) {
    ImageCopyRegion region({}, imageCI.extent,
                           ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0, face, 1u));
    LoadDataToImage(device, transferCtxt, faces[face], image, region);
  }

  dst = ImageTransitionInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
  GenMipMaps(device, transferCtxt.GetCmd(), image, dst, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  transferCtxt.Flush();

  VkImageViewCreateInfo imageViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  imageViewCI.image = image.GetHandle();
  imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  imageViewCI.format = imageCI.format;
  imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageViewCI.subresourceRange.baseMipLevel = 0;
  imageViewCI.subresourceRange.levelCount = imageCI.mipLevels;
  imageViewCI.subresourceRange.baseArrayLayer = 0;
  imageViewCI.subresourceRange.layerCount = 6u;

  ImageView imageView(device, imageViewCI);
  return Texture(device, std::move(image), std::move(imageView));
}

}  // namespace

Texture::Texture(const Device& device, Image&& image, ImageView&& imageView)
    : image_(std::move(image)), imageView_(std::move(imageView)) {}

Texture::Texture(const Device& device, MemoryAllocator& memoryAllocator,
                 VkMemoryPropertyFlags memoryProperties, const VkImageCreateInfo& imageCI,
                 const VkImageViewCreateInfo& viewCI)
    : image_(Image(device, imageCI, memoryAllocator, memoryProperties)),
      imageView_(CreateImageView(device, image_, viewCI)) {}

Texture::Texture(const Device& device, VkMemoryPropertyFlags memoryProperties,
                 const VkImageCreateInfo& imageCI, const VkImageViewCreateInfo& viewCI)
    : image_(Image(device, imageCI, memoryProperties)),
      imageView_(CreateImageView(device, image_, viewCI)) {}

Texture MakeTexture(const Device& device, TransferContext& transferCtxt, MemoryAllocator& allocator,
                    std::span<const std::byte> pixels, uint32_t width, uint32_t height,
                    uint32_t mipLevels) {
  const size_t expectedSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
  if (pixels.size() < expectedSize) {
    throw std::runtime_error("MakeTexture: pixel data too small");
  }

  const VkImageCreateInfo imageCI = MakeTextureImageCI(width, height, mipLevels);
  Image image(device, imageCI, allocator, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  return FinishTexture(device, transferCtxt, std::move(image), imageCI, pixels);
}

Texture MakeCubemap(const Device& device, TransferContext& transferCtxt, MemoryAllocator& allocator,
                    const std::array<std::span<const std::byte>, 6>& faces, uint32_t size,
                    uint32_t mipLevels) {
  const size_t expectedSize = static_cast<size_t>(size) * static_cast<size_t>(size) * 4u;
  for (uint32_t i = 0; i < 6u; ++i) {
    if (faces[i].size() < expectedSize) {
      throw std::runtime_error("MakeCubemap: face pixel data too small");
    }
  }

  const VkImageCreateInfo imageCI =
      MakeTextureImageCI(size, size, mipLevels, 6u, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
  Image image(device, imageCI, allocator, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  return FinishCubemap(device, transferCtxt, std::move(image), imageCI, faces);
}

}  // namespace vkcore