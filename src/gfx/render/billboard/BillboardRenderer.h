#pragma once

#include <memory>
#include <vector>

#include "../../../vkcore/pipeline/RenderPass.h"
#include "../GameDataBinding.h"
#include "BillboardRenderBucket.h"

namespace gfx {

class BillboardRenderer {
 public:
  BillboardRenderer(const vkcore::Device& device, const vkcore::RenderPass& renderPass,
                    const GameDataBinding& gameDataBinding, const ShaderCompiler& shaderCompiler);

  [[nodiscard]] BillboardRenderBucket& CreateBucket(vkcore::BufferAllocator& bufferAllocator, 
                                                    const Atlas& atlas, uint32_t framesCount);

  void Render(VkCommandBuffer cmd, const glm::vec3& cameraPos, RenderLayer renderLayer,
              uint32_t currentFrame);

 private:
  const vkcore::Device* device_;
  BillboardPipelines pipelines_;
  std::vector<std::unique_ptr<BillboardRenderBucket>> buckets_;
};
}  // namespace gfx
