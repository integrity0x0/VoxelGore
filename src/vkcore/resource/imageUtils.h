#pragma once

#include <span>

#include "../commands/CommandPool.h"
#include "../devices/Device.h"
#include "Buffer.h"
#include "Image.h"
#include "TransferContext.h"

namespace vkcore {

struct ImageSubresourceLayers : VkImageSubresourceLayers {
  ImageSubresourceLayers() : VkImageSubresourceLayers{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1u} {}

  ImageSubresourceLayers(VkImageAspectFlags aspectMask, uint32_t mipLevel = 0,
                         uint32_t baseArrayLayer = 0, uint32_t layerCount = 1u)
      : VkImageSubresourceLayers{aspectMask, mipLevel, baseArrayLayer, layerCount} {}
};

struct ImageCopyRegion {
  VkOffset3D offset{};
  VkExtent3D extent{};
  ImageSubresourceLayers subresource{};

  ImageCopyRegion() = default;

  ImageCopyRegion(const VkOffset3D& offset, const VkExtent3D& extent,
                  ImageSubresourceLayers subresource = {})
      : offset(offset), extent(extent), subresource(subresource) {}
};

struct ImageSubresourceRange : VkImageSubresourceRange {
  ImageSubresourceRange()
      : VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 1u} {}

  ImageSubresourceRange(VkImageAspectFlags aspectMask, uint32_t baseMipLevel = 0,
                        uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseArrayLayer = 0,
                        uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS)
      : VkImageSubresourceRange{aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount} {}
};

struct ImageTransitionInfo {
  VkImageLayout layout;
  VkAccessFlags accessFlags;
};

extern void TransitionImage(const Device& device, const CommandBuffer& commandBuffer,
                            const Image& image, ImageTransitionInfo src, ImageTransitionInfo dst,
                            const ImageSubresourceRange& range, VkPipelineStageFlags srcStage,
                            VkPipelineStageFlags dstStage);

extern void TransitionImage(const Device& device, const CommandBuffer& commandBuffer,
                            const Image& image, ImageTransitionInfo src, ImageTransitionInfo dst,
                            VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

extern void LoadDataToImage(const vkcore::Device& device, TransferContext& transferCtxt,
                            const std::span<const std::byte> data, const Image& dstImage,
                            const ImageCopyRegion& copyRegion);

extern void GenMipMaps(const Device& device, const CommandBuffer& commandBuffer,
                       const Image& dstImage, const ImageSubresourceRange& range,
                       ImageTransitionInfo dstTransition,
                       VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

extern void GenMipMaps(const Device& device, const CommandBuffer& commandBuffer,
                       const Image& dstImage, ImageTransitionInfo dstTransition,
                       VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}  // namespace vkcore