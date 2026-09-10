#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

#include "../texture/Atlas.h"
#include "Animation.h"
#include "Surface.h"
#include "UvBuffer.h"

namespace gfx::block {

using SurfaceId = uint32_t;

class SurfaceRegistry {
 public:
  static constexpr SurfaceId kInvalidSurface = 0u;

  SurfaceRegistry(Atlas& atlas) : atlas_(&atlas) {}

  [[nodiscard]] SurfaceId resolve(const std::string& path, std::vector<UvBuffer>& buffers);

  [[nodiscard]] const UvRegion& extractRegion(SurfaceId id) const;

  void updateAnimations(float dt, UvBuffer& buffer);

 private:
  [[nodiscard]] SurfaceId load(const std::string& path, std::vector<UvBuffer>& buffers);
  [[nodiscard]] SurfaceId registerSurface(const UvRegion& uvRegion, std::vector<UvBuffer>& buffers);
  [[nodiscard]] SurfaceId registerAnimatedSurface(const Animation& animation,
                                                  std::vector<UvBuffer>& buffers);

 private:
  Atlas* atlas_;

  std::vector<Surface> surfaces_;

  std::vector<SurfaceId> animatedIds_;

  std::unordered_map<std::string, SurfaceId> cache_;
};
}  // namespace gfx::block