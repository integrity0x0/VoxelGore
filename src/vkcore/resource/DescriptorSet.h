#pragma once
#include "../devices/Device.h"

namespace vkcore {
class DescriptorPool;

class DescriptorSet {
 public:
  VkDescriptorSet handle() const { return descriptorSet_.get(); }

  void Bind(VkCommandBuffer cmd, VkPipelineLayout layout,
            VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) {
    VkDescriptorSet descriptorSets[] = {descriptorSet_.get()};

    device_->dispatchTable().vkCmdBindDescriptorSets(cmd, bindPoint, layout, 0, 1, descriptorSets,
                                                     0, nullptr);
  }

 private:
  friend class DescriptorPool;

  DescriptorSet(const Device& device, UniqueDescriptorSet&& descriptorSet)
      : device_(&device), descriptorSet_(std::move(descriptorSet)) {}
  const Device* device_;
  UniqueDescriptorSet descriptorSet_;
};
}  // namespace vkcore
