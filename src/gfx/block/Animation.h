#pragma once

#include <cstdint>
#include <vector>

#include "../UvRegion.h"
#include "../texture/SpriteSheet.h"

namespace gfx::block {

class Animation {
 public:
  Animation(SpriteSheet&& spriteSheet, float frameDurationSec);

  void start();
  void stop();
  bool isPlaying() const { return playing_; }

  const UvRegion& Update(float dt);

  void setCurrentFrame(uint32_t frameIndex);
  uint32_t currentFrame() const { return currentFrame_; }

  void setTime(float timeSec);
  float time() const { return accumulator_; }

  const UvRegion& getCurrentRegion() const { return spriteSheet_.frameAt(currentFrame_); }

  void setFrameDuration(float frameDurationSec) { frameDuration_ = frameDurationSec; }

  float frameDuration() const { return frameDuration_; }

 private:
  SpriteSheet spriteSheet_;
  float frameDuration_;
  float accumulator_ = 0.0f;
  uint32_t currentFrame_ = 0u;
  bool playing_ = true;
};

}  // namespace gfx::block