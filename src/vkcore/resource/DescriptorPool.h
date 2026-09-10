#pragma once

#include <memory>
#include <span>
#include <vector>

#include "../devices/Device.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"

namespace vkcore {

class DescriptorPool {
 public:
  DescriptorPool(const Device& device, std::span<const VkDescriptorPoolSize> poolSizes,
                 uint32_t maxSets);

  DescriptorSet Allocate(const DescriptorSetLayout& layout) const;

  std::vector<DescriptorSet> Allocate(const DescriptorSetLayout& layout, uint32_t count) const;

  std::vector<DescriptorSet> Allocate(std::span<const DescriptorSetLayout*> layouts) const;

 private:
  const Device* device;
  UniqueDescriptorPool descriptorPool;
};

}  // namespace vkcore