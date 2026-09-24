#pragma once

#include <glm/glm.hpp>

#include "../../../game/voxel/BlockManager.h"
#include "../../../game/voxel/ChunkManager.h"
#include "../block/BlockRenderData.h"
#include "PhysicalParticleEmitter.h"

namespace gfx {

class BlockDebrisEmitter final : public PhysicalParticleEmitter {
 public:
  BlockDebrisEmitter(BlockRenderData& renderData, const gm::ChunkManager& chunkManager, 
					 const gm::BlockManager& blockManager);

  void Spawn(uint32_t blockId, const glm::vec3& position);

  void Update(float dt) override;

 private:
  static constexpr float kBlockSize = 1.0f;

  BlockRenderData* renderData_;
};

}  // namespace gfx