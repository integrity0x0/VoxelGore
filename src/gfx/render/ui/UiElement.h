#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "../../UvRegion.h"
#include "../../common/texture/MaterialManager.h"
#include "UiRenderer.h"

namespace gfx {

enum class UiElementType : uint8_t { Undefined, Container, Text, Image, Button, Checkbox, Slider };

class UiElement {
 public:
  using Callbacks = std::array<UiCallback, static_cast<size_t>(UiEventType::Count)>;

  UiElement(std::string_view id, const UiElement* parent);

  virtual ~UiElement() = default;

  virtual void Render(UiRenderer& renderer);
  [[nodiscard]] virtual UiElementType GetNodeType() const { return UiElementType::Undefined; }

  template <typename T>
  T* GetAs(UiElementType type) {
    return type == GetNodeType() ? static_cast<T*>(this) : nullptr;
  }

  // Identity
  [[nodiscard]] std::string_view GetId() const { return id_; }
  [[nodiscard]] const UiElement* GetParent() const { return parent_; }

  // Transform
  [[nodiscard]] const UiLength2& GetPos() const { return pos_; }
  void SetPos(const UiLength2& position) {
    pos_ = position;
    MarkLayoutDirty();
  }

  [[nodiscard]] const UiLength2& GetSize() const { return size_; }
  void SetSize(const UiLength2& size) {
    size_ = size;
    MarkLayoutDirty();
  }

  [[nodiscard]] const UiPoint& GetAnchor() const { return anchor_; }
  void SetAnchor(UiPoint anchor) {
    anchor_ = anchor;
    MarkLayoutDirty();
  }

  [[nodiscard]] const UiPoint& GetPivot() const { return pivot_; }
  void SetPivot(UiPoint pivot) {
    pivot_ = pivot;
    MarkLayoutDirty();
  }

  [[nodiscard]] const std::optional<UiPadding>& GetPadding() const { return padding_; }
  void SetPadding(std::optional<UiPadding> padding) {
    padding_ = std::move(padding);
    MarkLayoutDirty();
  }

  // Appearance 
  [[nodiscard]] const glm::vec4& GetColor() const { return color_; }
  void SetColor(const glm::vec4& color) { color_ = color; }

  [[nodiscard]] const glm::vec4& GetBgColor() const { return bgColor_; }
  void SetBgColor(const glm::vec4& color) { bgColor_ = color; }

  [[nodiscard]] const std::optional<UiTextureRegion>& GetBgImage() const { return bgImage_; }
  void SetBgImage(std::optional<UiTextureRegion> image) { bgImage_ = std::move(image); }

  [[nodiscard]] const std::optional<float>& GetBgRadius() const { return bgRadius_; }
  void SetBgRadius(const std::optional<float>& bgRadius) { bgRadius_ = bgRadius; }

  [[nodiscard]] const std::optional<float>& GetBorderWidth() const {
    return borderWidth_;
  }
  
  void SetBorderWidth(const std::optional<float>& borderWidth) {
    borderWidth_ = borderWidth;
  }

  [[nodiscard]] const std::optional<UiTextureRegion>& GetBorderImage() {
    return borderImage_;
  }

  void SetBorderImage(const std::optional<UiTextureRegion>& borderImage) {
    borderImage_ = borderImage;
  }

  [[nodiscard]] const glm::vec4& GetBorderColor() {
    return borderColor_;
  }

  void SetBorderColor(const glm::vec4& borderColor) {
    borderColor_ = borderColor;
  }

  [[nodiscard]] float GetRadius() const { return radius_; }
  void SetRadius(float radius) { radius_ = radius; }

  // Properties
  [[nodiscard]] bool IsVisible() const { return visible_; }
  void SetVisible(bool visible) { visible_ = visible; }

  [[nodiscard]] UiStateFlags GetStateFlags() const { return stateFlags_; }
  [[nodiscard]] UiStateFlags& GetStateFlagsRef() { return stateFlags_; }

