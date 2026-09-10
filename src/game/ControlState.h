#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace gm {

struct AxisInput {
  bool neg = false;
  bool pos = false;

  void setNeg(bool v) { neg = v; }
  void setPos(bool v) { pos = v; }

  int value() const { return static_cast<int>(pos) - static_cast<int>(neg); }

  void clear() { neg = pos = false; }
};

struct ControlState {
  glm::ivec3 move{0};

  bool jump = false;
  bool sneak = false;
  bool sprint = false;

  bool breakBlock = false;
  bool interact = false;
  bool drop = false;

  uint32_t hotbarSlot = 0;

  void setMoveLeft(bool v) {
    moveX_.setNeg(v);
    move.x = moveX_.value();
  }
  void setMoveRight(bool v) {
    moveX_.setPos(v);
    move.x = moveX_.value();
  }
  void setMoveDown(bool v) {
    moveY_.setNeg(v);
    move.y = moveY_.value();
  }
  void setMoveUp(bool v) {
    moveY_.setPos(v);
    move.y = moveY_.value();
  }
  void setMoveBack(bool v) {
    moveZ_.setNeg(v);
    move.z = moveZ_.value();
  }
  void setMoveForward(bool v) {
    moveZ_.setPos(v);
    move.z = moveZ_.value();
  }

  void setJump(bool v) { jump = v; }
  void setSneak(bool v) { sneak = v; }
  void setSprint(bool v) { sprint = v; }

  void setBreak(bool v) { breakBlock = v; }
  void setInteract(bool v) { interact = v; }
  void setDrop(bool v) { drop = v; }

  void setHotbarSlot(uint32_t slot) { hotbarSlot = slot; }

  void clear() {
    moveX_.clear();
    moveY_.clear();
    moveZ_.clear();
    move = {0, 0, 0};
    jump = false;
    sneak = false;
    sprint = false;
    breakBlock = false;
    interact = false;
    drop = false;
  }

  void Update() {
    breakBlock = false;
    interact = false;
  }

 private:
  AxisInput moveX_;
  AxisInput moveY_;
  AxisInput moveZ_;
};

}  // namespace gm