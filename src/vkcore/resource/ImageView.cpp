#include "ImageView.h"

namespace vkcore {

ImageView::ImageView(const Device& device, const VkImageViewCreateInfo& imageViewCI)
    : device_(&device),
      format_(imageViewCI.format),
      viewType_(imageViewCI.viewType),
      subresourceRange_(imageViewCI.subresourceRange) {
  VkImageView rawView = VK_NULL_HANDLE;
  VkResult result =
      device.dispatchTable().vkCreateImageView(device.handle(), &imageViewCI, nullptr, &rawView);
  SystemError::Check(result, "failed to create image view");

  ImageViewDeleter deleter{device.handle(), device.dispatchTable().vkDestroyImageView};
  imageView_ = UniqueImageView(rawView, deleter);
}

}  // namespace vkcore