#include "PipelineLayout.h"

#include "../common/SystemError.h"

namespace vkcore {

PipelineLayout::PipelineLayout(const Device& device,
                               std::span<const DescriptorSetLayout* const> descriptorSetLayouts,
                               std::span<const VkPushConstantRange> pcRanges, void* pNext,
                               VkPipelineLayoutCreateFlags flags)
    : device(&device) {
  VkPipelineLayoutCreateInfo pipelineLayoutCI = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  pipelineLayoutCI.pNext = pNext;
  pipelineLayoutCI.flags = flags;

  std::vector<VkDescriptorSetLayout> descriptorSetLayoutHandles;
  descriptorSetLayoutHandles.reserve(descriptorSetLayouts.size());
  for (const auto* layout : descriptorSetLayouts) {
    descriptorSetLayoutHandles.push_back(layout->handle());
  }

  pipelineLayoutCI.setLayoutCount = static_cast<uint32_t>(descriptorSetLayoutHandles.size());
  pipelineLayoutCI.pSetLayouts = descriptorSetLayoutHandles.data();
  pipelineLayoutCI.pushConstantRangeCount = static_cast<uint32_t>(pcRanges.size());
  pipelineLayoutCI.pPushConstantRanges = pcRanges.data();

  VkPipelineLayout pipelineLayoutRaw = VK_NULL_HANDLE;
  SystemError::check(device.dispatchTable().vkCreatePipelineLayout(
                         device.handle(), &pipelineLayoutCI, nullptr, &pipelineLayoutRaw),
                     "failed to create pipeline layout");
  pipelineLayout = UniquePipelineLayout(
      pipelineLayoutRaw, {device.handle(), device.dispatchTable().vkDestroyPipelineLayout});
}

void PipelineLayout::PushConstants(VkCommandBuffer commandBuffer, VkShaderStageFlags stageFlags,
                                   uint32_t offset, std::span<const std::byte> data) const {
  device->dispatchTable().vkCmdPushConstants(commandBuffer, pipelineLayout.get(), stageFlags,
                                             offset, static_cast<uint32_t>(data.size()),
                                             data.data());
}

}  // namespace vkcore