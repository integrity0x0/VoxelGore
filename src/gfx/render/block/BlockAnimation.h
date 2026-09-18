#pragma once

#include <cstdint>
#include <vector>

#include "../../UvRegion.h"
#include "../../common/texture/SpriteSheet.h"

namespace gfx {

class BlockAnimation {
 public:
  BlockAnimation(SpriteSheet&& spriteSheet, float frameDurationSec);

  void start();
  void stop();
  [[nodiscard]] bool IsPlaying() const { return playing_; }

  [[nodiscard]] const UvRegion& Update(float dt);

  void setCurrentFrame(uint32_t frameIndex);
  [[nodiscard]] uint32_t currentFrame() const { return currentFrame_; }

  void setTime(float timeSec);
  float time() const { return accumulator_; }

  [[nodiscard]] const UvRegion& getCurrentRegion() const { return spriteSheet_.frameAt(currentFrame_); }

  void setFrameDuration(float frameDurationSec) { frameDuration_ = frameDurationSec; }

  [[nodiscard]] float frameDuration() const { return frameDuration_; }

 private:
  SpriteSheet spriteSheet_;
  float frameDuration_;
  float accumulator_ = 0.0f;
  uint32_t currentFrame_ = 0;
  bool playing_ = true;
};

}  // namespace gfx