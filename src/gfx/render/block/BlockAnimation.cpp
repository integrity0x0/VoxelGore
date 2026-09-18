#include "BlockAnimation.h"

#include <cmath>
#include <stdexcept>

namespace gfx {

BlockAnimation::BlockAnimation(SpriteSheet&& spriteSheet, float frameDurationSec)
    : spriteSheet_(std::move(spriteSheet)), frameDuration_(frameDurationSec) {
  if (frameDuration_ <= 0.f) {
    throw std::runtime_error("BlockAnimation: frameDuration must be > 0");
  }
}

void BlockAnimation::start() { playing_ = true; }

void BlockAnimation::stop() { playing_ = false; }

const UvRegion& BlockAnimation::Update(float dt) {
  if (!playing_) {
    return getCurrentRegion();
  }

  accumulator_ += dt;

  while (accumulator_ >= frameDuration_) {
    accumulator_ -= frameDuration_;
    currentFrame_ = (currentFrame_ + 1) % static_cast<uint32_t>(spriteSheet_.frameCount());
  }

  return getCurrentRegion();
}

void BlockAnimation::setCurrentFrame(uint32_t frameIndex) {
  currentFrame_ = frameIndex % static_cast<uint32_t>(spriteSheet_.frameCount());
}

void BlockAnimation::setTime(float timeSec) {
  accumulator_ = std::fmod(timeSec, frameDuration_ * static_cast<float>(spriteSheet_.frameCount()));

  uint32_t framesElapsed = static_cast<uint32_t>(accumulator_ / frameDuration_);
  currentFrame_ = framesElapsed % static_cast<uint32_t>(spriteSheet_.frameCount());
  accumulator_ = accumulator_ - static_cast<float>(framesElapsed) * frameDuration_;
}

}  // namespace gfx