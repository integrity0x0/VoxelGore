#include "BlockRenderData.h"

namespace gfx {

BlockRenderData::BlockRenderData(const gm::BlockManager& blockManager, const vkcore::Device& device,
                                 vkcore::TransferContext& transferCtxt,
                                 vkcore::MemoryAllocator& memoryAllocator, uint32_t framesInFlight)
    : blockManager_(&blockManager),
      transferCtxt_(&transferCtxt),
      device_(&device),
      atlas_(device, transferCtxt, memoryAllocator, glm::ivec2(2048), 6, 2, 128),
      surfaceRegistry_(atlas_),
      descriptorSetLayout_(BuildDescriptorSetLayout(device)),
      descriptorPool_(BuildDescriptorPool(device, framesInFlight)) {
  uvBuffers_.reserve(framesInFlight);
  for (uint32_t i = 0; i < framesInFlight; ++i) {
    uvBuffers_.emplace_back(device, memoryAllocator, 512u);
  }

  blockInfos_.resize(blockManager.blockCount());

  Build();
  BuildDescriptors(framesInFlight);
}

vkcore::DescriptorSetLayout BlockRenderData::BuildDescriptorSetLayout(
    const vkcore::Device& device) {
  VkDescriptorSetLayoutBinding uvBufferBinding = {};
  uvBufferBinding.binding = 0;
  uvBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uvBufferBinding.descriptorCount = 1;
  uvBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding textureBinding = {};
  textureBinding.binding = 1;
  textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  textureBinding.descriptorCount = 1;
  textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  return vkcore::DescriptorSetLayout(device, std::to_array({uvBufferBinding, textureBinding}));
}

vkcore::DescriptorPool BlockRenderData::BuildDescriptorPool(const vkcore::Device& device,
                                                            uint32_t framesCount) {
  auto poolSizes = std::to_array<VkDescriptorPoolSize>(
      {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, framesCount},
       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, framesCount}});

  return vkcore::DescriptorPool(device, poolSizes, framesCount);
}

void BlockRenderData::BuildDescriptors(uint32_t framesCount) {
  auto poolSizes = std::to_array<VkDescriptorPoolSize>({{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, framesCount},
       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, framesCount}});

  descriptorSets_.reserve(framesCount);
  for (uint32_t i = 0; i < framesCount; ++i) {
    descriptorSets_.push_back(vkcore::DescriptorSet(descriptorPool_.Allocate(descriptorSetLayout_)));
  }

  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = atlas_.texture().imageView().handle();
  imageInfo.sampler = atlas_.texture().sampler().handle();

  for (uint32_t frame = 0; frame < framesCount; ++frame) {
    VkDescriptorBufferInfo uvInfo{};
    uvInfo.buffer = uvBuffers_[frame].handle();
    uvInfo.offset = 0;
    uvInfo.range = uvBuffers_[frame].capacityBytes();

    VkWriteDescriptorSet writes[2] = {};
    writes[0] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writes[0].dstSet = descriptorSets_[frame].handle();
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &uvInfo;

    writes[1] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writes[1].dstSet = descriptorSets_[frame].handle();
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = &imageInfo;

    device_->dispatchTable().vkUpdateDescriptorSets(device_->handle(), 2u, writes, 0, nullptr);
  }
}

void BlockRenderData::Build() {
  for (uint32_t blockId = 0; blockId < blockInfos_.size(); blockId++) {
    auto& info = blockInfos_[blockId];

    const gm::Block* block = blockManager_->block(blockId);

    if (!block) {
      continue;
    }

    for (uint32_t face = 0; face < kFaceCount; face++) {
      info.surfaces[face] = surfaceRegistry_.Resolve(block->getSurface(face), uvBuffers_);
    }

    info.renderGroup = renderGroupRegistry_.RegisterGroup(block->renderGroup());
  }
}

void BlockRenderData::Update(float dt, uint32_t currentFrameInFlight) {
  if (currentFrameInFlight >= uvBuffers_.size()) {
    return;
  }

  surfaceRegistry_.UpdateAnimations(dt, uvBuffers_[currentFrameInFlight]);
}

BlockSurfaceId BlockRenderData::surfaceId(uint32_t blockId, gm::Block::Face face) {
  if (blockId >= blockInfos_.size()) {
    return BlockSurfaceRegistry::kInvalidSurface;
  }

  const size_t faceIndex = static_cast<size_t>(face);

  if (faceIndex >= kFaceCount) {
    return BlockSurfaceRegistry::kInvalidSurface;
  }

  return blockInfos_[blockId].surfaces[faceIndex];
}

RenderGroupId BlockRenderData::renderGroupId(uint32_t blockId) {
  if (blockId >= blockInfos_.size()) {
    return RenderGroupRegistry::kInvalid;
  }

  return blockInfos_[blockId].renderGroup;
}

}  // namespace gfx