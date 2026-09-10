#include "Atlas.h"

#include "../../vkcore/resource/imageUtils.h"
#include "stb_image.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
extern AAssetManager* g_AAssetManager;
#endif

namespace gfx {

namespace {
std::vector<uint8_t> readFileBytes(std::string_view path) {
#if defined(__ANDROID__)
  AAsset* asset = AAssetManager_open(::g_AAssetManager, path.data(), AASSET_MODE_BUFFER);
  if (!asset) {
    return {};
  }

  off_t length = AAsset_getLength(asset);
  std::vector<uint8_t> buffer(static_cast<size_t>(length));

  int readBytes = AAsset_read(asset, buffer.data(), static_cast<size_t>(length));
  AAsset_close(asset);

  if (readBytes < 0 || static_cast<off_t>(readBytes) != length) {
    return {};
  }
  return buffer;
#else
  FILE* f = fopen(path.data(), "rb");
  if (!f) {
    return {};
  }
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  std::vector<uint8_t> buffer(static_cast<size_t>(size));
  size_t readBytes = fread(buffer.data(), 1, static_cast<size_t>(size), f);
  fclose(f);

  if (readBytes != static_cast<size_t>(size)) {
    return {};
  }
  return buffer;
#endif
}
}  // namespace

Atlas::Atlas(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
             vkcore::MemoryAllocator& allocator, glm::ivec2 size, uint32_t mipLevels,
             uint32_t arrayLayers)
    : device_(&device),
      transferCtxt_(&transferCtxt),
      memoryAllocator_(&allocator),
      size_(size),
      mipLevels_(mipLevels),
      texture_(device, allocator, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
               VkImageCreateInfo{
                   .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
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
                                   .maxLod = static_cast<float>(mipLevels)}),
      regionAllocator_(size, arrayLayers) {
  transferCtxt.Begin();

  vkcore::ImageTransitionInfo src(VK_IMAGE_LAYOUT_UNDEFINED, 0u);
  vkcore::ImageTransitionInfo dst(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_ACCESS_SHADER_READ_BIT);
  vkcore::TransitionImage(device, transferCtxt.cmd(), texture_.image(), src, dst,
                          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

  transferCtxt.Flush();
}

bool Atlas::load(std::string_view path, std::string_view key) {
  std::vector<uint8_t> fileBytes = readFileBytes(path);
  if (fileBytes.empty()) {
    return false;
  }

  int w, h, channels;
  std::unique_ptr<stbi_uc> pixels = std::unique_ptr<stbi_uc>(stbi_load_from_memory(
      fileBytes.data(), static_cast<int>(fileBytes.size()), &w, &h, &channels, STBI_rgb_alpha));
  if (!pixels) {
    throw std::runtime_error("failed to load region: " + std::string(path));
    return false;
  }

  glm::ivec2 imgSize(w, h);

  std::optional<ImageRegion> regionOpt = regionAllocator_.reserve(imgSize);
  if (!regionOpt.has_value()) {
    throw std::runtime_error("failed to reserve");
  }

  AtlasRegion atlasRegion = AtlasRegion(*regionOpt, size_);
  VkDeviceSize dataSize = static_cast<VkDeviceSize>(w) * static_cast<VkDeviceSize>(h) * 4ull;

  std::span<uint8_t> pixelData(pixels.get(), dataSize);

  vkcore::ImageTransitionInfo src(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_ACCESS_SHADER_READ_BIT);
  vkcore::ImageTransitionInfo dst(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_ACCESS_TRANSFER_WRITE_BIT);

  const auto& region = regionOpt.value();

  VkOffset3D imageOffset = {region.pos.x, region.pos.y, 0};
  VkExtent3D imageExtent = {static_cast<uint32_t>(w), static_cast<uint32_t>(h), 1};

  vkcore::ImageCopyRegion copyRegion(
      imageOffset, imageExtent,
      vkcore::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0u, region.arrayLayer));

  transferCtxt_->Begin();

  vkcore::TransitionImage(*device_, transferCtxt_->cmd(), texture_.image(), src, dst,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  vkcore::LoadDataToImage(*device_, *transferCtxt_, pixelData, texture_.image(), copyRegion);

  dst = vkcore::ImageTransitionInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                    VK_ACCESS_SHADER_READ_BIT);

  vkcore::GenMipMaps(*device_, transferCtxt_->cmd(), texture_.image(), dst);

  transferCtxt_->Flush();

  regions_.insert_or_assign(std::string(key), atlasRegion);
  return true;
}

const AtlasRegion* Atlas::get(std::string_view key) {
  auto it = regions_.find(std::string(key));
  if (it != regions_.end()) {
    return &it->second;
  }

  if (!load(key, key)) {
    return nullptr;
  }

  it = regions_.find(std::string(key));
  return (it != regions_.end()) ? &it->second : nullptr;
}

}  // namespace gfx