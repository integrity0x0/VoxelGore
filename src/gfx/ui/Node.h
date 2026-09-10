#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Renderer.h"

namespace gfx::ui {

enum class AnchorX : uint8_t { Left, Center, Right };

enum class AnchorY : uint8_t { Top, Center, Bottom };

enum class Unit : uint8_t { Px, Percent };

class Node {
 public:
  virtual ~Node() = default;

  // Geometry
  const glm::vec2& getPos() const { return pos_; }
  const glm::vec2& size() const { return size_; }

  Unit getPosUnitX() const { return posUnit_[0]; }
  Unit getPosUnitY() const { return posUnit_[1]; }
  Unit getSizeUnitX() const { return sizeUnit_[0]; }
  Unit getSizeUnitY() const { return sizeUnit_[1]; }

  AnchorX getAnchorX() const { return anchorX_; }
  AnchorY getAnchorY() const { return anchorY_; }

  void setPos(const glm::vec2& pos);
  void setSize(const glm::vec2& size);

  void setPosX(float v, Unit u = Unit::Px);
  void setPosY(float v, Unit u = Unit::Px);
  void setWidth(float v, Unit u = Unit::Px);
  void setHeight(float v, Unit u = Unit::Px);

  void setPosUnitX(Unit u);
  void setPosUnitY(Unit u);
  void setSizeUnitX(Unit u);
  void setSizeUnitY(Unit u);

  void setAnchorX(AnchorX x) {
    anchorX_ = x;
    relayout();
  }
  void setAnchorY(AnchorY y) {
    anchorY_ = y;
    relayout();
  }
  void setAnchor(AnchorX x, AnchorY y) {
    anchorX_ = x;
    anchorY_ = y;
    relayout();
  }

  const glm::vec2& getComputedPos() const { return computedPos_; }
  const glm::vec2& getComputedSize() const { return computedSize_; }
  const glm::vec2& getAbsolutePos() const { return absPos_; }

  // Colors & spacing
  const glm::vec4& getColor() const { return color_; }
  const std::optional<glm::vec4>& getPadding() const { return padding_; }
  const glm::vec4& getMargin() const { return margin_; }

  void setColor(const glm::vec4& color) { color_ = color; }
  void setPadding(const std::optional<glm::vec4>& padding);
  void setBgColor(const glm::vec4& bgColor) { bgColor_ = bgColor; }
  void setMargin(const glm::vec4& margin) { margin_ = margin; }

  const std::optional<glm::vec4>& getHeldColor() const { return heldColor_; }
  void setHeldColor(std::optional<glm::vec4> color) { heldColor_ = std::move(color); }

  float getHeldScale() const { return heldScale_; }
  void setHeldScale(float scale) { heldScale_ = scale; }

  // Textures
  const std::optional<TextureRegion>& getBgImage() const { return bgImage_; }
  const std::optional<TextureRegion>& getImage() const { return image_; }

  void setBgImage(std::optional<TextureRegion> region);
  void setImage(std::optional<TextureRegion> region);

  void clearBgImage() { bgImage_.reset(); }
  void clearImage() { image_.reset(); }

  const std::string& getBgImageSrc() const;
  const std::string& getImageSrc() const;

  // Hierarchy
  virtual const std::vector<std::shared_ptr<Node>>& getChildren() const {
    static const std::vector<std::shared_ptr<Node>> empty{};
    return empty;
  }

  // Hit-test & input
  bool contains(float px, float py) const {
    return px >= absPos_.x && px <= absPos_.x + computedSize_.x && py >= absPos_.y &&
           py <= absPos_.y + computedSize_.y;
  }

  bool isPressed() const { return pressed_; }
  bool isVisible() const { return visible_; }
  bool isInteractive() const { return interactive_; }

  void setVisible(bool visible) { visible_ = visible; }
  void setInteractive(bool interactive) { interactive_ = interactive; }

  void press() {
    if (!pressed_) {
      pressed_ = true;
      justPressed_ = true;
    }
  }

