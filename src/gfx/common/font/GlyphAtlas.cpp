#include "GlyphAtlas.h"

#include "../../../vkcore/resource/imageUtils.h"

namespace gfx {
GlyphAtlas::GlyphAtlas(const vkcore::Device& device, vkcore::MemoryAllocator& memoryAllocator,
                       vkcore::TransferContext& transferCtxt)
    : device_(device),
      transferCtxt_(transferCtxt),
      texture_(CreateTexture(device, memoryAllocator)),
      regionAllocator_(kAtlasResolution, 1) {

  transferCtxt.Begin();

  vkcore::ImageTransitionInfo src(VK_IMAGE_LAYOUT_UNDEFINED, 0);
  vkcore::ImageTransitionInfo dst(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_ACCESS_SHADER_READ_BIT);

  vkcore::TransitionImage(device, transferCtxt.GetCmd(), texture_.GetImage(), src, dst,
                          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  transferCtxt.Flush();
}

std::optional<ImageRegion> GlyphAtlas::AddGlyph(glm::ivec2 size,
                                                std::span<const std::byte> pixels) {
  assert(!pixels.empty());
  auto region = regionAllocator_.Allocate(size);

  if (!region) return std::nullopt;

  vkcore::ImageSubresourceLayers subresource(VK_IMAGE_ASPECT_COLOR_BIT);

  vkcore::ImageCopyRegion copyRegion(VkOffset3D{region->pos.x, region->pos.y},
      VkExtent3D{static_cast<uint32_t>(region->size.x), static_cast<uint32_t>(region->size.y)},
      subresource);

  const vkcore::Image& image = texture_.GetImage();

  vkcore::ImageTransitionInfo src(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_ACCESS_SHADER_READ_BIT);
  vkcore::ImageTransitionInfo dst(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_ACCESS_TRANSFER_WRITE_BIT);

  VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  VkPipelineStageFlags dstStage = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

  vkcore::TransitionImage(device_, transferCtxt_.get().GetCmd(), image, src, dst,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  vkcore::LoadDataToImage(device_, transferCtxt_, pixels, image, copyRegion);

  std::swap(src, dst);
  std::swap(srcStage, dstStage);

  vkcore::TransitionImage(device_, transferCtxt_.get().GetCmd(), image, src, dst,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
}

vkcore::SampledTexture GlyphAtlas::CreateTexture(const vkcore::Device& device,
                                                 vkcore::MemoryAllocator& memoryAllocator) {
  VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  imageCI.extent = {kAtlasResolution.x, kAtlasResolution.y, 1};
  imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  imageCI.mipLevels = 1;
  imageCI.arrayLayers = 1;
  imageCI.imageType = VK_IMAGE_TYPE_2D;
  imageCI.format = VK_FORMAT_R8_UNORM;
  VkImageViewCreateInfo viewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  viewCI.format = imageCI.format;
  viewCI.components.r = VK_COMPONENT_SWIZZLE_ONE;
  viewCI.components.g = VK_COMPONENT_SWIZZLE_ONE;
  viewCI.components.b = VK_COMPONENT_SWIZZLE_ONE;
  viewCI.components.a = VK_COMPONENT_SWIZZLE_R;
  viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewCI.subresourceRange.layerCount = 1;
  viewCI.subresourceRange.levelCount = 1;
  viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;

  VkSamplerCreateInfo samplerCI = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

  return vkcore::SampledTexture(device, memoryAllocator,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageCI, viewCI, samplerCI);
}

}  // namespace gfx