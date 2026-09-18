#pragma once

#include "../../../game/voxel/BlockManager.h"
#include "../../../game/voxel/ChunkManager.h"
#include "../block/BlockRenderData.h"
#include "ParticleEmitter.h"
#include "glm/glm.hpp"

namespace gfx {

class BlockDebrisEmitter final : public ParticleEmitter {
 public:
  BlockDebrisEmitter(BlockRenderData& renderData, const gm::BlockManager& blockManager,
                     const gm::ChunkManager& chunkManager);

  void Spawn(uint32_t blockId, const glm::vec3& position);

  void Update(float dt) override;

 private:
  static constexpr float kBlockSize = 1.0f;

  BlockRenderData* renderData_;
  const gm::BlockManager* blockManager_;
  const gm::ChunkManager* chunkManager_;

  void updateParticles(float dt) override;
};

}  // namespace gfx