#pragma once

#include <stdint.h>

#include <array>
#include <cassert>
#include <vector>

#include "../../block/RenderData.h"
#include "../RenderLayers.h"
#include "ChunkMeshes.h"

namespace gfx {
class ChunkMeshBuilder {
 public:
  struct Vertex {
    glm::vec3 pos;
    glm::vec4 color;
    uint8_t faceIndex;
    uint8_t cornerIndex;
    uint32_t blockSurfaceId;
  };

  ChunkMeshBuilder(const vkcore::Device& device, const gm::BlockManager& blockManager,
                   block::RenderData& blockRenderData, uint32_t framesCount);

  void BuildMeshes(VkCommandBuffer cmd, uint32_t currentFrame, gm::DirtyChunkSet& dirtyChunks,
                   const gm::ChunksMap& chunksMap, ChunkMeshes& meshes);

 private:
  class MeshStream {
   public:
    static constexpr uint32_t kVerticesPerQuad = 4u;
    static constexpr uint32_t kIndicesPerQuad = 6u;

    MeshStream() = default;

    MeshStream(uint8_t* mappedBase, VkDeviceSize baseOffset, VkDeviceSize regionBytes) {
      const VkDeviceSize vertexBytes = VertexBytesOf(regionBytes);
      const VkDeviceSize indexBytes = regionBytes - vertexBytes;

      vertexStagingOffset_ = baseOffset;
      indexStagingOffset_ = baseOffset + vertexBytes;

      vertexData_ = reinterpret_cast<Vertex*>(mappedBase + vertexStagingOffset_);
      indexData_ = reinterpret_cast<uint32_t*>(mappedBase + indexStagingOffset_);

      vertexCapacity_ = static_cast<uint32_t>(vertexBytes / sizeof(Vertex));
      indexCapacity_ = static_cast<uint32_t>(indexBytes / sizeof(uint32_t));
    }

    bool CanFit(uint32_t quadCount) const {
      return vertexCount_ + quadCount * kVerticesPerQuad <= vertexCapacity_ &&
             indexCount_ + quadCount * kIndicesPerQuad <= indexCapacity_;
    }

    bool empty() const { return vertexCount_ == 0u; }

    uint32_t PushQuad(const std::array<Vertex, 4>& vertices,
                      const std::array<uint32_t, 6>& localIndices) {
      assert(CanFit(1));

      const uint32_t baseVertex = vertexCount_;

      for (size_t i = 0u; i < kVerticesPerQuad; ++i) {
        vertexData_[vertexOffset_ + vertexCount_++] = vertices[i];
      }
      for (size_t i = 0u; i < kIndicesPerQuad; ++i) {
        indexData_[indexOffset_ + indexCount_++] = baseVertex + localIndices[i];
      }

      return baseVertex;
    }

    void CommitChunk() {
      vertexOffset_ += vertexCount_;
      indexOffset_ += indexCount_;
      vertexCount_ = 0u;
      indexCount_ = 0u;
    }

    void Reset() {
      vertexOffset_ = 0u;
      indexOffset_ = 0u;
      vertexCount_ = 0u;
      indexCount_ = 0u;
    }

    uint32_t vertexCount() const { return vertexCount_; }
    uint32_t indexCount() const { return indexCount_; }

    VkDeviceSize VertexSrcOffset() const {
      return vertexStagingOffset_ + static_cast<VkDeviceSize>(vertexOffset_) * sizeof(Vertex);
    }
    VkDeviceSize VertexBytes() const {
      return static_cast<VkDeviceSize>(vertexCount_) * sizeof(Vertex);
    }
    VkDeviceSize IndexSrcOffset() const {
      return indexStagingOffset_ + static_cast<VkDeviceSize>(indexOffset_) * sizeof(uint32_t);
    }
    VkDeviceSize IndexBytes() const {
      return static_cast<VkDeviceSize>(indexCount_) * sizeof(uint32_t);
    }
    const uint32_t* IndexData() const { return indexData_ + indexOffset_; }

   private:
    static constexpr VkDeviceSize kQuadVertexBytes = kVerticesPerQuad * sizeof(Vertex);
    static constexpr VkDeviceSize kQuadIndexBytes = kIndicesPerQuad * sizeof(uint32_t);
    static constexpr VkDeviceSize kQuadTotalBytes = kQuadVertexBytes + kQuadIndexBytes;

