#pragma once

#include <cstdint>
#include <vector>

#include "../../UvRegion.h"
#include "../../common/texture/SpriteSheet.h"

namespace gfx {

class BlockAnimation {
 public:
  BlockAnimation(SpriteSheet&& spriteSheet, float frameDurationSec);

  void Start();
  void Stop();
  [[nodiscard]] bool IsPlaying() const { return playing_; }

  [[nodiscard]] const UvRegion& Update(float dt);

  void SetCurrentFrame(uint32_t frameIndex);
  [[nodiscard]] uint32_t GetCurrentFrame() const { return currentFrame_; }

  void SetTime(float timeSec);
  float GetTime() const { return accumulator_; }

  [[nodiscard]] const UvRegion& GetCurrentRegion() const { return spriteSheet_.frameAt(currentFrame_); }

  void SetFrameDuration(float frameDurationSec) { frameDuration_ = frameDurationSec; }

  [[nodiscard]] float GetFrameDuration() const { return frameDuration_; }

 private:
  SpriteSheet spriteSheet_;
  float frameDuration_;
  float accumulator_ = 0.0f;
  uint32_t currentFrame_ = 0;
  bool playing_ = true;
};

}  // namespace gfx