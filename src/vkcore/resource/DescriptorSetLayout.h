#pragma once

#include <span>

#include "../devices/Device.h"

namespace vkcore {
class DescriptorSetLayout {
 public:
  DescriptorSetLayout(const Device& device, std::span<const VkDescriptorSetLayoutBinding> bindings,
                      void* pNext = nullptr, VkDescriptorBindingFlags flags = 0)
      : device_(&device) {
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutCI.pBindings = bindings.data();
    descriptorSetLayoutCI.pNext = pNext;
    descriptorSetLayoutCI.flags = flags;

    VkDescriptorSetLayout descriptorSetLayoutRaw = VK_NULL_HANDLE;
    SystemError::check(
        device.dispatchTable().vkCreateDescriptorSetLayout(device.handle(), &descriptorSetLayoutCI,
                                                           nullptr, &descriptorSetLayoutRaw),
        "failed to create descriptor set layout");

    descriptorSetLayout_ = UniqueDescriptorSetLayout(
        descriptorSetLayoutRaw,
        {device.handle(), device.dispatchTable().vkDestroyDescriptorSetLayout});
  }

  VkDescriptorSetLayout handle() const { return descriptorSetLayout_.get(); }

 private:
  const Device* device_;
  UniqueDescriptorSetLayout descriptorSetLayout_;
};
}  // namespace vkcore
