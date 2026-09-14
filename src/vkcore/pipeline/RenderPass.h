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

  [[nodiscard]] VkRenderPass handle() const { return renderPass_.get(); }
  [[nodiscard]] const std::vector<VkAttachmentDescription>& attachments() const {
    return attachments_;
  }
  [[nodiscard]] const std::vector<SubpassDescription>& subpasses() const { return subpasses_; }
  [[nodiscard]] const std::vector<VkSubpassDependency>& dependencies() const {
    return dependencies_;
  }

  void Begin(VkCommandBuffer cmd, const Framebuffer& framebuffer, VkRect2D renderArea,
             const std::vector<VkClearValue>& clearValues,
             VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;

  void End(VkCommandBuffer cmd) const;

  [[nodiscard]] Framebuffer MakeFramebuffer(std::span<const ImageView* const> attachments,
                              VkExtent2D extent, uint32_t layers = 1,
                              VkFramebufferCreateFlags flags = 0) const;

  [[nodiscard]] Framebuffer MakeFramebuffer(const ImageView* attachment, VkExtent2D extent,
                                           uint32_t layers = 1, 
                                           VkFramebufferCreateFlags flags = 0) const;
 private:
  const Device* device_;
  UniqueRenderPass renderPass_;
  std::vector<VkAttachmentDescription> attachments_;
  std::vector<SubpassDescription> subpasses_;
  std::vector<VkSubpassDependency> dependencies_;
};

}  // namespace vkcore