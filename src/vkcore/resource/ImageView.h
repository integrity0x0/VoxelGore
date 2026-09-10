#pragma once

#include "../devices/Device.h"

namespace vkcore {

class ImageView {
 public:
  ImageView(const Device& device, const VkImageViewCreateInfo& imageViewCI);

  VkImageView handle() const noexcept { return imageView_.get(); }
  VkFormat format() const noexcept { return format_; }
  VkImageViewType viewType() const noexcept { return viewType_; }
  VkImageSubresourceRange subresourceRange() const noexcept { return subresourceRange_; }

  ImageView(const ImageView&) = delete;
  ImageView& operator=(const ImageView&) = delete;

  ImageView(ImageView&&) noexcept = default;
  ImageView& operator=(ImageView&&) = delete;

 private:
  const Device* device_;
  UniqueImageView imageView_;
  VkFormat format_;
  VkImageViewType viewType_;
  VkImageSubresourceRange subresourceRange_;
};

}  // namespace vkcore