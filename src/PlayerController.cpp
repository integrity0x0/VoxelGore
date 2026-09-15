#include "PlayerController.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "core/Window.h"
#include "gfx/render/particle/ParticleEngine.h"
#include "WorldSession.h"


namespace gm {

PlayerController::PlayerController(WorldSession& session, const glm::vec3& spawnPos)
    : session_(&session) {
  playerEntity_ = session.world().CreateEntity();
  session.components().Storage<HitboxComponent>().Add(playerEntity_.id,
                                                      HitboxComponent{
                                                          .pos = spawnPos,
                                                          .size = glm::vec3(0.6f, 1.8f, 0.6f),
                                                          .vel = glm::vec3(0.0f),
                                                      });
  camera_.LookAt(spawnPos, spawnPos + glm::vec3(1, 0, 0));
}

HitboxComponent& PlayerController::hitbox() {
  return *session_->components().Storage<HitboxComponent>().Get(playerEntity_.id);
}

const HitboxComponent& PlayerController::hitbox() const {
  return *session_->components().Storage<HitboxComponent>().Get(playerEntity_.id);
}

glm::vec3 PlayerController::forward() const {
  const float yaw = glm::radians(camera_.yaw);
  const float pitch = glm::radians(camera_.pitch);
  return glm::normalize(glm::vec3(cosf(pitch) * cosf(yaw), sinf(pitch), cosf(pitch) * sinf(yaw)));
}

void PlayerController::HandleInput(core::Window& window, ControlState& control,
                                   bool& cursorLocked) {
  auto& input = window.getInput();
  const auto& state = input.getState();

  control.move.z = 0;
  control.move.x = 0;
  if (state.Down(GLFW_KEY_W)) control.move.z = 1;
  if (state.Down(GLFW_KEY_S)) control.move.z = -1;
  if (state.Down(GLFW_KEY_A)) control.move.x = -1;
  if (state.Down(GLFW_KEY_D)) control.move.x = 1;

  control.jump = state.Down(GLFW_KEY_SPACE);
  control.breakBlock = state.MousePressed(GLFW_MOUSE_BUTTON_LEFT);
  control.interact = state.MousePressed(GLFW_MOUSE_BUTTON_RIGHT);

  for (int i = 0; i < 8; ++i) {
    if (state.pressed(GLFW_KEY_1 + i)) {
      control.hotbarSlot = i + 1;
    }
  }

  if (state.pressed(GLFW_KEY_ESCAPE)) {
    cursorLocked = !cursorLocked;
    glfwSetInputMode(window.getWindow(), GLFW_CURSOR,
                     cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
  }

  if (cursorLocked) {
    camera_.Rotate(state.cursorDeltaX(), state.cursorDeltaY());
  }
}

void PlayerController::UpdateMovement(ControlState& control, float /*dt*/) {
  auto& hb = hitbox();

  glm::vec3 flatForward(cosf(glm::radians(camera_.yaw)), 0.0f, sinf(glm::radians(camera_.yaw)));
  glm::vec3 flatRight = glm::normalize(glm::cross(flatForward, camera_.up));
  glm::vec3 moveDir = flatForward * static_cast<float>(control.move.z) +
                      flatRight * static_cast<float>(control.move.x);

  const float len2 = glm::length2(moveDir);
  if (len2 > 0.0001f) {
    moveDir /= glm::sqrt(len2);
    hb.vel.x = moveDir.x * kMoveSpeed;
    hb.vel.z = moveDir.z * kMoveSpeed;
  }

  if (control.jump && hb.grounded) {
    hb.vel.y = kJumpVelocity;
  }
}

void PlayerController::UpdateCamera(float dt) {
  auto& hb = hitbox();
  cameraShake_.Update(dt, hb.vel * glm::vec3(1, 0, 1), hb.grounded);
  camera_.position =
      hb.pos + glm::vec3(0.0f, kEyeHeight, 0.0f) * 0.5f + cameraShake_.getPositionOffset();
}

void PlayerController::TryBreak(WorldSession& session, gfx::ParticleEngine* particles) {
  auto hit = session.collision().Raycast(camera_.position, forward(), session.components(),
                                         kReachDistance);
  if (!hit) return;

  if (hit->entity.id != kInvalidEntityId) {
    auto* hb = session.components().Storage<HitboxComponent>().Get(hit->entity.id);
    auto* hl = session.components().Storage<HealthComponent>().Get(hit->entity.id);
    if (!hb) return;
    if (hl) hl->Damage(10.0f);

    glm::vec3 dir = hb->pos - hitbox().pos;
    dir.y = 0.0f;
    if (glm::length2(dir) < 0.0001f) {
      dir = forward();
      dir.y = 0.0f;
    }
    dir = glm::normalize(dir);
    constexpr float kKnockback = 10.0f;
    hb->vel.x = dir.x * kKnockback;
    hb->vel.z = dir.z * kKnockback;
    hb->vel.y = glm::max(hb->vel.y, 1.8f);
    return;
  }

  auto voxel = session.world().chunks().getVoxel(hit->ipos);
  if (!voxel) return;

  if (particles) {
    particles->SpawnBlockDebris(voxel->id, glm::vec3(hit->ipos) + glm::vec3(0.5f));
    if (voxel->id == 8) particles->SpawnExplosion(hit->pos);
  }

  session.SetVoxel(hit->ipos, 0);
}

void PlayerController::TryPlace(WorldSession& session, uint32_t blockId) {
  auto hit = session.collision().Raycast(camera_.position, forward(), session.components(),
                                         kReachDistance);
  if (!hit) return;

  const glm::ivec3 placePos = hit->ipos + glm::ivec3(hit->normal);
  if (!session.collision().CanPlaceBlock(placePos, session.components())) return;

  session.SetVoxel(placePos, static_cast<uint16_t>(blockId));
}

}  // namespace gm