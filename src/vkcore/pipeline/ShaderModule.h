#pragma once

#include <span>

#include "../devices/Device.h"

namespace vkcore {
class ShaderModule {
 public:
  ShaderModule(const Device& device, std::span<const uint32_t> data,
               VkShaderModuleCreateFlags flags = 0, void* pNext = nullptr);

  [[nodiscard]] VkShaderModule handle() const { return shaderModule_.get(); }
 private:
  UniqueShaderModule shaderModule_;
};
}  // namespace vkcore
