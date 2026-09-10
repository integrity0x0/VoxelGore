#pragma once

#include <unordered_map>

#include "../../../game/voxel/ChunkManager.h"
#include "../../../util/hashers.h"
#include "../../mesh/Mesh.h"
#include "../../mesh/TranslucentMesh.h"

namespace gfx {
struct ChunkMeshes {
 public:
  std::unordered_map<glm::ivec3, Mesh, util::Vec3Hash> solid;
  std::unordered_map<glm::ivec3, Mesh, util::Vec3Hash> cutout;
  std::unordered_map<glm::ivec3, TranslucentMesh, util::Vec3Hash> translucent;
};
}  // namespace gfx