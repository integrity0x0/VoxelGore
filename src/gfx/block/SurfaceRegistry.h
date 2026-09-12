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
  static constexpr SurfaceId kInvalidSurface = 0;

  SurfaceRegistry(Atlas& atlas) : atlas_(&atlas) {}

  [[nodiscard]] SurfaceId Resolve(const std::string& path, std::vector<UvBuffer>& buffers);

  [[nodiscard]] const UvRegion& ExtractRegion(SurfaceId id) const;

  void UpdateAnimations(float dt, UvBuffer& buffer);

 private:
  [[nodiscard]] SurfaceId Load(const std::string& path, std::vector<UvBuffer>& buffers);
  [[nodiscard]] SurfaceId RegisterSurface(const UvRegion& uvRegion, std::vector<UvBuffer>& buffers);
  [[nodiscard]] SurfaceId RegisterAnimatedSurface(const Animation& animation,
                                                  std::vector<UvBuffer>& buffers);

 private:
  Atlas* atlas_;

  std::vector<Surface> surfaces_;

  std::vector<SurfaceId> animatedIds_;

  std::unordered_map<std::string, SurfaceId> cache_;
};
}  // namespace gfx::block