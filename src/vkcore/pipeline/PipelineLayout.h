#pragma once

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include "../devices/Device.h"
#include "../resource/DescriptorSetLayout.h"

namespace vkcore {

class PipelineLayout {
 public:
  PipelineLayout(const Device& device,
                 std::span<const DescriptorSetLayout* const> descriptorSetLayouts = {},
                 std::span<const VkPushConstantRange> pcRanges = {}, void* pNext = nullptr,
                 VkPipelineLayoutCreateFlags flags = 0u);

  VkPipelineLayout handle() const { return pipelineLayout.get(); }

  void PushConstants(VkCommandBuffer commandBuffer, VkShaderStageFlags stageFlags, uint32_t offset,
                     std::span<const std::byte> data) const;

  template <typename T>
  void PushConstants(VkCommandBuffer commandBuffer, VkShaderStageFlags stageFlags, uint32_t offset,
                     const T& data) const {
    PushConstants(commandBuffer, stageFlags, offset,
                  std::span<const std::byte>(reinterpret_cast<const std::byte*>(&data), sizeof(T)));
  }

  template <typename T>
  void PushConstants(VkCommandBuffer commandBuffer, VkShaderStageFlags stageFlags,
                     const T& data) const {
    PushConstants(commandBuffer, stageFlags, 0u,
                  std::span<const std::byte>(reinterpret_cast<const std::byte*>(&data), sizeof(T)));
  }

 private:
  const Device* device;
  UniquePipelineLayout pipelineLayout;
};

}  // namespace vkcore