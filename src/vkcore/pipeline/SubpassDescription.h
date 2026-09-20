#pragma once
#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

namespace vkcore {

struct SubpassDescription {
  VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  std::vector<VkAttachmentReference> colorRefs;
  std::vector<VkAttachmentReference> inputRefs;
  std::vector<VkAttachmentReference> resolveRefs;
  std::optional<VkAttachmentReference> depthStencilRef;
  std::vector<uint32_t> preserveIndices;

  [[nodiscard]] VkSubpassDescription Build() const {
    VkSubpassDescription desc = {};
    desc.pipelineBindPoint = bindPoint;
    desc.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
    desc.pColorAttachments = colorRefs.empty() ? nullptr : colorRefs.data();
    desc.inputAttachmentCount = static_cast<uint32_t>(inputRefs.size());
    desc.pInputAttachments = inputRefs.empty() ? nullptr : inputRefs.data();
    desc.pResolveAttachments = resolveRefs.empty() ? nullptr : resolveRefs.data();
    desc.pDepthStencilAttachment = depthStencilRef.has_value() ? &depthStencilRef.value() : nullptr;
    desc.preserveAttachmentCount = static_cast<uint32_t>(preserveIndices.size());
    desc.pPreserveAttachments = preserveIndices.empty() ? nullptr : preserveIndices.data();
    return desc;
  }
};

}  // namespace vkcore
