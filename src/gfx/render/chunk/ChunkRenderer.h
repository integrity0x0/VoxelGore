#pragma once

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../../../game/voxel/BlockManager.h"
#include "../../../game/voxel/ChunkManager.h"
#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/pipeline/RenderPass.h"
#include "../../../vkcore/resource/Buffer.h"
#include "../../../vkcore/resource/DescriptorPool.h"
#include "../../../vkcore/resource/SampledTexture.h"
#include "../../../vkcore/resource/Sampler.h"
#include "../../render/block/BlockRenderData.h"
#include "../shadow/ShadowContext.h"
#include "ChunkShadowPipelines.h"
#include "../GameDataBinding.h"
#include "ChunkMeshBuilder.h"
#include "ChunkMeshes.h"
#include "ChunkRenderPipelines.h"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace gfx {

class ChunkRenderer {
 public:
  ChunkRenderer(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                const vkcore::DeviceQueue& graphicsQueue, vkcore::MemoryAllocator& memoryAllocator,
                const vkcore::RenderPass& renderPass, const GameDataBinding& gameDataBinding, const ShadowContext* shadowCtxt,
                const ShaderCompiler& shaderCompiler,
                gm::ChunkManager& chunkManager, const gm::Lighting& lighting, 
                const gm::BlockManager& blockManager,
                uint32_t framesCount);

  ~ChunkRenderer();

  void BuildAll(VkCommandBuffer cmd, uint32_t currentFrame);
  void UpdateDirty(VkCommandBuffer cmd, uint32_t currentFrame);

  void Render(VkCommandBuffer cmd, float dt, uint32_t currentFrame, const glm::vec3& cameraPos,
              RenderLayer renderLayer);
  void RenderShadow(VkCommandBuffer cmd, uint32_t currentFrame);
  Atlas& getAtlas() { return blockRenderData_->atlas(); }

  BlockRenderData& blockRenderData() { return *blockRenderData_; }

  const vkcore::PipelineLayout& pipelineLayout() { return pipelines_->pipelineLayout(); }

 private:
  uint32_t kBlockRenderDataSetIndex = 1u;
  const vkcore::Device& device_;
  vkcore::TransferContext* transferCtxt_;
  const vkcore::DeviceQueue* graphicsQueue_;
  const ShadowContext* shadowCtxt_;
  std::unique_ptr<BlockRenderData> blockRenderData_;

  std::unique_ptr<ChunkRenderPipelines> pipelines_;
  std::unique_ptr<ChunkShadowPipelines> shadowPipelines_;
  gm::ChunkManager* chunkManager_;
  ChunkMeshBuilder meshBuilder_;
  ChunkMeshes meshes_;

  uint32_t framesInFlightCount_ = 0;
};

}  // namespace gfx