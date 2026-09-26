#include "GameDataBinding.h"

#include <array>

namespace gfx {

vkcore::DescriptorSetLayout GameDataBinding::BuildDescriptorSetLayout(
    const vkcore::Device& device) {
  VkDescriptorSetLayoutBinding binding = {};
  binding.binding = 0;
  binding.descriptorCount = 1u;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  return vkcore::DescriptorSetLayout(device, std::to_array({binding}));
}

vkcore::DescriptorPool GameDataBinding::BuildDescriptorPool(
    const vkcore::Device& device, uint32_t framesCount) {
  return vkcore::DescriptorPool(
      device, std::to_array({VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, framesCount}}),
      framesCount);
}

vkcore::PipelineLayout GameDataBinding::BuildPipelineLayout(const vkcore::Device& device) {
  return vkcore::PipelineLayout(device, std::to_array({&descriptorSetLayout_}));
}

GameDataBinding::GameDataBinding(const vkcore::Device& device,
                                 vkcore::BufferAllocator& bufferAllocator, uint32_t framesCount)
    : device_(&device),
      descriptorSetLayout_(BuildDescriptorSetLayout(device)),
      descriptorPool_(BuildDescriptorPool(device, framesCount)),
      pipelineLayout_(BuildPipelineLayout(device)){
  frames_.reserve(framesCount);

  std::vector<VkWriteDescriptorSet> writes;
  writes.reserve(framesCount);

  std::vector<VkDescriptorBufferInfo> bufferInfos;
  bufferInfos.reserve(framesCount);

  VkDeviceSize alignment =
      device.getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;

  for (size_t i = 0; i < static_cast<size_t>(framesCount); ++i) {
    vkcore::BufferSlice buffer = bufferAllocator.Allocate(sizeof(UniformGameData), alignment,
                                                          kBuffersUsage, kMemoryProperties);

    UniformGameData* mapped = reinterpret_cast<UniformGameData*>(buffer.map());

    auto descriptorSet = descriptorPool_.Allocate(descriptorSetLayout_);

    
    VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
    bufferInfo.buffer = buffer.handle();
    bufferInfo.offset = buffer.offset();
    bufferInfo.range = sizeof(UniformGameData);

    frames_.emplace_back(mapped, std::move(buffer), std::move(descriptorSet));

    VkWriteDescriptorSet& write = writes.emplace_back();
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = frames_.back().descriptorSet.handle();
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = &bufferInfo;
  }

  device.dispatchTable().vkUpdateDescriptorSets(
      device.handle(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
}  // namespace gfx