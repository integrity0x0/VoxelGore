#pragma once

#include <vector>

#include "../../UvRegion.h"
#include "Atlas.h"
#include "../../../../include/glm/glm.hpp"

namespace gfx {

class SpriteSheet {
 public:
  class Playback {
   public:
    Playback(const SpriteSheet& anim, float duration) : anim_(&anim), duration_(duration) {}
    void tick(float dt);
    const UvRegion& currentUv() const;

   private:
    const SpriteSheet* anim_;
    float currentTime_ = 0.0f;
    float duration_;
    uint32_t currentFrame_ = 0;
  };

  enum class Orientation { Vertical, Horizontal };

  SpriteSheet(const AtlasRegion& region, glm::ivec2 frameSize, Orientation orientation,
              uint32_t row);

  const UvRegion& frameAt(uint32_t index) const { return frames_[index % frames_.size()]; }
  uint32_t frameCount() const { return static_cast<uint32_t>(frames_.size()); }

 private:
  std::vector<UvRegion> frames_;
};

}  // namespace gfx