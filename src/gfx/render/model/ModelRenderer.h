#pragma once

#include <glm/mat4x3.hpp>
#include <glm/vec4.hpp>
#include <vector>

#include "../shadow/ShadowContext.h"
#include "../../common/mesh/ModelCache.h"
#include "ModelInstanceData.h"
#include "ModelPipeline.h"
#include "ModelShadowPipeline.h"

namespace gfx {

class ModelRenderer {
 public:
  ModelRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                const vkcore::RenderPass& renderPass, const GameDataBinding& gameDataBinding,
                const vkcore::DescriptorSetLayout& materialSetLayout,
                const ShadowContext* shadowCtxt, const ShaderCompiler& shaderCompiler,
                uint32_t framesCount);

  void Submit(const Model& model, const glm::mat4x3& transform,
              const glm::vec4& color = glm::vec4(1.0f));

  void UploadInstances(uint32_t frameIndex);

  void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame);
  void RenderShadow(VkCommandBuffer commandBuffer, uint32_t currentFrame);

 private:
  struct FrameData {
    vkcore::BufferSlice instanceBuffer;
    ModelInstanceData* mapped = nullptr;
  };

  struct ModelGroup {
    const Model* model;
    std::vector<ModelInstanceData> instances;
  };

  void DrawGroups(VkCommandBuffer cmd, uint32_t frameIndex, const vkcore::PipelineLayout& layout);
 private:
  static constexpr uint32_t kMaxInstances = 2048;
  static constexpr uint32_t kInvalidGroup = std::numeric_limits<uint32_t>::max();

  const vkcore::Device* device_;
  vkcore::BufferAllocator* bufferAllocator_;
  ModelPipeline pipeline_;
  std::optional<ModelShadowPipeline> shadowPipeline_;
  std::vector<FrameData> frames_;

  std::vector<uint32_t> sparseIndices_;
  std::vector<ModelGroup> groups_;

  uint32_t instanceCount_ = 0;
};

}  // namespace gfx