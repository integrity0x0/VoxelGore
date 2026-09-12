#include "Atlas.h"

#include <algorithm>
#include <cstring>

#include "../../vkcore/resource/imageUtils.h"
#include "../../util/files.h"
#include "stb_image.h"

namespace gfx {

vkcore::SampledTexture Atlas::CreateTexture(glm::ivec2 size, uint32_t mipLevels,
                                            uint32_t arrayLayers) {
  return vkcore::SampledTexture(
      *device_, *memoryAllocator_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VkImageCreateInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                        .imageType = VK_IMAGE_TYPE_2D,
                        .format = VK_FORMAT_R8G8B8A8_SRGB,
                        .extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1},
                        .mipLevels = mipLevels,
                        .arrayLayers = arrayLayers,
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .tiling = VK_IMAGE_TILING_OPTIMAL,
                        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                 VK_IMAGE_USAGE_SAMPLED_BIT,
                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED},
      VkImageViewCreateInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                            .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                            .format = VK_FORMAT_R8G8B8A8_SRGB,
                            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                              .baseMipLevel = 0,
                                              .levelCount = mipLevels,
                                              .baseArrayLayer = 0,
                                              .layerCount = arrayLayers}},
      VkSamplerCreateInfo{.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                          .magFilter = VK_FILTER_NEAREST,
                          .minFilter = VK_FILTER_NEAREST,
                          .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                          .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                          .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                          .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                          .maxLod = static_cast<float>(mipLevels)});
}

Atlas::Atlas(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
             vkcore::MemoryAllocator& allocator, glm::ivec2 size, uint32_t mipLevels,
             uint32_t arrayLayers, uint32_t padding)
    : device_(&device),
      transferCtxt_(&transferCtxt),
      memoryAllocator_(&allocator),
      size_(size),
      mipLevels_(mipLevels),
      padding_(padding),
      texture_(CreateTexture(size, mipLevels, arrayLayers)),
      regionAllocator_(size, arrayLayers) {
  transferCtxt.Begin();
  vkcore::ImageTransitionInfo src(VK_IMAGE_LAYOUT_UNDEFINED, 0);
  vkcore::ImageTransitionInfo dst(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_ACCESS_SHADER_READ_BIT);
  vkcore::TransitionImage(device, transferCtxt.cmd(), texture_.image(), src, dst,
                          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

  transferCtxt.Flush();
}

const AtlasRegion* Atlas::Load(std::string_view path, std::string_view key) {
  if (const AtlasRegion* region = Find(key)) {
    return region;
  }

  std::vector<std::byte> fileBytes = util::ReadFileBytes(path);
  if (fileBytes.empty()) {
    return nullptr;
  }

  int w, h, channels;
  std::unique_ptr<std::byte[]> pixels(reinterpret_cast<std::byte*>(stbi_load_from_memory(
      reinterpret_cast<stbi_uc*>(fileBytes.data()), static_cast<int>(fileBytes.size()), &w, &h,
      &channels, STBI_rgb_alpha)));

  if (!pixels) {
    throw std::runtime_error("failed to load region: " + std::string(path));
  }

  glm::ivec2 paddedSize(w + padding_ * 2, h + padding_ * 2);
  std::optional<ImageRegion> paddedRegion = regionAllocator_.reserve(paddedSize);
  if (!paddedRegion) {
    throw std::runtime_error("failed to reserve");
  }

  const size_t dataSize = static_cast<size_t>(paddedSize.x) * paddedSize.y * 4;
  std::vector<std::byte> paddedPixels(dataSize);

  for (int32_t y = 0; y < paddedSize.y; ++y) {
    for (int32_t x = 0; x < paddedSize.x; ++x) {
      const int32_t srcX = std::clamp<int32_t>(x - padding_, 0, w - 1);
      const int32_t srcY = std::clamp<int32_t>(y - padding_, 0, h - 1);
      const size_t srcIndex = (static_cast<size_t>(srcY) * w + srcX) * 4;
      const size_t dstIndex = (static_cast<size_t>(y) * paddedSize.x + x) * 4;

      std::memcpy(paddedPixels.data() + dstIndex, pixels.get() + srcIndex, 4);
    }
  }

  ImageRegion region = *paddedRegion;
  region.pos += glm::ivec2(padding_);
  region.size -= glm::ivec2(padding_ * 2);

  AtlasRegion atlasRegion(region, size_);
  std::span<std::byte> pixelData(paddedPixels.data(), paddedPixels.size());

  vkcore::ImageTransitionInfo src(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_ACCESS_SHADER_READ_BIT);
  vkcore::ImageTransitionInfo dst(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_ACCESS_TRANSFER_WRITE_BIT);

  VkOffset3D imageOffset = {paddedRegion->pos.x, paddedRegion->pos.y, 0};
  VkExtent3D imageExtent = {static_cast<uint32_t>(paddedSize.x),
                            static_cast<uint32_t>(paddedSize.y), 1};

  vkcore::ImageCopyRegion copyRegion(
      imageOffset, imageExtent,
      vkcore::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0, region.arrayLayer));

  transferCtxt_->Begin();

  vkcore::TransitionImage(*device_, transferCtxt_->cmd(), texture_.image(), src, dst,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  vkcore::LoadDataToImage(*device_, *transferCtxt_, pixelData, texture_.image(), copyRegion);

  dst = vkcore::ImageTransitionInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                    VK_ACCESS_SHADER_READ_BIT);
  vkcore::GenMipMaps(*device_, transferCtxt_->cmd(), texture_.image(), dst);

  transferCtxt_->Flush();

  const auto& it = regions_.emplace(std::string(key), atlasRegion).first;

  return &it->second;
}


const AtlasRegion* Atlas::Require(std::string_view key) {
  auto it = regions_.find(key);
  if (it != regions_.end()) {
    return &it->second;
  }

  if (!Load(key, key)) {
    return nullptr;
  }

  it = regions_.find(key);
  return (it != regions_.end()) ? &it->second : nullptr;
}

}  // namespace gfx