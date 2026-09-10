#include "RenderPass.h"

namespace vkcore {

RenderPass::RenderPass(const Device& device, std::span<const VkAttachmentDescription> attachments,
                       std::span<const SubpassDescription> subpasses,
                       std::span<const VkSubpassDependency> dependencies)
    : device_(&device),
      attachments_(attachments.begin(), attachments.end()),
      subpasses_(subpasses.begin(), subpasses.end()),
      dependencies_(dependencies.begin(), dependencies.end()) {
  std::vector<VkSubpassDescription> builtSubpasses;
  builtSubpasses.reserve(subpasses_.size());
  for (const auto& sp : subpasses_) {
    builtSubpasses.push_back(sp.build());
  }

  VkRenderPassCreateInfo renderPassCI = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  renderPassCI.attachmentCount = static_cast<uint32_t>(attachments_.size());
  renderPassCI.pAttachments = attachments_.data();
  renderPassCI.subpassCount = static_cast<uint32_t>(builtSubpasses.size());
  renderPassCI.pSubpasses = builtSubpasses.data();
  renderPassCI.dependencyCount = static_cast<uint32_t>(dependencies_.size());
  renderPassCI.pDependencies = dependencies_.data();

  VkRenderPass raw = VK_NULL_HANDLE;
  VkResult result =
      device.dispatchTable().vkCreateRenderPass(device.handle(), &renderPassCI, nullptr, &raw);
  SystemError::check(result, "failed to create render pass");

  RenderPassDeleter deleter{device.handle(), device.dispatchTable().vkDestroyRenderPass};
  renderPass_ = UniqueRenderPass(raw, deleter);
}

void RenderPass::Begin(VkCommandBuffer cmd, const Framebuffer& framebuffer, VkRect2D renderArea,
                       const std::vector<VkClearValue>& clearValues,
                       VkSubpassContents contents) const {
  VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  beginInfo.renderPass = renderPass_.get();
  beginInfo.framebuffer = framebuffer.handle();
  beginInfo.renderArea = renderArea;
  beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  beginInfo.pClearValues = clearValues.empty() ? nullptr : clearValues.data();

  device_->dispatchTable().vkCmdBeginRenderPass(cmd, &beginInfo, contents);
}

void RenderPass::End(VkCommandBuffer cmd) const {
  device_->dispatchTable().vkCmdEndRenderPass(cmd);
}

Framebuffer RenderPass::MakeFramebuffer(const std::vector<const ImageView*>& attachments,
                                        uint32_t width, uint32_t height, uint32_t layers,
                                        VkFramebufferCreateFlags flags) const {
  return Framebuffer(*device_, attachments, renderPass_.get(), width, height, layers, flags);
}

Framebuffer RenderPass::MakeFramebuffer(const std::vector<const ImageView*>& attachments,
                                        VkExtent2D extent, uint32_t layers,
                                        VkFramebufferCreateFlags flags) const {
  return MakeFramebuffer(attachments, extent.width, extent.height, layers, flags);
}

Framebuffer RenderPass::MakeFramebuffer(const ImageView* attachment, uint32_t width,
                                        uint32_t height, uint32_t layers,
                                        VkFramebufferCreateFlags flags) const {
  std::vector<const ImageView*> attachments = {attachment};
  return MakeFramebuffer(attachments, width, height, layers, flags);
}

}  // namespace vkcore