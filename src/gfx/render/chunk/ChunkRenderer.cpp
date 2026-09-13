#include "ChunkRenderer.h"

#include <cassert>
#include <cstring>

#include "../../../core/PathPrefixes.h"
#include "glm/gtc/matrix_transform.hpp"

namespace gfx {

ChunkRenderer::ChunkRenderer(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                             const vkcore::DeviceQueue& graphicsQueue,
                             vkcore::MemoryAllocator& memoryAllocator, VkRenderPass renderPass,
                             const GameDataBinding& gameDataBinding, gm::ChunkManager& chunkManager,
                             const gm::lighting::Lighting& lighting,
                             const gm::BlockManager& blockManager, uint32_t framesInFlightCount)
    : device_(device),
      transferCtxt_(&transferCtxt),
      graphicsQueue_(&graphicsQueue),
      blockManager_(&blockManager),
      chunkManager_(&chunkManager),
      blockRenderData_(std::make_unique<block::RenderData>(blockManager, device, transferCtxt,
                                                           memoryAllocator, framesInFlightCount)),
      meshBuilder_(device, lighting, blockManager, *blockRenderData_, framesInFlightCount),
      framesInFlightCount_(framesInFlightCount) {
  // ---------------------------------------------------------------------
  //   0 - UniformGameData (UBO)
  //   2 - BlockUvBuffer   (storage buffer, per-frame)
  //   3 - Atlas texture   (combined image sampler)
  // ---------------------------------------------------------------------

  VkDescriptorSetLayoutBinding uvBufferBinding = {};
  uvBufferBinding.binding = 0;
  uvBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uvBufferBinding.descriptorCount = 1;
  uvBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding textureBinding{};
  textureBinding.binding = 1;
  textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  textureBinding.descriptorCount = 1;
  textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  descriptorSetLayout_ = std::make_unique<vkcore::DescriptorSetLayout>(
      device_, std::vector<VkDescriptorSetLayoutBinding>{uvBufferBinding, textureBinding});

  pipelines_ = std::make_unique<ChunkRenderPipelines>(device_, renderPass, gameDataBinding,
                                                      *descriptorSetLayout_);

  // ---------------------------------------------------------------------
  // Descriptor Pool + Sets
  // ---------------------------------------------------------------------
  std::vector<VkDescriptorPoolSize> poolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, framesInFlightCount_},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, framesInFlightCount_}};

  descriptorPool_ =
      std::make_unique<vkcore::DescriptorPool>(device_, poolSizes, framesInFlightCount_);

  descriptorSets_.reserve(framesInFlightCount_);
  for (uint32_t i = 0; i < framesInFlightCount_; ++i) {
    descriptorSets_.push_back(
        std::make_unique<vkcore::DescriptorSet>(descriptorPool_->Allocate(*descriptorSetLayout_)));
  }

  // ---------------------------------------------------------------------
  // Uniform buffer (UniformGameData) — один буфер, динамические оффсеты
  // ---------------------------------------------------------------------
  VkPhysicalDeviceProperties deviceProps = device.getPhysicalDevice().getProperties();
  VkDeviceSize minAlign = deviceProps.limits.minUniformBufferOffsetAlignment;

  const std::vector<gfx::block::UvBuffer>& uvBuffers = blockRenderData_->uvBuffers();

  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = blockRenderData_->atlas().texture().imageView().handle();
  imageInfo.sampler = blockRenderData_->atlas().texture().sampler().handle();

  for (uint32_t frame = 0; frame < framesInFlightCount_; ++frame) {
    VkDescriptorBufferInfo uvInfo = {};
    uvInfo.buffer = uvBuffers[frame].handle();
    uvInfo.offset = 0;
    uvInfo.range = uvBuffers[frame].capacityBytes();

    VkWriteDescriptorSet writes[2] = {};

    writes[0] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writes[0].dstSet = descriptorSets_[frame]->handle();
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &uvInfo;

    writes[1] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writes[1].dstSet = descriptorSets_[frame]->handle();
    writes[1].dstBinding = 1;
    writes[1].dstArrayElement = 0;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = &imageInfo;

    device_.dispatchTable().vkUpdateDescriptorSets(device_.handle(), 2u, writes, 0, nullptr);
  }
}

ChunkRenderer::~ChunkRenderer() = default;

void ChunkRenderer::buildAll(VkCommandBuffer cmd, uint32_t currentFrameInFlight) {
  gm::DirtyChunkSet seedDirty;
  for (const auto& [chunkPos, chunkPtr] : chunkManager_->getChunks()) {
    seedDirty.insert(chunkPos);
  }

  meshBuilder_.BuildMeshes(cmd, currentFrameInFlight, seedDirty, chunkManager_->getChunks(),
                           meshes_);
}

void ChunkRenderer::updateDirty(VkCommandBuffer cmd, uint32_t currentFrameInFlight) {
  auto& dirty = chunkManager_->getDirtyChunks();
  if (dirty.empty()) return;

  for (const glm::ivec3& pos : dirty) {
    meshes_.solid.erase(pos);
    meshes_.cutout.erase(pos);
    meshes_.translucent.erase(pos);
  }

  graphicsQueue_->waitIdle();
  meshBuilder_.BuildMeshes(cmd, currentFrameInFlight, dirty, chunkManager_->getChunks(), meshes_);
}

void ChunkRenderer::Render(VkCommandBuffer cmd, float dt, uint32_t currentFrame,
                           const glm::vec3& cameraPos, RenderLayer renderLayer) {
  assert(currentFrame < descriptorSets_.size());

  blockRenderData_->Update(dt, currentFrame);

  VkDescriptorSet set = descriptorSets_[currentFrame]->handle();

  device_.dispatchTable().vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                  pipelines_->pipelineLayout().handle(),
                                                  kDescriptorSetIndex, 1u, &set, 0, nullptr);

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