#pragma once

#include "SkyboxPipeline.h"
#include "../../texture/Skybox.h"
#include "../../mesh/Mesh.h"

namespace gfx {
class SkyboxRenderer {
 public:
  SkyboxRenderer(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                 const GameDataBinding& gameDataBinding);

  void Draw(VkCommandBuffer cmd, const Skybox& skybox);
  [[nodiscard]] const vkcore::DescriptorPool& descriptorPool() const { return descriptorPool_; }
 private:
  SkyboxPipeline pipeline_;
  vkcore::DescriptorPool descriptorPool_;
  Mesh mesh_;
};
}  // namespace gfx