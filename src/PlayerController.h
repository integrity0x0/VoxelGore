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

  PlayerController(WorldSession& session, const glm::vec3& spawnPos);

  void HandleInput(core::Window& window, ControlState& control, bool& cursorLocked);
  void UpdateMovement(ControlState& control, float dt);
  void UpdateCamera(float dt);

  void TryBreak(WorldSession& session, gfx::ParticleEngine* particles);
  void TryPlace(WorldSession& session, uint32_t blockId);

  core::Camera& camera() { return camera_; }
  const core::Camera& camera() const { return camera_; }

  Entity player() const { return playerEntity_; }
  HitboxComponent& hitbox();
  const HitboxComponent& hitbox() const;

  glm::vec3 forward() const;

 private:
  WorldSession* session_;
  Entity playerEntity_;
  core::Camera camera_;
  core::CameraShake cameraShake_;
};

}  // namespace gm