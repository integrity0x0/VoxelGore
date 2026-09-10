#include "Framebuffer.h"

namespace vkcore {

Framebuffer::Framebuffer(const Device& device, std::span<const ImageView* const> attachments,
                         VkRenderPass renderPass, uint32_t width, uint32_t height, uint32_t layers,
                         VkFramebufferCreateFlags flags)
    : width_(width),
      height_(height),
      layers_(layers),
      flags_(flags),
      renderPass_(renderPass),
      attachments_(attachments.begin(), attachments.end()) {
  std::vector<VkImageView> viewHandles;
  viewHandles.reserve(attachments_.size());
  for (const ImageView* view : attachments_) {
    viewHandles.push_back(view->handle());
  }

  VkFramebufferCreateInfo framebufferCI = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  framebufferCI.flags = flags_;
  framebufferCI.renderPass = renderPass_;
  framebufferCI.attachmentCount = static_cast<uint32_t>(viewHandles.size());
  framebufferCI.pAttachments = viewHandles.data();
  framebufferCI.width = width_;
  framebufferCI.height = height_;
  framebufferCI.layers = layers_;

  VkFramebuffer rawFramebuffer = VK_NULL_HANDLE;
  VkResult result = device.dispatchTable().vkCreateFramebuffer(device.handle(), &framebufferCI,
                                                               nullptr, &rawFramebuffer);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to create framebuffer");
  }

  FramebufferDeleter deleter{device.handle(), device.dispatchTable().vkDestroyFramebuffer};
  framebuffer_ = UniqueFramebuffer(rawFramebuffer, deleter);
}

}  // namespace vkcore