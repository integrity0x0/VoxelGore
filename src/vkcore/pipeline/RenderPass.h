#pragma once

#include <span>
#include <vector>

#include "../devices/Device.h"
#include "../resource/ImageView.h"
#include "Framebuffer.h"
#include "SubpassDescription.h"

namespace vkcore {

class RenderPass {
 public:
  RenderPass(const Device& device, std::span<const VkAttachmentDescription> attachments,
             std::span<const SubpassDescription> subpasses,
             std::span<const VkSubpassDependency> dependencies = {});

  VkRenderPass handle() const { return renderPass_.get(); }
  const std::vector<VkAttachmentDescription>& attachments() const { return attachments_; }
  const std::vector<SubpassDescription>& subpasses() const { return subpasses_; }
  const std::vector<VkSubpassDependency>& dependencies() const { return dependencies_; }

  void Begin(VkCommandBuffer cmd, const Framebuffer& framebuffer, VkRect2D renderArea,
             const std::vector<VkClearValue>& clearValues,
             VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;

  void End(VkCommandBuffer cmd) const;

  Framebuffer MakeFramebuffer(const std::vector<const ImageView*>& attachments, uint32_t width,
                              uint32_t height, uint32_t layers = 1,
                              VkFramebufferCreateFlags flags = 0) const;

  Framebuffer MakeFramebuffer(const std::vector<const ImageView*>& attachments, VkExtent2D extent,
                              uint32_t layers = 1, VkFramebufferCreateFlags flags = 0) const;

  Framebuffer MakeFramebuffer(const ImageView* attachment, uint32_t width, uint32_t height,
                              uint32_t layers = 1, VkFramebufferCreateFlags flags = 0) const;

 private:
  const Device* device_;
  UniqueRenderPass renderPass_;
  std::vector<VkAttachmentDescription> attachments_;
  std::vector<SubpassDescription> subpasses_;
  std::vector<VkSubpassDependency> dependencies_;
};

}  // namespace vkcore