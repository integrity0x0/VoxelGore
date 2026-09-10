#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "../lighting/LightMap.h"
#include "Voxel.h"

namespace gm {

class Chunk {
 public:
  static constexpr size_t kLength = 32ull;
  static constexpr size_t kVolume = kLength * kLength * kLength;

  explicit Chunk(const glm::ivec3& pos);

  [[nodiscard]] Voxel& getVoxel(const glm::ivec3& localPos);
  [[nodiscard]] const Voxel& getVoxel(const glm::ivec3& localPos) const;

  void setVoxel(const glm::ivec3& localPos, const Voxel& voxel) {
    if (localPos.x >= kLength || localPos.y >= kLength || localPos.z >= kLength) return;

    size_t voxelIndex = ArrayIndex(localPos);

    if (voxelIndex >= kVolume) return;

    bool wasEmpty = voxels_[voxelIndex].id == 0;
    bool isEmpty = voxel.id == 0;

    if (wasEmpty && !isEmpty) {
      ++nonEmptyCount_;
    } else if (!wasEmpty && isEmpty) {
      --nonEmptyCount_;
    }

    voxels_[voxelIndex] = voxel;
  }

  [[nodiscard]] glm::ivec3 pos() const { return pos_; }

  [[nodiscard]] LightMap& lightMap() { return lightMap_; }
  [[nodiscard]] const LightMap& lightMap() const { return lightMap_; }

  [[nodiscard]] bool IsEmpty() const { return nonEmptyCount_ == 0ull; }

 private:
  size_t ArrayIndex(const glm::ivec3& localPos) const {
    return static_cast<size_t>((localPos.x * kLength + localPos.y) * kLength + localPos.z);
  }
  std::unique_ptr<Voxel[]> voxels_;
  glm::ivec3 pos_;
  LightMap lightMap_;
  size_t nonEmptyCount_ = 0ull;
};

}  // namespace gm