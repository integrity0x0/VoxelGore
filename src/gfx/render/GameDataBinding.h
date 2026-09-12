#pragma once

#include <vector>

#include "../../vkcore/resource/BufferAllocator.h"
#include "../../vkcore/resource/DescriptorPool.h"
#include "../../vkcore/resource/DescriptorSetLayout.h"
#include "../../vkcore/pipeline/PipelineLayout.h"
#include "UniformGameData.h"

namespace gfx {
class GameDataBinding {
 public:
  GameDataBinding(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                  uint32_t framesCount);

  void Update(uint32_t frameIndex, const UniformGameData& data) {
    *frames_[frameIndex].mapped = data;
  }

  void Bind(VkCommandBuffer cmd, uint32_t frameIndex) const {
    VkDescriptorSet set = frames_[frameIndex].descriptorSet.handle();
    device_->dispatchTable().vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_.handle(),
                                                     kFirstSetIndex, 1u, &set, 0, nullptr);
  }

  [[nodiscard]] const vkcore::DescriptorSetLayout& descriptorSetLayout() const {
    return descriptorSetLayout_;
  }
 private:
  [[nodiscard]] vkcore::DescriptorSetLayout BuildDescriptorSetLayout(const vkcore::Device& device);
  [[nodiscard]] vkcore::PipelineLayout BuildPipelineLayout(const vkcore::Device& device);
  [[nodiscard]] vkcore::DescriptorPool BuildDescriptorPool(const vkcore::Device& device,
                                                           uint32_t framesCount);
 private:
  static constexpr uint32_t kFirstSetIndex = 0;
  static constexpr VkBufferUsageFlags kBuffersUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  static constexpr VkMemoryPropertyFlags kMemoryProperties =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  const vkcore::Device* device_;
  vkcore::DescriptorSetLayout descriptorSetLayout_;
  vkcore::PipelineLayout pipelineLayout_;
  vkcore::DescriptorPool descriptorPool_;

  struct FrameData {
    UniformGameData* mapped;
    vkcore::BufferSlice buffer;
    vkcore::DescriptorSet descriptorSet;

    FrameData(UniformGameData* mapped, vkcore::BufferSlice&& buffer,
              vkcore::DescriptorSet&& descriptorSet)
        : mapped(mapped), buffer(std::move(buffer)), descriptorSet(std::move(descriptorSet)) {}
  };

  std::vector<FrameData> frames_;
};

}  // namespace gfx