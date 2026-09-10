#pragma once

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../../../game/voxel/BlockManager.h"
#include "../../../game/voxel/ChunkManager.h"
#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/resource/Buffer.h"
#include "../../../vkcore/resource/DescriptorPool.h"
#include "../../../vkcore/resource/SampledTexture.h"
#include "../../../vkcore/resource/Sampler.h"
#include "../../block/RenderData.h"
#include "../GameDataBinding.h"
#include "ChunkMeshBuilder.h"
#include "ChunkMeshes.h"
#include "ChunkRenderPipelines.h"
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"

namespace gfx {

class ChunkRenderer {
 public:
  ChunkRenderer(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                const vkcore::DeviceQueue& graphicsQueue, vkcore::MemoryAllocator& memoryAllocator,
                VkRenderPass renderPass, const GameDataBinding& gameDataBinding,
                gm::ChunkManager& chunkManager, const gm::BlockManager& blockManager,
                uint32_t framesInFlightCount);

  ~ChunkRenderer();

  void buildAll(VkCommandBuffer cmd, uint32_t currentFrame);
  void updateDirty(VkCommandBuffer cmd, uint32_t currentFrame);

  void Render(VkCommandBuffer cmd, float dt, uint32_t currentFrame, const glm::vec3& cameraPos,
              RenderLayer renderLayer);

  Atlas& getAtlas() { return blockRenderData_->atlas(); }

  block::RenderData& blockRenderData() { return *blockRenderData_; }

  const vkcore::PipelineLayout& pipelineLayout() { return pipelines_->pipelineLayout(); }

 private:
  uint32_t kDescriptorSetIndex = 1u;
  const vkcore::Device& device_;
  vkcore::TransferContext* transferCtxt_;
  const vkcore::DeviceQueue* graphicsQueue_;
  const gm::BlockManager* blockManager_;

  std::unique_ptr<block::RenderData> blockRenderData_;

  std::unique_ptr<vkcore::DescriptorSetLayout> descriptorSetLayout_;
  std::unique_ptr<ChunkRenderPipelines> pipelines_;

  std::unique_ptr<vkcore::DescriptorPool> descriptorPool_;
  std::vector<std::unique_ptr<vkcore::DescriptorSet>> descriptorSets_;

  gm::ChunkManager* chunkManager_;
  ChunkMeshBuilder meshBuilder_;
  ChunkMeshes meshes_;

  uint32_t framesInFlightCount_ = 0;
};

}  // namespace gfx