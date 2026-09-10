#include "Texture.h"

#include <stb_image.h>

#include <stdexcept>
#include <vector>

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <android/asset_manager.h>

extern AAssetManager* g_AAssetManager;
#endif

namespace vkcore {

namespace {

struct LoadedImage {
  int32_t width = 0;
  int32_t height = 0;
  std::vector<stbi_uc> pixels;
};

ImageView CreateImageView(const Device& device, const Image& image, VkImageViewCreateInfo viewCI) {
  viewCI.image = image.handle();
  return ImageView(device, viewCI);
}

LoadedImage LoadImagePixels(std::string_view path) {
  LoadedImage result;

  int32_t nrChannels = 0;
  stbi_uc* loadedImage = nullptr;

#ifdef VK_USE_PLATFORM_ANDROID_KHR

  AAsset* asset = AAssetManager_open(g_AAssetManager, path.data(), AASSET_MODE_BUFFER);

  if (!asset) {
    throw std::runtime_error("failed to open asset: " + std::string(path));
  }

  const off_t assetLength = AAsset_getLength(asset);

  std::vector<uint8_t> fileBuffer(static_cast<size_t>(assetLength));

  const int readResult = AAsset_read(asset, fileBuffer.data(), assetLength);

  AAsset_close(asset);

  if (readResult < 0) {
    throw std::runtime_error("failed to read asset: " + std::string(path));
  }

  loadedImage = stbi_load_from_memory(fileBuffer.data(), static_cast<int>(fileBuffer.size()),
                                      &result.width, &result.height, &nrChannels, STBI_rgb_alpha);

#else

  loadedImage = stbi_load(path.data(), &result.width, &result.height, &nrChannels, STBI_rgb_alpha);

#endif

  if (!loadedImage) {
    throw std::runtime_error("failed to load texture: " + std::string(path));
  }

  const size_t pixelCount = static_cast<size_t>(result.width) * static_cast<size_t>(result.height);

  result.pixels.assign(loadedImage, loadedImage + pixelCount * 4u);

  stbi_image_free(loadedImage);

  return result;
}

VkImageCreateInfo MakeTextureImageCI(uint32_t width, uint32_t height, uint32_t mipLevels,
                                     uint32_t arrayLayers = 1u, VkImageCreateFlags flags = 0u) {
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
                      const VkImageCreateInfo& imageCI, std::span<const stbi_uc> data) {
  transferCtxt.Begin();

  ImageTransitionInfo src(VK_IMAGE_LAYOUT_UNDEFINED, 0u);

  ImageTransitionInfo dst(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);

  ImageCopyRegion region({}, imageCI.extent, ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT));

  TransitionImage(device, transferCtxt.cmd(), image, src, dst, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                  VK_PIPELINE_STAGE_TRANSFER_BIT);

  LoadDataToImage(device, transferCtxt, data, image, region);

  dst = ImageTransitionInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);

  GenMipMaps(device, transferCtxt.cmd(), image, dst, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  transferCtxt.Flush();

  VkImageViewCreateInfo imageViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

  imageViewCI.image = image.handle();
  imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCI.format = imageCI.format;

  imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  imageViewCI.subresourceRange.baseMipLevel = 0u;
  imageViewCI.subresourceRange.levelCount = imageCI.mipLevels;

  imageViewCI.subresourceRange.baseArrayLayer = 0u;
  imageViewCI.subresourceRange.layerCount = 1u;

  ImageView imageView(device, imageViewCI);

  return Texture(device, std::move(image), std::move(imageView));
}

Texture FinishCubemap(const Device& device, TransferContext& transferCtxt, Image&& image,
                      const VkImageCreateInfo& imageCI, std::span<const LoadedImage, 6> faces) {
  transferCtxt.Begin();

  ImageTransitionInfo src(VK_IMAGE_LAYOUT_UNDEFINED, 0u);

  ImageTransitionInfo dst(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);

  TransitionImage(device, transferCtxt.cmd(), image, src, dst, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                  VK_PIPELINE_STAGE_TRANSFER_BIT);

  for (uint32_t face = 0u; face < 6u; ++face) {
    ImageCopyRegion region({}, imageCI.extent,
                           ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0u, face, 1u));

    LoadDataToImage(device, transferCtxt, faces[face].pixels, image, region);
  }

  dst = ImageTransitionInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);

  GenMipMaps(device, transferCtxt.cmd(), image, dst, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  transferCtxt.Flush();

  VkImageViewCreateInfo imageViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  imageViewCI.image = image.handle();
  imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  imageViewCI.format = imageCI.format;
  imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageViewCI.subresourceRange.baseMipLevel = 0u;
  imageViewCI.subresourceRange.levelCount = imageCI.mipLevels;
  imageViewCI.subresourceRange.baseArrayLayer = 0u;
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

Texture ImportTexture(const Device& device, TransferContext& transferCtxt,
                      MemoryAllocator& allocator, std::string_view path, uint32_t mipLevels) {
  const LoadedImage loadedImage = LoadImagePixels(path);

  const VkImageCreateInfo imageCI =
      MakeTextureImageCI(static_cast<uint32_t>(loadedImage.width),
                         static_cast<uint32_t>(loadedImage.height), mipLevels);

  Image image(device, imageCI, allocator, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  return FinishTexture(device, transferCtxt, std::move(image), imageCI, loadedImage.pixels);
}

Texture ImportCubemap(const Device& device, TransferContext& transferCtxt,
                      MemoryAllocator& allocator, const std::array<std::string, 6>& paths,
                      uint32_t mipLevels) {
  std::array<LoadedImage, 6> faces;

  for (uint32_t i = 0u; i < 6u; ++i) {
    faces[i] = LoadImagePixels(paths[i]);
  }

  const uint32_t width = static_cast<uint32_t>(faces[0].width);

  const uint32_t height = static_cast<uint32_t>(faces[0].height);

  for (uint32_t i = 1u; i < 6u; ++i) {
    if (faces[i].width != faces[0].width || faces[i].height != faces[0].height) {
      throw std::runtime_error("cubemap faces have different dimensions");
    }
  }

  if (width != height) {
    throw std::runtime_error("cubemap faces must be square");
  }

  const VkImageCreateInfo imageCI =
      MakeTextureImageCI(width, height, mipLevels, 6u, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

  Image image(device, imageCI, allocator, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  return FinishCubemap(device, transferCtxt, std::move(image), imageCI, faces);
}

}  // namespace vkcore