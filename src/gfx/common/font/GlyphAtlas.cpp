#include "GlyphAtlas.h"

#include <cstring>
#include <stdexcept>

#include "../../../vkcore/resource/imageUtils.h"

namespace gfx {
GlyphAtlas::GlyphAtlas(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
                       vkcore::TransferContext& transferCtxt, MaterialManager& materialManager)
    : device_(device),
      memoryAllocator_(memoryAllocator),
      transferCtxt_(transferCtxt),
      materialManager_(materialManager) {
  if (!AddPage()) throw std::runtime_error("Unable to create glyph atlas page");
}

std::optional<GlyphSlot> GlyphAtlas::AddGlyph(glm::ivec2 size, std::span<const std::byte> pixels) {
  assert(size.x > 0 && size.y > 0);
  assert(pixels.size() == static_cast<size_t>(size.x) * static_cast<size_t>(size.y));

  const glm::ivec2 paddedSize = size + glm::ivec2(2 * kGlyphPadding);
  if (paddedSize.x > kAtlasResolution.x || paddedSize.y > kAtlasResolution.y) return std::nullopt;

  Page* page = nullptr;
  std::optional<ImageRegion> region;

  for (auto& it : pages_) {
    region = it->allocator.Allocate(paddedSize);
    if (region) {
      page = it.get();
      break;
    }
  }

  if (!region) {
    page = AddPage();
    if (!page) return std::nullopt;
    region = page->allocator.Allocate(paddedSize);
    if (!region) return std::nullopt;
  }

  paddedPixels_.assign(static_cast<size_t>(paddedSize.x) * paddedSize.y, std::byte{0});
  for (int32_t row = 0; row < size.y; ++row) {
    std::memcpy(paddedPixels_.data() + static_cast<size_t>(row + kGlyphPadding) * paddedSize.x +
                    kGlyphPadding,
                pixels.data() + static_cast<size_t>(row) * size.x, static_cast<size_t>(size.x));
  }

  Upload(*page, *region, paddedPixels_);

  const ImageRegion inner(region->pos + glm::ivec2(kGlyphPadding), size, 0);
  return GlyphSlot{.materialId = page->materialId, .region = inner.ToUv(kAtlasResolution)};
}

GlyphAtlas::Page* GlyphAtlas::AddPage() {
  if (pages_.size() >= kMaxPages) return nullptr;

  auto page = std::make_unique<Page>(CreateTexture());

  vkcore::TransferContext& transferCtxt = transferCtxt_.get();
  transferCtxt.Begin();
  vkcore::TransitionImage(device_, transferCtxt.GetCmd(), page->texture.GetImage(),
                          vkcore::ImageTransitionInfo(VK_IMAGE_LAYOUT_UNDEFINED, 0),
                          vkcore::ImageTransitionInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                      VK_ACCESS_SHADER_READ_BIT),
                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  transferCtxt.Flush();

  page->materialId = materialManager_.get().Add(page->texture);
  pages_.push_back(std::move(page));
  return pages_.back().get();
}

void GlyphAtlas::Upload(Page& page, const ImageRegion& region, std::span<const std::byte> pixels) {
  vkcore::TransferContext& transferCtxt = transferCtxt_.get();
  const vkcore::Image& image = page.texture.GetImage();

  const vkcore::ImageCopyRegion copyRegion(
      VkOffset3D{region.pos.x, region.pos.y, 0},
      VkExtent3D{static_cast<uint32_t>(region.size.x), static_cast<uint32_t>(region.size.y), 1},
      vkcore::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT));

  const vkcore::ImageTransitionInfo shaderRead(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                               VK_ACCESS_SHADER_READ_BIT);
  const vkcore::ImageTransitionInfo transferDst(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                VK_ACCESS_TRANSFER_WRITE_BIT);

  transferCtxt.Begin();

  vkcore::TransitionImage(device_, transferCtxt.GetCmd(), image, shaderRead, transferDst,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  vkcore::LoadDataToImage(device_, transferCtxt, pixels, image, copyRegion);

  vkcore::TransitionImage(device_, transferCtxt.GetCmd(), image, transferDst, shaderRead,
                          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  transferCtxt.Flush();
}

vkcore::SampledTexture GlyphAtlas::CreateTexture() const {
  VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  imageCI.imageType = VK_IMAGE_TYPE_2D;
  imageCI.format = VK_FORMAT_R8_UNORM;
  imageCI.extent = {static_cast<uint32_t>(kAtlasResolution.x),
                    static_cast<uint32_t>(kAtlasResolution.y), 1};
  imageCI.mipLevels = 1;
  imageCI.arrayLayers = 1;
  imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VkImageViewCreateInfo viewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCI.format = imageCI.format;
  viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewCI.subresourceRange.levelCount = 1;
  viewCI.subresourceRange.layerCount = 1;

  VkSamplerCreateInfo samplerCI = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  samplerCI.magFilter = VK_FILTER_LINEAR;
  samplerCI.minFilter = VK_FILTER_LINEAR;
  samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  samplerCI.addressModeU = samplerCI.addressModeV = samplerCI.addressModeW =
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerCI.maxLod = 0.0f;

  return vkcore::SampledTexture(device_, memoryAllocator_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                imageCI, viewCI, samplerCI);
}
}  // namespace gfx