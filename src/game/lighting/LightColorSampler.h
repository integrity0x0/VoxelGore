// LightColorSampler.h
#pragma once

#include <array>
#include <glm/glm.hpp>

#include "LightChunkStorage.h"
#include "Lighting.h"

namespace gm {
class Chunk;
}

namespace gm::lighting {

class LightColorSampler {
 public:

  LightColorSampler(const Lighting& lighting, const glm::ivec3& chunkPos,
                    const std::array<const gm::Chunk*, 27>& neighbors);

  glm::vec4 GetColor(glm::ivec3 localPos) const;

  std::array<glm::vec4, 4> SampleFace(glm::ivec3 localPos, uint32_t face) const;

 private:
  const LightChunkStorage::ChunkData* ResolveLightChunk(glm::ivec3& localPos) const;

  const Lighting* lighting_;
  const LightChunkStorage* storage_;
  glm::ivec3 chunkPos_;

  mutable const LightChunkStorage::ChunkData* lastData_ = nullptr;
  mutable glm::ivec3 lastChunkPos_{std::numeric_limits<int>::max()};
};

}  // namespace gm::lighting