#include "ShaderModule.h"

namespace vkcore {
ShaderModule::ShaderModule(const Device& device, std::span<const uint32_t> data,
                           VkShaderModuleCreateFlags flags, void* pNext) {
  VkShaderModuleCreateInfo moduleCI = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  moduleCI.codeSize = data.size() * sizeof(uint32_t);
  moduleCI.pCode = data.data();
  moduleCI.flags = flags;
  moduleCI.pNext = pNext;

  VkShaderModule shaderModule;
  SystemError::Check(device.dispatchTable().vkCreateShaderModule(device.handle(), &moduleCI,
                                                                 nullptr, &shaderModule),
                     "failed to create shader module");

  shaderModule_ = UniqueShaderModule(
      shaderModule, {device.handle(), device.dispatchTable().vkDestroyShaderModule});
}
}  // namespace vkcore