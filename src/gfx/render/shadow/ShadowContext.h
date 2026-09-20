#pragma once

#include "../../../vkcore/resource/DescriptorPool.h"
#include "../../../vkcore/resource/DescriptorSet.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "ShadowMap.h"
#include "ShadowPass.h"
#include "glm/glm.hpp"

namespace gfx {

class ShadowContext {
 public:
  ShadowContext(const vkcore::Device& device, vkcore::MemoryAllocator& allocator,
                VkExtent2D resolution = {4096, 4096});

  [[nodiscard]] const ShadowPass& pass() const { return pass_; }
  [[nodiscard]] const ShadowMap& map() const { return map_; }
  [[nodiscard]] const vkcore::Framebuffer& framebuffer() const { return map_.framebuffer(); }
  [[nodiscard]] const vkcore::SampledTexture& texture() const { return map_.texture(); }

  [[nodiscard]] const vkcore::DescriptorSetLayout& descriptorSetLayout() const {
    return descriptorSetLayout_;
  }
  [[nodiscard]] const vkcore::DescriptorSet& descriptorSet() const { return descriptorSet_; }

  void UpdateLightMatrix(const glm::vec3& lightDir, const glm::vec3& focusPoint);

  void Begin(VkCommandBuffer cmd) const {
    VkRect2D rect = {.extent = resolution_};
    pass_.Begin(cmd, framebuffer(), rect, std::vector<VkClearValue>{VkClearValue{1.0f}});
  }

  void End(VkCommandBuffer cmd) const {
    pass_.End(cmd);
  }

  [[nodiscard]] const glm::mat4& lightViewProj() const { return lightViewProj_; }
  [[nodiscard]] const glm::vec3& lightDir() const { return lightDir_; }

 private:
  [[nodiscard]] static vkcore::DescriptorSetLayout BuildDescriptorSetLayout(
      const vkcore::Device& device);
  [[nodiscard]] static vkcore::DescriptorPool BuildDescriptorPool(const vkcore::Device& device);

 private:
  static constexpr float kOrthoHalfExtent =
      64.0f;

  ShadowPass pass_;
  ShadowMap map_;
  vkcore::DescriptorSetLayout descriptorSetLayout_;
  vkcore::DescriptorPool descriptorPool_;
  vkcore::DescriptorSet descriptorSet_;

  glm::mat4 lightViewProj_ = glm::mat4(1.0f);
  glm::vec3 lightDir_ = glm::vec3(0.0f, -1.0f, 0.0f);

  VkExtent2D resolution_;
};

}  // namespace gfx