#include "imageUtils.h"

namespace vkcore {
void TransitionImage(const Device& device, const CommandBuffer& commandBuffer, const Image& image,
                     ImageTransitionInfo src, ImageTransitionInfo dst,
                     const ImageSubresourceRange& range, VkPipelineStageFlags srcStage,
                     VkPipelineStageFlags dstStage) {
  VkCommandBuffer cmd = commandBuffer.handle();

  VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.image = image.handle();
  barrier.oldLayout = src.layout;
  barrier.srcAccessMask = src.accessFlags;
  barrier.newLayout = dst.layout;
  barrier.dstAccessMask = dst.accessFlags;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange = range;

  device.dispatchTable().vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr,
                                              1, &barrier);
}

void TransitionImage(const Device& device, const CommandBuffer& commandBuffer, const Image& image,
                     ImageTransitionInfo src, ImageTransitionInfo dst,
                     VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) {
  ImageSubresourceRange range(VK_IMAGE_ASPECT_COLOR_BIT, 0, image.mipLevels(), 0, image.layers());
  TransitionImage(device, commandBuffer, image, src, dst, range, srcStage, dstStage);
}

void LoadDataToImage(const vkcore::Device& device,
                     TransferContext& transferCtxt,  // requires image be in TRANSFER_DST_OPTIMAL
                     std::span<const std::byte> data, const Image& dstImage,
                     const ImageCopyRegion& copyRegion) {
  TransferContext::Allocation allocation =
      transferCtxt.AllocateStagingBuffer(data.size());  // if overflowed, returns nullopt

  VkCommandBuffer cmd = transferCtxt.cmd().handle();

  VkBufferImageCopy region = {};
  region.imageOffset = copyRegion.offset;
  region.imageExtent = copyRegion.extent;
  region.imageSubresource = copyRegion.subresource;
  region.bufferOffset = allocation.bufferOffset;

  std::memcpy(reinterpret_cast<uint8_t*>(allocation.mapped), data.data(), data.size());

  device.dispatchTable().vkCmdCopyBufferToImage(cmd, allocation.buffer, dstImage.handle(),
                                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void GenMipMaps(const Device& device, const CommandBuffer& commandBuffer,
                const Image& dstImage,  // requires image be in TRANSFER_DST_OPTIMAL
                const ImageSubresourceRange& range, ImageTransitionInfo dstTransition,
                VkPipelineStageFlags dstStage) {
  const auto& dt = device.dispatchTable();
  VkCommandBuffer cmd = commandBuffer.handle();

  VkExtent3D mipExtent = dstImage.extent();

  mipExtent.width = std::max(1u, mipExtent.width / (1 << range.baseMipLevel));
  mipExtent.height = std::max(1u, mipExtent.height / (1 << range.baseMipLevel));
  mipExtent.depth = std::max(1u, mipExtent.depth / (1 << range.baseMipLevel));

  VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.image = dstImage.handle();
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  for (uint32_t i = range.baseMipLevel + 1u; i < range.baseMipLevel + range.levelCount; ++i) {
    barrier.subresourceRange = {
        range.aspectMask, i - 1u, 1u, range.baseArrayLayer, range.layerCount,
    };

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    dt.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                            0, nullptr, 0, nullptr, 1u, &barrier);

    VkImageBlit blit = {};

    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {
        static_cast<int32_t>(mipExtent.width),
        static_cast<int32_t>(mipExtent.height),
        static_cast<int32_t>(mipExtent.depth),
    };

    blit.srcSubresource = {
        range.aspectMask,
        i - 1,
        range.baseArrayLayer,
        range.layerCount,
    };

    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {
        static_cast<int32_t>(std::max(1u, mipExtent.width / 2u)),
        static_cast<int32_t>(std::max(1u, mipExtent.height / 2u)),
        static_cast<int32_t>(std::max(1u, mipExtent.depth / 2u)),
    };

    blit.dstSubresource = {
        range.aspectMask,
        i,
        range.baseArrayLayer,
        range.layerCount,
    };

    dt.vkCmdBlitImage(cmd, dstImage.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      dstImage.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                      VK_FILTER_LINEAR);

    barrier.subresourceRange = {
        range.aspectMask, i - 1, 1, range.baseArrayLayer, range.layerCount,
    };

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = dstTransition.layout;

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = dstTransition.accessFlags;

    dt.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, dstStage, 0, 0, nullptr, 0,
                            nullptr, 1u, &barrier);

    mipExtent.width = std::max(1u, mipExtent.width / 2u);
    mipExtent.height = std::max(1u, mipExtent.height / 2u);
    mipExtent.depth = std::max(1u, mipExtent.depth / 2u);
  }

  barrier.subresourceRange = {
      range.aspectMask, range.baseMipLevel + range.levelCount - 1u, 1u, range.baseArrayLayer,
      range.layerCount,
  };

  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = dstTransition.layout;

  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = dstTransition.accessFlags;

  dt.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, dstStage, 0, 0, nullptr, 0,
                          nullptr, 1u, &barrier);
}

void GenMipMaps(const Device& device, const CommandBuffer& commandBuffer, const Image& dstImage,
                ImageTransitionInfo dstTransition, VkPipelineStageFlags dstStage) {
  ImageSubresourceRange range(VK_IMAGE_ASPECT_COLOR_BIT, 0, dstImage.mipLevels(), 0,
                              dstImage.layers());

  GenMipMaps(device, commandBuffer, dstImage, range, dstTransition, dstStage);
}
}  // namespace vkcore