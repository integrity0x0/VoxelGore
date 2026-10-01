#include "DescriptorPool.h"

#include <cassert>

#include "../common/SystemError.h"

namespace vkcore {

DescriptorPool::DescriptorPool(const Device& device,
                               std::span<const VkDescriptorPoolSize> poolSizes, uint32_t maxSets)
    : device(&device) {
  VkDescriptorPoolCreateInfo descriptorPoolCI = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolCI.pPoolSizes = poolSizes.data();
  descriptorPoolCI.maxSets = maxSets;
  descriptorPoolCI.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  VkDescriptorPool descriptorPoolRaw = VK_NULL_HANDLE;
  SystemError::Check(device.GetDispatchTable().vkCreateDescriptorPool(
                         device.GetHandle(), &descriptorPoolCI, nullptr, &descriptorPoolRaw),
                     "failed to create descriptor pool");

  descriptorPool = UniqueDescriptorPool(
      descriptorPoolRaw, {device.GetHandle(), device.GetDispatchTable().vkDestroyDescriptorPool});
}

DescriptorSet DescriptorPool::Allocate(const DescriptorSetLayout& layout) const {
  VkDescriptorSetLayout layouts[] = {layout.GetHandle()};
  VkDescriptorSetAllocateInfo descriptorSetAI = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  descriptorSetAI.descriptorPool = descriptorPool.get();
  descriptorSetAI.descriptorSetCount = 1;
  descriptorSetAI.pSetLayouts = layouts;
  VkDescriptorSet descriptorSetRaw = VK_NULL_HANDLE;
  SystemError::Check(device->GetDispatchTable().vkAllocateDescriptorSets(
                         device->GetHandle(), &descriptorSetAI, &descriptorSetRaw),
                     "Failed to allocate descriptor set");

  return DescriptorSet(
      *device, std::move(UniqueDescriptorSet(descriptorSetRaw,
                                             {device->GetHandle(), descriptorPool.get(),
                                              device->GetDispatchTable().vkFreeDescriptorSets})));
}

std::vector<DescriptorSet> DescriptorPool::Allocate(const DescriptorSetLayout& layout,
                                                    uint32_t count) const {
  std::vector<VkDescriptorSetLayout> rawLayouts(count, layout.GetHandle());

  VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocInfo.descriptorPool = descriptorPool.get();
  allocInfo.descriptorSetCount = static_cast<uint32_t>(rawLayouts.size());
  allocInfo.pSetLayouts = rawLayouts.data();

  std::vector<VkDescriptorSet> rawSets(allocInfo.descriptorSetCount);
  SystemError::Check(device->GetDispatchTable().vkAllocateDescriptorSets(device->GetHandle(), &allocInfo,
                                                                      rawSets.data()),
                     "Failed to allocate descriptor sets");

  std::vector<DescriptorSet> result;
  result.reserve(rawSets.size());

  DescriptorSetDeleter deleter{device->GetHandle(), descriptorPool.get(),
                               device->GetDispatchTable().vkFreeDescriptorSets};

  for (auto rawSet : rawSets) {
    UniqueDescriptorSet unique(rawSet, deleter);
    result.push_back(DescriptorSet(*device, std::move(unique)));
  }

  return result;
}

std::vector<DescriptorSet> DescriptorPool::Allocate(
    std::span<const DescriptorSetLayout*> layouts) const {
  std::vector<VkDescriptorSetLayout> rawLayouts;
  rawLayouts.reserve(layouts.size());
  for (const auto& layout : layouts) {
    assert(layout);
    rawLayouts.push_back(layout->GetHandle());
  }

  VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocInfo.descriptorPool = descriptorPool.get();
  allocInfo.descriptorSetCount = static_cast<uint32_t>(rawLayouts.size());
  allocInfo.pSetLayouts = rawLayouts.data();

  std::vector<VkDescriptorSet> rawSets(allocInfo.descriptorSetCount);
  SystemError::Check(device->GetDispatchTable().vkAllocateDescriptorSets(device->GetHandle(), &allocInfo,
                                                                      rawSets.data()),
                     "Failed to allocate descriptor sets");

  std::vector<DescriptorSet> result;
  result.reserve(rawSets.size());

  DescriptorSetDeleter deleter{device->GetHandle(), descriptorPool.get(),
                               device->GetDispatchTable().vkFreeDescriptorSets};

  for (auto rawSet : rawSets) {
    UniqueDescriptorSet unique(rawSet, deleter);
    result.push_back(DescriptorSet(*device, std::move(unique)));
  }

  return result;
}

}  // namespace vkcore