  [[nodiscard]] bool IsState(UiStateFlags flags) const {
    return static_cast<bool>(stateFlags_ & flags);
  }

  void SetStateFlags(UiStateFlags flags) { stateFlags_ = flags; }

  // Callbacks
  [[nodiscard]] const Callbacks& GetCallbacks() const { return callbacks_; }
  [[nodiscard]] Callbacks& GetCallbacks() { return callbacks_; }

  [[nodiscard]] UiCallback& GetCallback(UiEventType type) {
    assert(type < UiEventType::Count);
    return callbacks_[static_cast<size_t>(type)];
  }

  void SetCallback(UiEventType type, std::string functionName) {
    assert(type < UiEventType::Count);
    UiCallback& callback = GetCallback(type);
    callback.functionName = std::move(functionName);
    callback.ref.reset();
  }

  [[nodiscard]] UiStateOverrides& GetStateOverrides() { return stateOverrides_; }

  [[nodiscard]] const UiStateOverrides& GetStateOverrides() const { return stateOverrides_; }

  [[nodiscard]] UiVisualProps& GetStateOverride(UiState state) {
    return stateOverrides_[static_cast<size_t>(state)];
  }

  [[nodiscard]] const UiVisualProps& GetStateOverride(UiState state) const {
    return stateOverrides_[static_cast<size_t>(state)];
  }

  void SetStateOverride(UiState state, const UiVisualProps& props) {
    stateOverrides_[static_cast<size_t>(state)] = props;
  }

  void SetStateOverride(UiState state, UiVisualProps props) {
    stateOverrides_[static_cast<size_t>(state)] = std::move(props);
  }

  [[nodiscard]] virtual std::span<const std::unique_ptr<UiElement>> GetChildren() const {
    return {};
  }

  [[nodiscard]] bool Contains(glm::vec2 pos) const {
    const UiRect& rect = resolvedLayout_.rect;
    return pos.x >= rect.pos.x && pos.x <= rect.pos.x + rect.size.x && pos.y >= rect.pos.y &&
           pos.y <= rect.pos.y + rect.size.y;
  }

  // Layout
  [[nodiscard]] const UiLayout& GetResolvedLayout() const { return resolvedLayout_; }

  void UpdateLayout(const UiRect& parentRect);

  virtual void Relayout(const UiRect& rect);

 protected:
  void MarkLayoutDirty() {
    layoutDirty_ = true;
    for (const UiElement* p = parent_; p && !p->layoutDirty_ && !p->childDirty_; p = p->parent_) {
      p->childDirty_ = true;
    }
  }

  virtual void RenderBg(UiRenderer& renderer);
  virtual void RenderBorder(UiRenderer& renderer);

 protected:
  static constexpr std::array<float, static_cast<size_t>(UiAlignX::Count)> kAlignmentFactors{
      0.0f, 0.5f, 1.0f};
  static_assert(static_cast<size_t>(UiAlignX::Count) == static_cast<size_t>(UiAlignY::Count));

  // Identity / hierarchy
  std::string id_;
  const UiElement* parent_ = nullptr;

  // Transform
  UiLength2 pos_;
  UiLength2 size_;
  UiPoint anchor_;
  UiPoint pivot_;
  std::optional<UiPadding> padding_;

  // Appearance
  glm::vec4 color_ = glm::vec4(1.0f);
  glm::vec4 bgColor_ = glm::vec4(1.0f);
  std::optional<UiTextureRegion> bgImage_;
  float radius_ = 0.0f;
  std::optional<float> bgRadius_;

  std::optional<float> borderWidth_;
  glm::vec4 borderColor_ = glm::vec4(1.0f);
  std::optional<UiTextureRegion> borderImage_;

  // Properties
  bool visible_ = true;
  UiStateFlags stateFlags_ = {};

  Callbacks callbacks_;

  UiStateOverrides stateOverrides_;

  mutable bool layoutDirty_ = false;
  mutable bool childDirty_ = false;
  mutable UiLayout resolvedLayout_;
};

}  // namespace gfx