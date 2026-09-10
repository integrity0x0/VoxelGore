#pragma once

#include <glm/mat4x3.hpp>
#include <glm/vec4.hpp>
#include <unordered_map>
#include <vector>

#include "../mesh/ModelCache.h"
#include "ModelInstanceData.h"
#include "ModelPipeline.h"

namespace gfx {

class ModelRenderer {
 public:
  ModelRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                const ModelPipeline& pipeline, uint32_t framesCount);

  void BeginFrame(uint32_t currentFrame);

  void Submit(const Model& model, const glm::mat4x3& transform,
              const glm::vec4& color = glm::vec4(1.0f));

  void Render(VkCommandBuffer commandBuffer);

 private:
  struct FrameData {
    vkcore::BufferSlice instanceBuffer;
    ModelInstanceData* mapped = nullptr;
  };

 private:
  static constexpr uint32_t kMaxInstances = 2048;

  const vkcore::Device* device_;
  vkcore::BufferAllocator* bufferAllocator_;
  const ModelPipeline* pipeline_;

  std::vector<FrameData> frames_;

  std::unordered_map<const Model*, std::vector<ModelInstanceData>> items_;

  uint32_t currentFrame_ = 0;
  uint32_t instanceCount_ = 0;
};

}  // namespace gfx