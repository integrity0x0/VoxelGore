#include "SpriteSheet.h"

namespace gfx {

void SpriteSheet::Playback::tick(float dt) {
  float duration = duration_;
  if (duration <= 0.0f || anim_->frameCount() <= 1) {
    return;
  }

  currentTime_ += dt;
  while (currentTime_ >= duration) {
    currentTime_ -= duration;
    currentFrame_ = (currentFrame_ + 1) % anim_->frameCount();
  }
}

const UvRegion& SpriteSheet::Playback::currentUv() const { return anim_->frameAt(currentFrame_); }

SpriteSheet::SpriteSheet(const AtlasRegion& region, glm::ivec2 frameSize, Orientation orientation,
                         uint32_t row) {
  glm::ivec2 regionSize = region.size();

  uint32_t count = orientation == Orientation::Horizontal ? regionSize.x / frameSize.x
                                                          : regionSize.y / frameSize.y;

  frames_.reserve(count);

  for (uint32_t i = 0; i < count; ++i) {
    glm::ivec2 offset;

    if (orientation == Orientation::Horizontal) {
      offset = {
          i * frameSize.x,
          row * frameSize.y,
      };
    } else {
      offset = {
          row * frameSize.x,
          i * frameSize.y,
      };
    }

    frames_.push_back(region.toUv(offset, frameSize));
  }
}

}  // namespace gfx