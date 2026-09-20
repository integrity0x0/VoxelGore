#include "ChunkRenderer.h"

#include <cassert>
#include <cstring>

#include "../../../core/PathPrefixes.h"
#include "glm/gtc/matrix_transform.hpp"

namespace gfx {

ChunkRenderer::ChunkRenderer(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                             const vkcore::DeviceQueue& graphicsQueue,
                             vkcore::MemoryAllocator& memoryAllocator, const vkcore::RenderPass& renderPass,
                             const GameDataBinding& gameDataBinding,
                             const ShadowContext* shadowCtxt,
                             const ShaderCompiler& shaderCompiler,
                             gm::ChunkManager& chunkManager,
                             const gm::Lighting& lighting,
                             const gm::BlockManager& blockManager, uint32_t framesCount)
    : device_(device),
      transferCtxt_(&transferCtxt),
      graphicsQueue_(&graphicsQueue),
      chunkManager_(&chunkManager),
      blockRenderData_(std::make_unique<BlockRenderData>(blockManager, device, transferCtxt,
                                                         memoryAllocator, framesCount)),
      meshBuilder_(device, lighting, blockManager, *blockRenderData_, framesCount),
      framesInFlightCount_(framesCount),
      shadowCtxt_(shadowCtxt) {

  if (shadowCtxt) {
    pipelines_ = std::make_unique<ChunkRenderPipelines>(
        device_, renderPass.handle(), gameDataBinding, &shadowCtxt->descriptorSetLayout(),
        blockRenderData_->descriptorSetLayout(), shaderCompiler);
    shadowPipelines_ = std::make_unique<ChunkShadowPipelines>(
        device_, shadowCtxt->pass(), gameDataBinding, blockRenderData_->descriptorSetLayout(),
        shaderCompiler);
  } else {
    pipelines_ = std::make_unique<ChunkRenderPipelines>(
        device_, renderPass.handle(), gameDataBinding, nullptr,
        blockRenderData_->descriptorSetLayout(), shaderCompiler);
  }
  
}

ChunkRenderer::~ChunkRenderer() = default;

void ChunkRenderer::BuildAll(VkCommandBuffer cmd, uint32_t currentFrameInFlight) {
  gm::DirtyChunkSet seedDirty;
  for (const auto& [chunkPos, chunkPtr] : chunkManager_->getChunks()) {
    seedDirty.insert(chunkPos);
  }

  meshBuilder_.BuildMeshes(cmd, currentFrameInFlight, seedDirty, chunkManager_->getChunks(),
                           meshes_);
}

void ChunkRenderer::UpdateDirty(VkCommandBuffer cmd, uint32_t currentFrameInFlight) {
  auto& dirty = chunkManager_->getDirtyChunks();
  if (dirty.empty()) return;

  for (const glm::ivec3& pos : dirty) {
    meshes_.solid.erase(pos);
    meshes_.cutout.erase(pos);
    meshes_.translucent.erase(pos);
  }

  graphicsQueue_->WaitIdle();
  meshBuilder_.BuildMeshes(cmd, currentFrameInFlight, dirty, chunkManager_->getChunks(), meshes_);
}

void ChunkRenderer::RenderShadow(VkCommandBuffer cmd, uint32_t currentFrame) {
  const vkcore::DescriptorSet& set = blockRenderData_->descriptorSet(currentFrame);
  set.Bind(cmd, pipelineLayout().handle(), kBlockRenderDataSetIndex);

  for (ShadowLayer layer : {ShadowLayer::Solid, ShadowLayer::Cutout}) {
    shadowPipelines_->Bind(cmd, layer);
    auto& bucket = (layer == ShadowLayer::Solid) ? meshes_.solid : meshes_.cutout;
    for (auto& [pos, mesh] : bucket) {
      if (chunkManager_->isDirty(pos)) continue;
      mesh.Bind(cmd);
      mesh.Draw(cmd);
    }
  }
}

void ChunkRenderer::Render(VkCommandBuffer cmd, float dt, uint32_t currentFrame,
                           const glm::vec3& cameraPos, RenderLayer renderLayer) {
  blockRenderData_->Update(dt, currentFrame);

  const vkcore::DescriptorSet& set = blockRenderData_->descriptorSet(currentFrame);
  set.Bind(cmd, pipelineLayout().handle(), kBlockRenderDataSetIndex);

  if (shadowCtxt_) {
    shadowCtxt_->descriptorSet().Bind(cmd, pipelineLayout().handle(), 2);
  }
  switch (renderLayer) {
    case RenderLayer::Solid: {
      pipelines_->Bind(cmd, RenderLayer::Solid);

      for (auto& [pos, mesh] : meshes_.solid) {
        if (chunkManager_->isDirty(pos)) {
          continue;
        }

        mesh.Bind(cmd);
        mesh.Draw(cmd);
      }

      break;
    }

    case RenderLayer::Cutout: {
      pipelines_->Bind(cmd, RenderLayer::Cutout);

      for (auto& [pos, mesh] : meshes_.cutout) {
        if (chunkManager_->isDirty(pos)) {
          continue;
        }

        mesh.Bind(cmd);
        mesh.Draw(cmd);
      }

      break;
    }

    case RenderLayer::Translucent: {
      struct Item {
        glm::ivec3 pos;
        TranslucentMesh* mesh;
        float dist2;
      };

      std::vector<Item> items;
      items.reserve(meshes_.translucent.size());

      for (auto& [pos, mesh] : meshes_.translucent) {
        if (chunkManager_->isDirty(pos)) {
          continue;
        }

        const glm::vec3 center = (glm::vec3(pos) + 0.5f) * static_cast<float>(gm::Chunk::kLength);

        items.push_back({
            pos,
            &mesh,
            glm::distance2(cameraPos, center),
        });
      }

      std::sort(items.begin(), items.end(),
                [](const Item& a, const Item& b) { return a.dist2 > b.dist2; });

      pipelines_->Bind(cmd, RenderLayer::Translucent);

      for (Item& item : items) {
        item.mesh->Sort(currentFrame, cameraPos);
        item.mesh->Draw(cmd, currentFrame);
      }

      break;
    }
  }
}
}  // namespace gfx