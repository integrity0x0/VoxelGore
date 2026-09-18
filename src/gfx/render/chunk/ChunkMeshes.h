#pragma once

#include <unordered_map>

#include "../../../game/voxel/ChunkManager.h"
#include "../../../util/hashers.h"
#include "../../common/mesh/Mesh.h"
#include "../../common/mesh/TranslucentMesh.h"

namespace gfx {
struct ChunkMeshes {
 public:
  std::unordered_map<glm::ivec3, Mesh, util::IVec3Hash> solid;
  std::unordered_map<glm::ivec3, Mesh, util::IVec3Hash> cutout;
  std::unordered_map<glm::ivec3, TranslucentMesh, util::IVec3Hash> translucent;
};
}  // namespace gfx