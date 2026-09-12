#pragma once

#include <vulkan/vulkan.h>

#include <span>
#include <vector>

#include "../devices/Device.h"
#include "../resource/ImageView.h"

namespace vkcore {

class Framebuffer {
 public:
  Framebuffer(const Device& device, std::span<const ImageView* const> attachments,
              VkRenderPass renderPass, uint32_t width, uint32_t height, uint32_t layers = 1,
              VkFramebufferCreateFlags flags = 0);

  VkFramebuffer handle() const noexcept { return framebuffer_.get(); }

  uint32_t width() const noexcept { return width_; }
  uint32_t height() const noexcept { return height_; }
  uint32_t layers() const noexcept { return layers_; }
  VkFramebufferCreateFlags flags() const noexcept { return flags_; }

  const std::vector<const ImageView*>& attachments() const noexcept { return attachments_; }

 private:
  UniqueFramebuffer framebuffer_;
  uint32_t width_;
  uint32_t height_;
  uint32_t layers_;
  VkFramebufferCreateFlags flags_;
  VkRenderPass renderPass_;
  std::vector<const ImageView*> attachments_;
};

}  // namespace vkcore