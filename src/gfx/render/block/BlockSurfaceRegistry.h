#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

#include "../../common/texture/Atlas.h"
#include "BlockAnimation.h"
#include "BlockSurface.h"
#include "BlockUvBuffer.h"

namespace gfx {

using BlockSurfaceId = uint32_t;

class BlockSurfaceRegistry {
 public:
  static constexpr BlockSurfaceId kInvalidSurface = 0;

  BlockSurfaceRegistry(Atlas& atlas) : atlas_(&atlas) {}

  [[nodiscard]] BlockSurfaceId Resolve(const std::string& path, std::vector<BlockUvBuffer>& buffers);

  [[nodiscard]] const UvRegion& ExtractRegion(BlockSurfaceId id) const;

  void UpdateAnimations(float dt, BlockUvBuffer& buffer);

 private:
  [[nodiscard]] BlockSurfaceId Load(const std::string& path, std::vector<BlockUvBuffer>& buffers);
  [[nodiscard]] BlockSurfaceId RegisterSurface(const UvRegion& uvRegion, std::vector<BlockUvBuffer>& buffers);
  [[nodiscard]] BlockSurfaceId RegisterAnimatedSurface(const BlockAnimation& animation,
                                                  std::vector<BlockUvBuffer>& buffers);

 private:
  Atlas* atlas_;

  std::vector<BlockSurface> surfaces_;

  std::vector<BlockSurfaceId> animatedIds_;

  std::unordered_map<std::string, BlockSurfaceId> cache_;
};
}  // namespace gfx