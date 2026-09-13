#include "ChunkMeshBuilder.h"

namespace gfx {

namespace {

static constexpr glm::ivec3 kFaceNormals[6u] = {
    {0, 0, -1},  // Z-
    {0, 0, 1},   // Z+
    {-1, 0, 0},  // X-
    {1, 0, 0},   // X+
    {0, -1, 0},  // Y-
    {0, 1, 0},   // Y+
};

static constexpr glm::ivec3 kFaceRight[6u] = {
    {1, 0, 0},  // 0: -Z
    {1, 0, 0},  // 1: +Z
    {0, 0, 1},  // 2: -X
    {0, 0, 1},  // 3: +X
    {1, 0, 0},  // 4: -Y
    {1, 0, 0},  // 5: +Y
};

static constexpr glm::ivec3 kFaceUp[6u] = {
    {0, 1, 0},  // 0: -Z
    {0, 1, 0},  // 1: +Z
    {0, 1, 0},  // 2: -X
    {0, 1, 0},  // 3: +X
    {0, 0, 1},  // 4: -Y
    {0, 0, 1},  // 5: +Y
};

#include "facePositions.inl"

struct CornerSign {
  int8_t right;
  int8_t up;
};

static std::array<CornerSign, 24> BuildCornerSigns() {
  std::array<CornerSign, 24> signs{};
  for (uint32_t face = 0; face < 6u; ++face) {
    for (uint32_t corner = 0; corner < 4u; ++corner) {
      uint32_t idx = face * 4u + corner;
      const glm::vec3& p = kFaceVertices[idx].pos;
      float rightDot = glm::dot(p, glm::vec3(kFaceRight[face]));
      float upDot = glm::dot(p, glm::vec3(kFaceUp[face]));
      signs[idx].right = rightDot > 0.0f ? 1 : -1;
      signs[idx].up = upDot > 0.0f ? 1 : -1;
    }
  }
  return signs;
}

static const std::array<CornerSign, 24> kCornerSigns = BuildCornerSigns();

static constexpr int32_t kChunkCubeLen = 3;
static constexpr size_t kChunkCubeVolume =
    static_cast<size_t>(kChunkCubeLen) * kChunkCubeLen * kChunkCubeLen;

static uint32_t ChunkCubeIndex(glm::ivec3 offset) {
  offset += glm::ivec3(1);
  return static_cast<uint32_t>((offset.x * kChunkCubeLen + offset.y) * kChunkCubeLen + offset.z);
}

static std::array<const gm::Chunk*, kChunkCubeVolume> CollectNeighbors(
    const gm::ChunksMap& chunksMap, const glm::ivec3& pos) {
  std::array<const gm::Chunk*, kChunkCubeVolume> neighbors{};

  for (int32_t dx = -1; dx <= 1; ++dx) {
    for (int32_t dy = -1; dy <= 1; ++dy) {
      for (int32_t dz = -1; dz <= 1; ++dz) {
        glm::ivec3 offset(dx, dy, dz);
        const auto& it = chunksMap.find(pos + offset);

        neighbors[ChunkCubeIndex(offset)] = (it == chunksMap.end()) ? nullptr : it->second.get();
      }
    }
  }

  return neighbors;
}

static const gm::Chunk* ResolveChunk(
    const gm::Chunk& self, const std::array<const gm::Chunk*, kChunkCubeVolume>& neighbors,
    glm::ivec3& localPos) {
  glm::ivec3 chunkOffset(0);
  chunkOffset.x =
      localPos.x < 0 ? -1 : (localPos.x >= static_cast<int32_t>(gm::Chunk::kLength) ? 1 : 0);
  chunkOffset.y =
      localPos.y < 0 ? -1 : (localPos.y >= static_cast<int32_t>(gm::Chunk::kLength) ? 1 : 0);
  chunkOffset.z =
      localPos.z < 0 ? -1 : (localPos.z >= static_cast<int32_t>(gm::Chunk::kLength) ? 1 : 0);

  if (chunkOffset == glm::ivec3(0)) return &self;

  int32_t len = static_cast<int32_t>(gm::Chunk::kLength);
  localPos.x = (localPos.x % len + len) % len;
  localPos.y = (localPos.y % len + len) % len;
  localPos.z = (localPos.z % len + len) % len;

  return neighbors[ChunkCubeIndex(chunkOffset)];
}

glm::vec4 LightSample(const gm::Chunk& self,
                      const std::array<const gm::Chunk*, kChunkCubeVolume>& neighbors,
                      glm::ivec3 localPos, uint32_t face, uint32_t corner) {
  glm::ivec3 facePos = localPos + kFaceNormals[face];

  const CornerSign& sign = kCornerSigns[face * 4u + corner];
  glm::ivec3 rightOffset = kFaceRight[face] * static_cast<int32_t>(sign.right);
  glm::ivec3 upOffset = kFaceUp[face] * static_cast<int32_t>(sign.up);

  auto sampleOne = [&](glm::ivec3 pos) -> glm::ivec4 {
    const gm::Chunk* chunk = ResolveChunk(self, neighbors, pos);
    if (!chunk) return glm::ivec4(0);
    const gm::LightMap& lm = chunk->lightMap();
    return glm::ivec4(lm.getR(pos), lm.getG(pos), lm.getB(pos), lm.getS(pos));
  };

  glm::ivec4 sum = sampleOne(facePos) + sampleOne(facePos + upOffset) +
                   sampleOne(facePos + rightOffset) + sampleOne(facePos + rightOffset + upOffset);

  return glm::vec4(sum) * 0.25f;
}

bool IsBlocked(block::RenderData& renderData, const gm::Chunk& self,
               const std::array<const gm::Chunk*, kChunkCubeVolume>& neighbors,
               glm::ivec3 neighborLocalPos, uint32_t sourceVoxelId) {
  const gm::Chunk* target = ResolveChunk(self, neighbors, neighborLocalPos);
  if (!target) return true;

  uint32_t neighborId = target->GetVoxel(neighborLocalPos).id;
  return renderData.renderGroupId(neighborId) == renderData.renderGroupId(sourceVoxelId);
}

}  // namespace

ChunkMeshBuilder::ChunkMeshBuilder(const vkcore::Device& device,
                                   const gm::BlockManager& blockManager,
                                   block::RenderData& blockRenderData, uint32_t framesCount)
    : device_(&device),
      blockManager_(&blockManager),
      blockRenderData_(&blockRenderData),
      memoryAllocator_(device),
      bufferAllocator_(device, memoryAllocator_),
      framesCount_(framesCount) {
  stagingInfos_.reserve(framesCount);

  for (size_t i = 0; i < static_cast<uint32_t>(framesCount); ++i) {
    stagingInfos_.emplace_back(bufferAllocator_);
  }
}

bool ChunkMeshBuilder::AddFace(StagingInfo& staging, uint32_t face, const glm::vec3& worldOffset,
                               uint32_t surfaceId, const std::array<glm::vec4, 4>& cornerColors,
                               RenderLayer renderLayer) {
  MeshStream& stream = staging.layer(renderLayer);

  if (!stream.CanFit(1)) {
    return false;
  }

  std::array<Vertex, 4> vertices;
  for (uint32_t corner = 0; corner < 4u; ++corner) {
    vertices[corner] = kFaceVertices[face * 4u + corner];
    vertices[corner].pos += worldOffset;
    vertices[corner].color = cornerColors[corner];
    vertices[corner].blockSurfaceId = surfaceId;
  }

  std::array<uint32_t, 6> localIndices;
  for (uint32_t i = 0; i < 6u; ++i) {
    localIndices[i] = kFaceIndices[face * 6u + i];
  }

  const uint32_t baseVertex = stream.PushQuad(vertices, localIndices);

  if (renderLayer == RenderLayer::Translucent) {
    std::array<uint32_t, 6> globalIndices;
    for (size_t i = 0; i < 6u; ++i) globalIndices[i] = baseVertex + localIndices[i];
    staging.AddTranslucentQuad(worldOffset, globalIndices);
  }

  return true;
}

std::optional<Mesh> ChunkMeshBuilder::MakeMeshLayer(VkCommandBuffer cmd, const StagingInfo& staging,
                                                    RenderLayer renderLayer) {
  const MeshStream& stream = staging.layer(renderLayer);

  if (stream.empty()) {
    return std::nullopt;
  }

  vkcore::BufferSlice vertexBuffer = bufferAllocator_.Allocate(
      stream.VertexBytes(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vkcore::BufferSlice indexBuffer = bufferAllocator_.Allocate(
      stream.IndexBytes(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VkBufferCopy vertexRegion{};
  vertexRegion.srcOffset = stream.VertexSrcOffset();
  vertexRegion.dstOffset = vertexBuffer.offset();
  vertexRegion.size = stream.VertexBytes();
  device_->dispatchTable().vkCmdCopyBuffer(cmd, staging.BufferHandle(), vertexBuffer.handle(), 1,
                                           &vertexRegion);

  VkBufferCopy indexRegion{};
  indexRegion.srcOffset = stream.IndexSrcOffset();
  indexRegion.dstOffset = indexBuffer.offset();
  indexRegion.size = stream.IndexBytes();
  device_->dispatchTable().vkCmdCopyBuffer(cmd, staging.BufferHandle(), indexBuffer.handle(), 1,
                                           &indexRegion);

  return Mesh(*device_, std::move(vertexBuffer), stream.vertexCount(), std::move(indexBuffer),
              stream.indexCount(), VK_INDEX_TYPE_UINT32);
}

std::optional<TranslucentMesh> ChunkMeshBuilder::MakeTranslucentMesh(VkCommandBuffer cmd,
                                                                     StagingInfo& staging) {
  MeshStream& stream = staging.layer(RenderLayer::Translucent);
  if (stream.empty()) return std::nullopt;

  vkcore::BufferSlice vertexBuffer = bufferAllocator_.Allocate(
      stream.VertexBytes(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VkBufferCopy vertexRegion{};
  vertexRegion.srcOffset = stream.VertexSrcOffset();
  vertexRegion.dstOffset = vertexBuffer.offset();
  vertexRegion.size = stream.VertexBytes();
  device_->dispatchTable().vkCmdCopyBuffer(cmd, staging.BufferHandle(), vertexBuffer.handle(), 1,
                                           &vertexRegion);

  Mesh mesh(*device_, std::move(vertexBuffer), stream.vertexCount());

  return TranslucentMesh(*device_, std::move(mesh), bufferAllocator_, framesCount_,
                         staging.TranslucentQuads());
}

bool ChunkMeshBuilder::BuildChunk(VkCommandBuffer cmd, StagingInfo& staging,
                                  const gm::ChunksMap& chunksMap, const gm::Chunk& chunk) {
  auto neighbors = CollectNeighbors(chunksMap, chunk.pos());

  for (uint32_t face = 0; face < 6u; face++) {
    for (uint32_t x = 0; x < gm::Chunk::kLength; x++) {
      for (uint32_t y = 0; y < gm::Chunk::kLength; y++) {
        for (uint32_t z = 0; z < gm::Chunk::kLength; z++) {
          glm::ivec3 localPos(x, y, z);
          const gm::Voxel& v = chunk.GetVoxel(localPos);

          if (!v.id) continue;

          const gm::Block* block = blockManager_->block(v.id);

          if (IsBlocked(*blockRenderData_, chunk, neighbors, localPos + kFaceNormals[face], v.id))
            continue;

          RenderLayer renderLayer = ToRenderLayer(block->renderLayer());

          if (renderLayer >= RenderLayer::Count) continue;

          uint32_t blockSurfaceId =
              block ? blockRenderData_->surfaceId(v.id, static_cast<gm::Block::Face>(face)) : 0;

          std::array<glm::vec4, 4> cornerColors;
          if (block && block->isIgnoreLighting()) {
            cornerColors.fill(glm::vec4(1.0f));
          } else {
            for (uint32_t corner = 0; corner < 4u; ++corner) {
              cornerColors[corner] = LightSample(chunk, neighbors, localPos, face, corner) / 15.0f;
            }
          }

          glm::vec3 worldOffset = glm::vec3(localPos) + glm::vec3(0.5f) +
                                  glm::vec3(chunk.pos()) * static_cast<float>(gm::Chunk::kLength);

          if (!AddFace(staging, face, worldOffset, blockSurfaceId, cornerColors, renderLayer))
            return false;
        }
      }
    }
  }

  return true;
}

void ChunkMeshBuilder::BuildMeshes(VkCommandBuffer cmd, uint32_t currentFrame,
                                   gm::DirtyChunkSet& dirtyChunks, const gm::ChunksMap& chunksMap,
                                   ChunkMeshes& meshes) {
  StagingInfo& staging = stagingInfos_[currentFrame];
  staging.Reset();

  auto it = dirtyChunks.begin();
  while (it != dirtyChunks.end()) {
    const glm::ivec3& pos = *it;

    auto chunkIt = chunksMap.find(pos);
    if (chunkIt == chunksMap.end() || chunkIt->second->IsEmpty()) {
      it = dirtyChunks.erase(it);
      continue;
    }

    const gm::Chunk& chunk = *chunkIt->second;

    if (!BuildChunk(cmd, staging, chunksMap, chunk)) break;

    if (auto mesh = MakeMeshLayer(cmd, staging, RenderLayer::Solid)) {
      meshes.solid.emplace(pos, std::move(*mesh));
    }

    if (auto mesh = MakeMeshLayer(cmd, staging, RenderLayer::Cutout)) {
      meshes.cutout.emplace(pos, std::move(*mesh));
    }

    if (auto mesh = MakeTranslucentMesh(cmd, staging)) {
      meshes.translucent.emplace(pos, std::move(*mesh));
    }

    staging.CommitChunk();
    it = dirtyChunks.erase(it);
  }
}

}  // namespace gfx