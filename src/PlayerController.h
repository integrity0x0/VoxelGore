#pragma once

#include <glm/glm.hpp>

#include "core/Camera.h"
#include "core/CameraShake.h"
#include "game/ControlState.h"
#include "WorldSession.h"
#include "game/entity/Entity.h"
#include "RenderWorld.h"

namespace core {
class Window;
}

namespace gm {

class PlayerController {
 public:
  static constexpr float kEyeHeight = 1.7f;
  static constexpr float kJumpVelocity = 7.953f;
  static constexpr float kReachDistance = 6.0f;
  static constexpr float kMoveSpeed = 6.0f;
  static constexpr glm::vec3 kSpawnpoint = glm::vec3(16.0f, 16.0f, 16.0f);
  PlayerController(WorldSession& session, const glm::vec3& spawnPos);

#ifndef __ANDROID__
  void HandleInput(core::Window& window, ControlState& control, bool& cursorLocked);
#endif
  void UpdateMovement(ControlState& control, float dt);
  void UpdateCamera(float dt);
  void UpdateDamage(float dt);

  void TryBreak(WorldSession& session, gfx::ParticleEngine* particles);
  void TryPlace(WorldSession& session, uint32_t blockId);

  core::Camera& GetCamera() { return camera_; }
  const core::Camera& GetCamera() const { return camera_; }

  Entity GetPlayer() const { return playerEntity_; }
  HitboxComponent& GetHitbox();
  const HitboxComponent& GetHitbox() const;

 private:
  static constexpr float kSafeFall = 3.5f;
  static constexpr float kDamagePerBlock = 5.0f;

  WorldSession* session_;
  Entity playerEntity_;
  core::Camera camera_;
  core::CameraShake cameraShake_;

  bool inVoid_ = false;
  bool wasGrounded_ = true;
  float fallStartY_ = 0.0f;
  float voidTimer_ = 0.f;
};

}  // namespace gm