  void release() {
    if (pressed_) {
      pressed_ = false;
      justReleased_ = true;
    }
  }

  // Rendering
  virtual void renderBg(Renderer& renderer) const;
  virtual void render(Renderer& renderer) const;

  // Lua callbacks
  const std::string& getOnPressLua() const { return onPressLua_; }
  const std::string& getOnHeldLua() const { return onHeldLua_; }
  const std::string& getOnReleaseLua() const { return onReleaseLua_; }

  void setOnPress(std::string luaFn) { onPressLua_ = std::move(luaFn); }
  void setOnHeld(std::string luaFn) { onHeldLua_ = std::move(luaFn); }
  void setOnRelease(std::string luaFn) { onReleaseLua_ = std::move(luaFn); }

  bool consumeJustPressed() {
    bool v = justPressed_;
    justPressed_ = false;
    return v;
  }

  bool consumeJustReleased() {
    bool v = justReleased_;
    justReleased_ = false;
    return v;
  }

  // Layout
  virtual void layout(const glm::vec2& parentAbsPos, const glm::vec2& parentSize);

 protected:
  glm::vec4 getEffectiveColor() const { return (pressed_ && heldColor_) ? *heldColor_ : color_; }

  void relayout();

 private:
  static float anchorFactorX(AnchorX a) {
    switch (a) {
      case AnchorX::Left:
        return 0.0f;
      case AnchorX::Center:
        return 0.5f;
      case AnchorX::Right:
        return 1.0f;
    }
    return 0.0f;
  }

  static float anchorFactorY(AnchorY a) {
    switch (a) {
      case AnchorY::Top:
        return 0.0f;
      case AnchorY::Center:
        return 0.5f;
      case AnchorY::Bottom:
        return 1.0f;
    }
    return 0.0f;
  }

  struct Insets {
    float left, top, right, bottom;
  };
  Insets padding() const {
    const glm::vec4 p = padding_.value_or(glm::vec4(0.0f));
    return {p.w, p.x, p.y, p.z};
  }

  struct Rect {
    glm::vec2 pos;
    glm::vec2 size;
  };

  Rect contentRect() const {
    const Insets in = padding();
    return {absPos_ + glm::vec2(in.left, in.top),
            computedSize_ - glm::vec2(in.left + in.right, in.top + in.bottom)};
  }

  Rect applyHeldScale(const Rect& r) const {
    if (!pressed_ || heldScale_ == 1.0f) return r;
    const glm::vec2 scaledSize = r.size * heldScale_;
    const glm::vec2 scaledPos = r.pos + (r.size - scaledSize) * 0.5f;
    return {scaledPos, scaledSize};
  }

  // Geometry
  glm::vec2 pos_{0.0f};
  glm::vec2 size_{0.0f};
  std::array<Unit, 2> posUnit_{Unit::Px, Unit::Px};
  std::array<Unit, 2> sizeUnit_{Unit::Px, Unit::Px};
  AnchorX anchorX_{AnchorX::Left};
  AnchorY anchorY_{AnchorY::Top};

  glm::vec2 computedPos_{0.0f};
  glm::vec2 computedSize_{0.0f};
  glm::vec2 absPos_{0.0f};

  glm::vec2 lastParentAbsPos_{0.0f};
  glm::vec2 lastParentSize_{0.0f};

  // Visual
  glm::vec4 color_{1.0f};
  std::optional<glm::vec4> padding_;
  glm::vec4 bgColor_{1.0f};
  glm::vec4 margin_{0.0f};

  std::optional<glm::vec4> heldColor_;
  float heldScale_ = 1.0f;

  std::optional<TextureRegion> bgImage_;
  std::optional<TextureRegion> image_;

  // Input state
  bool pressed_ = false;
  bool justPressed_ = false;
  bool justReleased_ = false;
  bool visible_ = true;
  bool interactive_ = true;

  // Lua callbacks
  std::string onPressLua_;
  std::string onHeldLua_;
  std::string onReleaseLua_;
};

}  // namespace gfx::ui