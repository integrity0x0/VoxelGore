#include "PlayerController.h"

#ifndef __ANDROID__
#include <GLFW/glfw3.h>
#include "core/Window.h"
#endif

#include <glm/gtc/matrix_transform.hpp>


#include "gfx/render/particle/ParticleEngine.h"
#include "WorldSession.h"
#include "game/entity/BleedComponent.h"
#include "game/entity/HealthComponent.h"
#include "game/entity/PlayerComponent.h"

namespace gm {

PlayerController::PlayerController(WorldSession& session, const glm::vec3& spawnPos)
    : session_(&session) {
  playerEntity_ = session.GetWorld().CreateEntity();
  session.components().Storage<HitboxComponent>().Add(playerEntity_.id,
                                                      HitboxComponent{
                                                          .pos = kSpawnpoint,
                                                          .size = glm::vec3(0.6f, 1.8f, 0.6f),
                                                          .vel = glm::vec3(0.0f),
                                                      });

  session.components().Storage<HealthComponent>().Add(playerEntity_.id, HealthComponent{});
  session.components().Storage<PlayerComponent>().Add(playerEntity_.id,
                                                      PlayerComponent(kSpawnpoint));
  camera_.LookAt(spawnPos, spawnPos + glm::vec3(1, 0, 0));
}

HitboxComponent& PlayerController::GetHitbox() {
  return *session_->components().Storage<HitboxComponent>().Get(playerEntity_.id);
}

const HitboxComponent& PlayerController::GetHitbox() const {
  return *session_->components().Storage<HitboxComponent>().Get(playerEntity_.id);
}

#ifndef __ANDROID__
void PlayerController::HandleInput(core::Window& window, ControlState& control,
                                   bool& cursorLocked) {
  auto& input = window.input();
  const auto& state = input.GetState();

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
    if (state.Pressed(GLFW_KEY_1 + i)) {
      control.hotbarSlot = i + 1;
    }
  }

  if (state.Pressed(GLFW_KEY_ESCAPE)) {
    cursorLocked = !cursorLocked;
    glfwSetInputMode(window.window(), GLFW_CURSOR,
                     cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
  }

  if (cursorLocked) {
    camera_.Rotate(state.GetCursorDeltaX(), state.GetCursorDeltaY());
  }
}
#endif

void PlayerController::UpdateMovement(ControlState& control, float /*dt*/) {
  auto& hb = GetHitbox();

  glm::vec3 flatForward(cosf(glm::radians(camera_.GetYaw())), 0.0f, sinf(glm::radians(camera_.GetYaw())));
  glm::vec3 flatRight = glm::normalize(glm::cross(flatForward, camera_.GetUp()));
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
  auto& hb = GetHitbox();
  cameraShake_.Update(dt, hb.vel * glm::vec3(1, 0, 1), hb.grounded);

  const glm::vec3 off = cameraShake_.GetPositionOffset();
  const glm::vec3& fwd = camera_.GetForward();
  const glm::vec3& right = camera_.GetRight();
  const glm::vec3 up = glm::cross(right, fwd);

  const glm::vec3 shakeWorld = right * off.x + up * off.y + fwd * off.z;

  camera_.SetPos(hb.pos + glm::vec3(0.0f, kEyeHeight * 0.5f, 0.0f) + shakeWorld);
  camera_.SetRoll(cameraShake_.GetRoll());
}

void PlayerController::TryBreak(WorldSession& session, gfx::ParticleEngine* particles) {
  auto hit =
      session.collision().Raycast(camera_.GetPos(), camera_.GetForward(), session.components(),
                                         kReachDistance);
  if (!hit) return;

  if (hit->entity.id != kInvalidEntityId) {
    auto* hb = session.components().Storage<HitboxComponent>().Get(hit->entity.id);
    auto* hl = session.components().Storage<HealthComponent>().Get(hit->entity.id);
    auto* bleed = session.components().Storage<BleedComponent>().Get(hit->entity.id);
    if (!hb) return;
    if (hl) hl->Damage(10.0f);

    if (bleed) {
      bleed->damage = 10.0f;
      bleed->hurted = true;
      bleed->pos = hit->pos;
      bleed->normal = hit->normal;
    }

    glm::vec3 dir = hb->pos - GetHitbox().pos;
    dir.y = 0.0f;
    if (glm::length2(dir) < 0.0001f) {
      dir = camera_.GetForward();
      dir.y = 0.0f;
    }
    dir = glm::normalize(dir);
    constexpr float kKnockback = 10.0f;
    hb->vel.x = dir.x * kKnockback;
    hb->vel.z = dir.z * kKnockback;
    hb->vel.y = glm::max(hb->vel.y, 1.8f);
    return;
  }

  auto voxel = session.GetWorld().GetChunks().getVoxel(hit->ipos);
  if (!voxel) return;

  if (particles) {
    particles->SpawnBlockDebris(voxel->id, glm::vec3(hit->ipos) + glm::vec3(0.5f));
    if (voxel->id == 8) particles->SpawnExplosion(hit->pos);
  }

  session.SetVoxel(hit->ipos, 0);
}

void PlayerController::TryPlace(WorldSession& session, uint32_t blockId) {
  auto hit =
      session.collision().Raycast(camera_.GetPos(), camera_.GetForward(), session.components(),
                                         kReachDistance);
  if (!hit) return;

  const glm::ivec3 placePos = hit->ipos + glm::ivec3(hit->normal);
  if (!session.collision().CanPlaceBlock(placePos, session.components())) return;

  session.SetVoxel(placePos, static_cast<uint16_t>(blockId));
}

void PlayerController::UpdateDamage(float dt) {
  auto& hb = GetHitbox();
  auto* hl = session_->components().Storage<HealthComponent>().Get(playerEntity_.id);
  if (!hl) return;

  constexpr glm::vec3 kDown{0.0f, -1.0f, 0.0f};

  if (hb.pos.y < 0.0f) {
    constexpr float kVoidDps = 25.0f;
    hl->Damage(kVoidDps * dt);
  }

  if (!hb.grounded) {
    if (wasGrounded_) {
      fallStartY_ = hb.pos.y;
    }
  } else if (!wasGrounded_) {
    const float fallDistance = fallStartY_ - hb.pos.y;
    if (fallDistance > kSafeFall) {
      const float damage = (fallDistance - kSafeFall) * kDamagePerBlock;
      hl->Damage(damage);

      float traumaAmount = glm::clamp(damage / hl->max * 1.5f, 0.1f, 0.7f);
      cameraShake_.Impulse(kDown, 2.0f + damage * 0.05f, traumaAmount);
    } else if (fallDistance > 1.5f) {
      cameraShake_.Impulse(kDown, fallDistance * 0.4f, 0.0f);
    }
  }

  wasGrounded_ = hb.grounded;
}

}  // namespace gm