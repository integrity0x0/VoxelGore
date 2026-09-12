#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "../../util/hashers.h"
#include "Chunk.h"

namespace gm {
using ChunksMap = std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, util::Vec3Hash>;
using DirtyChunkSet = std::unordered_set<glm::ivec3, util::Vec3Hash>;

class ChunkManager {
 public:
  ChunkManager(uint32_t width, uint32_t height, uint32_t depth);

  bool isDirty(const glm::ivec3& chunkPos) const {
    return dirtyChunks_.find(chunkPos) != dirtyChunks_.end();
  }

  Chunk* getChunk(const glm::ivec3& chunkPos) {
    auto it = chunks_.find(chunkPos);
    return it != chunks_.end() ? it->second.get() : nullptr;
  }

  const Chunk* getChunk(const glm::ivec3& chunkPos) const {
    auto it = chunks_.find(chunkPos);
    return it != chunks_.end() ? it->second.get() : nullptr;
  }

  bool hasChunk(const glm::ivec3& chunkPos) const {
    return chunks_.find(chunkPos) != chunks_.end();
  }

  bool hasVoxel(const glm::ivec3& worldPos) const {
    glm::ivec3 chunkPos = toChunkPos(worldPos);
    return hasChunk(chunkPos);
  }

  std::optional<Voxel> getVoxel(const glm::ivec3& worldPos) const {
    glm::ivec3 chunkPos = toChunkPos(worldPos);
    const Chunk* chunk = getChunk(chunkPos);
    if (!chunk) {
      return std::nullopt;
    }
    return chunk->getVoxel(toLocalPos(worldPos, chunkPos));
  }

  bool setVoxel(const glm::ivec3& worldPos, const Voxel& value) {
    glm::ivec3 chunkPos = toChunkPos(worldPos);
    Chunk* chunk = getChunk(chunkPos);
    if (!chunk) {
      return false;
    }
    glm::ivec3 localPos = toLocalPos(worldPos, chunkPos);
    chunk->setVoxel(localPos, value);
    markDirtyWithNeighbors(chunkPos, localPos);
    return true;
  }

  glm::vec3 getLightColor(const glm::ivec3& worldPos, const glm::vec3& ambientColor = glm::vec3(1.0f)) const {
    const uint16_t light = getLight(worldPos);

    constexpr float kMaxLight = 15.0f;

    const float r = static_cast<float>((light >> 0) & 0xF) / kMaxLight;
    const float g = static_cast<float>((light >> 4) & 0xF) / kMaxLight;
    const float b = static_cast<float>((light >> 8) & 0xF) / kMaxLight;
    const float s = static_cast<float>((light >> 12) & 0xF) / kMaxLight;

    glm::vec3 sunLight = ambientColor * s;

    return glm::clamp(glm::vec3(r, g, b) + sunLight, 0.0f, 1.0f);
  }

  uint32_t getLight(const glm::ivec3& worldPos, LightChannel channel) const {
    glm::ivec3 chunkPos = toChunkPos(worldPos);
    const Chunk* chunk = getChunk(chunkPos);
    if (!chunk) {
      return 0u;
    }
    glm::ivec3 localPos = toLocalPos(worldPos, chunkPos);
    const LightMap& lightMap = chunk->lightMap();
    switch (channel) {
      case LightChannel::R:
        return lightMap.getR(localPos);
      case LightChannel::G:
        return lightMap.getG(localPos);
      case LightChannel::B:
        return lightMap.getB(localPos);
      case LightChannel::S:
        return lightMap.getS(localPos);
    }
    return 15u;
  }

  uint16_t getLight(const glm::ivec3& worldPos) const {
    glm::ivec3 chunkPos = toChunkPos(worldPos);
    const Chunk* chunk = getChunk(chunkPos);
    if (!chunk) {
      return 0;
    }
    glm::ivec3 localPos = toLocalPos(worldPos, chunkPos);
    return chunk->lightMap().get(localPos);
  }

  bool setLight(const glm::ivec3& worldPos, LightChannel channel, uint8_t value) {
    glm::ivec3 chunkPos = toChunkPos(worldPos);
    Chunk* chunk = getChunk(chunkPos);
    if (!chunk) {
      return false;
    }
    glm::ivec3 localPos = toLocalPos(worldPos, chunkPos);
    LightMap& lightMap = chunk->lightMap();
    switch (channel) {
      case LightChannel::R:
        lightMap.setR(localPos, value);
        break;
      case LightChannel::G:
        lightMap.setG(localPos, value);
        break;
      case LightChannel::B:
        lightMap.setB(localPos, value);
        break;
      case LightChannel::S:
        lightMap.setS(localPos, value);
        break;
    }
    markDirtyWithNeighbors(chunkPos, localPos);
    return true;
  }

  DirtyChunkSet& getDirtyChunks() { return dirtyChunks_; }

  const DirtyChunkSet& getDirtyChunks() const { return dirtyChunks_; }

  std::optional<glm::ivec3> popDirtyChunk() {
    if (dirtyChunks_.empty()) {
      return std::nullopt;
    }
    auto it = dirtyChunks_.begin();
    glm::ivec3 chunkPos = *it;
    dirtyChunks_.erase(it);
    return chunkPos;
  }

  void clearDirty() { dirtyChunks_.clear(); }

  uint32_t width() const { return width_; }
  uint32_t height() const { return height_; }
  uint32_t depth() const { return depth_; }
  const ChunksMap& getChunks() const { return chunks_; }

 private:
  static glm::ivec3 toChunkPos(const glm::ivec3& worldPos) {
    constexpr int L = static_cast<int>(Chunk::kLength);
    return glm::ivec3{floorDiv(worldPos.x, L), floorDiv(worldPos.y, L), floorDiv(worldPos.z, L)};
  }

  static glm::ivec3 toLocalPos(const glm::ivec3& worldPos, const glm::ivec3& chunkPos) {
    constexpr int L = static_cast<int>(Chunk::kLength);
    return worldPos - chunkPos * L;
  }

  static int floorDiv(int a, int b) {
    int d = a / b;
    int r = a % b;
    return (r != 0 && ((r < 0) != (b < 0))) ? d - 1 : d;
  }

  void markDirtyWithNeighbors(const glm::ivec3& chunkPos, const glm::ivec3& localPos) {
    dirtyChunks_.insert(chunkPos);

    constexpr int last = Chunk::kLength - 1;

    if (localPos.x == 0) {
      dirtyChunks_.insert(chunkPos + glm::ivec3(-1, 0, 0));
    }
    if (localPos.x == last) {
      dirtyChunks_.insert(chunkPos + glm::ivec3(1, 0, 0));
    }
    if (localPos.y == 0) {
      dirtyChunks_.insert(chunkPos + glm::ivec3(0, -1, 0));
    }
    if (localPos.y == last) {
      dirtyChunks_.insert(chunkPos + glm::ivec3(0, 1, 0));
    }
    if (localPos.z == 0) {
      dirtyChunks_.insert(chunkPos + glm::ivec3(0, 0, -1));
    }
    if (localPos.z == last) {
      dirtyChunks_.insert(chunkPos + glm::ivec3(0, 0, 1));
    }
  }

  ChunksMap chunks_;
  DirtyChunkSet dirtyChunks_;
  uint32_t width_;
  uint32_t height_;
  uint32_t depth_;
};

}  // namespace gm