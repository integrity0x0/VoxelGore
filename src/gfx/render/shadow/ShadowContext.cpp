#include "ShadowContext.h"

#include "glm/gtc/matrix_transform.hpp"

namespace gfx {

ShadowContext::ShadowContext(const vkcore::Device& device, vkcore::MemoryAllocator& allocator,
                             VkExtent2D resolution)
    : pass_(device),
      map_(device, allocator, pass_, resolution),
      descriptorSetLayout_(BuildDescriptorSetLayout(device)),
      descriptorPool_(BuildDescriptorPool(device)),
      descriptorSet_(descriptorPool_.Allocate(descriptorSetLayout_)),
      resolution_(resolution) {
  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  imageInfo.imageView = map_.texture().imageView().handle();
  imageInfo.sampler = map_.texture().sampler().handle();

  VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write.dstSet = descriptorSet_.handle();
  write.dstBinding = 0;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.pImageInfo = &imageInfo;

  device.dispatchTable().vkUpdateDescriptorSets(device.handle(), 1u, &write, 0, nullptr);
}

vkcore::DescriptorSetLayout ShadowContext::BuildDescriptorSetLayout(const vkcore::Device& device) {
  VkDescriptorSetLayoutBinding shadowMapBinding = {};
  shadowMapBinding.binding = 0;
  shadowMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  shadowMapBinding.descriptorCount = 1;
  shadowMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  return vkcore::DescriptorSetLayout(device,
                                     std::vector<VkDescriptorSetLayoutBinding>{shadowMapBinding});
}

vkcore::DescriptorPool ShadowContext::BuildDescriptorPool(const vkcore::Device& device) {
  std::vector<VkDescriptorPoolSize> poolSizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
  return vkcore::DescriptorPool(device, poolSizes, 1u);
}

void ShadowContext::UpdateLightMatrix(const glm::vec3& lightDir, const glm::vec3& focusPoint) {
  lightDir_ = glm::normalize(lightDir);

  const glm::vec3 eye = focusPoint - lightDir_ * kOrthoHalfExtent;
  const glm::vec3 up = std::abs(lightDir_.y) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

  glm::mat4 view = glm::lookAt(eye, focusPoint, up);
  glm::mat4 proj = glm::ortho(-kOrthoHalfExtent, kOrthoHalfExtent, -kOrthoHalfExtent,
                              kOrthoHalfExtent, -kOrthoHalfExtent, kOrthoHalfExtent * 2.0f);
  proj[1][1] *= -1; 

  lightViewProj_ = proj * view;
}

}  // namespace gfx