    static constexpr VkDeviceSize VertexBytesOf(VkDeviceSize regionBytes) {
      return regionBytes * kQuadVertexBytes / kQuadTotalBytes;
    }

    VkDeviceSize vertexStagingOffset_ = 0;
    VkDeviceSize indexStagingOffset_ = 0;

    Vertex* vertexData_ = nullptr;
    uint32_t* indexData_ = nullptr;

    uint32_t vertexCapacity_ = 0;
    uint32_t indexCapacity_ = 0;

    uint32_t vertexOffset_ = 0;
    uint32_t indexOffset_ = 0;

    uint32_t vertexCount_ = 0;
    uint32_t indexCount_ = 0;
  };
  class StagingInfo {
   public:
    explicit StagingInfo(vkcore::BufferAllocator& bufferAllocator)
        : buffer_(bufferAllocator.Allocate(
              kBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
      uint8_t* base = reinterpret_cast<uint8_t*>(buffer_.map());

      constexpr VkDeviceSize kLayerSizes[] = {kSolidSize, kCutoutSize, kTranslucentSize};

      VkDeviceSize runningOffset = 0;
      for (uint32_t layer = 0; layer < kLayerCount; ++layer) {
        meshStreams_[layer] = MeshStream(base, runningOffset, kLayerSizes[layer]);
        runningOffset += kLayerSizes[layer];
      }
    }

    MeshStream& layer(RenderLayer renderLayer) {
      return meshStreams_[static_cast<size_t>(renderLayer)];
    }
    const MeshStream& layer(RenderLayer renderLayer) const {
      return meshStreams_[static_cast<size_t>(renderLayer)];
    }

    void AddTranslucentQuad(const glm::vec3& center, const std::array<uint32_t, 6>& indices) {
      TranslucentMesh::QuadEntry entry{};
      entry.center = center;
      for (size_t i = 0; i < 6; ++i) entry.indices[i] = indices[i];
      translucentQuadEntries_.push_back(entry);
    }

    const std::vector<TranslucentMesh::QuadEntry>& TranslucentQuads() const {
      return translucentQuadEntries_;
    }

    VkBuffer BufferHandle() const { return buffer_.handle(); }

    void Reset() {
      for (auto& stream : meshStreams_) stream.Reset();
      translucentQuadEntries_.clear();
    }

    void CommitChunk() {
      for (auto& stream : meshStreams_) stream.CommitChunk();
      translucentQuadEntries_.clear();
    }

   private:
    static constexpr uint32_t kLayerCount = static_cast<uint32_t>(RenderLayer::Count);

    static constexpr VkDeviceSize kSolidSize = 12 * 1024 * 1024ull;
    static constexpr VkDeviceSize kCutoutSize = 8 * 1024 * 1024ull;
    static constexpr VkDeviceSize kTranslucentSize = 8 * 1024 * 1024ull;
    static constexpr VkDeviceSize kBufferSize = kSolidSize + kCutoutSize + kTranslucentSize;

    std::array<MeshStream, kLayerCount> meshStreams_;
    std::vector<TranslucentMesh::QuadEntry> translucentQuadEntries_;
    vkcore::BufferSlice buffer_;
  };

  bool BuildChunk(VkCommandBuffer cmd, StagingInfo& staging, const gm::ChunksMap& chunksMap,
                  const gm::Chunk& chunk);

  bool AddFace(StagingInfo& staging, uint32_t face, const glm::vec3& worldOffset,
               uint32_t surfaceId, const std::array<glm::vec4, 4>& cornerColors,
               RenderLayer renderLayer);

  std::optional<Mesh> MakeMeshLayer(VkCommandBuffer cmd, const StagingInfo& staging,
                                    RenderLayer renderLayer);
  std::optional<TranslucentMesh> MakeTranslucentMesh(VkCommandBuffer cmd, StagingInfo& staging);

  const vkcore::Device* device_;
  const gm::BlockManager* blockManager_;
  block::RenderData* blockRenderData_;

  vkcore::MemoryAllocator memoryAllocator_;
  vkcore::BufferAllocator bufferAllocator_;

  std::vector<StagingInfo> stagingInfos_;

  uint32_t framesCount_;
};
}  // namespace gfx