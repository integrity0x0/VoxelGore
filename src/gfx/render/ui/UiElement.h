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

enum class UiElementType : uint8_t { Undefined, Container, Text, Image, Button, Bar, Checkbox, Slider, Stick };

class UiElement {
 public:
  using Callbacks = std::array<UiCallback, static_cast<size_t>(UiEventType::Count)>;

  UiElement(std::string_view id, const UiElement* parent) : id_(id), parent_(parent) {}

  virtual ~UiElement() = default;

  virtual void Render(UiRenderer& renderer);
  [[nodiscard]] virtual UiElementType GetNodeType() const = 0;

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

  [[nodiscard]] glm::vec4 GetColor() const {
    return baseVisual_.color.value_or(kVisualDefaults.color);
  }

  void SetColor(const glm::vec4& color) {
    baseVisual_.color = color;
    MarkVisualDirty();
  }

  [[nodiscard]] glm::vec4 GetBgColor() const {
    return baseVisual_.bgColor.value_or(kVisualDefaults.bgColor);
  }
  void SetBgColor(const glm::vec4& color) {
    baseVisual_.bgColor = color;
    MarkVisualDirty();
  }

  [[nodiscard]] const std::optional<UiTextureRegion>& GetBgImage() const {
    return baseVisual_.bgImage;
  }
  void SetBgImage(std::optional<UiTextureRegion> image) {
    baseVisual_.bgImage = std::move(image);
    MarkVisualDirty();
  }

  [[nodiscard]] const std::optional<float>& GetBgRadius() const { return baseVisual_.bgRadius; }
  void SetBgRadius(const std::optional<float>& bgRadius) {
    baseVisual_.bgRadius = bgRadius;
    MarkVisualDirty();
  }

  [[nodiscard]] const std::optional<float>& GetBorderWidth() const {
    return baseVisual_.borderWidth;
  }
  void SetBorderWidth(const std::optional<float>& borderWidth) {
    baseVisual_.borderWidth = borderWidth;
    MarkVisualDirty();
  }

  [[nodiscard]] const std::optional<UiTextureRegion>& GetBorderImage() const {
    return baseVisual_.borderImage;
  }
  void SetBorderImage(const std::optional<UiTextureRegion>& borderImage) {
    baseVisual_.borderImage = borderImage;
    MarkVisualDirty();
  }

  [[nodiscard]] glm::vec4 GetBorderColor() const {
    return baseVisual_.borderColor.value_or(kVisualDefaults.borderColor);
  }
  void SetBorderColor(const glm::vec4& borderColor) {
    baseVisual_.borderColor = borderColor;
    MarkVisualDirty();
  }

  [[nodiscard]] float GetRadius() const {
    return baseVisual_.radius.value_or(kVisualDefaults.radius);
  }
  void SetRadius(float radius) {
    baseVisual_.radius = radius;
    MarkVisualDirty();
  }

  [[nodiscard]] float GetScale() const { return baseVisual_.scale.value_or(kVisualDefaults.scale); }
  void SetScale(float scale) {
    baseVisual_.scale = scale;
    MarkVisualDirty();
  }

  [[nodiscard]] const UiVisualProps& GetBaseVisual() const { return baseVisual_; }
  void SetBaseVisual(UiVisualProps props) {
    baseVisual_ = std::move(props);
    MarkVisualDirty();
  }
  void MergeBaseVisual(const UiVisualProps& props) {
    baseVisual_.MergeFrom(props);
    MarkVisualDirty();
  }

  [[nodiscard]] const UiResolvedVisual& GetResolvedVisual() const {
    if (visualDirty_) UpdateVisual();
    return resolvedVisual_;
  }

  // Properties
  [[nodiscard]] bool IsVisible() const { return visible_; }
  void SetVisible(bool visible) { visible_ = visible; }

  [[nodiscard]] UiStateFlags GetVisibleOn() const { return visibleOn_; }
  void SetVisibleOn(UiStateFlags flags) { visibleOn_ = flags; }

  [[nodiscard]] bool IsEffectivelyVisible() const {
    if (!visible_) return false;
    if (!static_cast<bool>(visibleOn_)) return true;
    return !parent_ || parent_->IsState(visibleOn_);
  }

  [[nodiscard]] bool IsInteractive() const {
    return interactive_;
  }

  void SetIntercative(bool interactive) {
    interactive_ = interactive;
  }

  [[nodiscard]] bool IsEffectivelyInteractive() const {
    return IsEffectivelyVisible() && interactive_;
  }

  // State
  [[nodiscard]] UiStateFlags GetStateFlags() const { return stateFlags_; }

  [[nodiscard]] bool IsState(UiStateFlags flags) const {
    return static_cast<bool>(stateFlags_ & flags);
  }

  void SetStateFlags(UiStateFlags flags) {
    if (flags == stateFlags_) return;
    stateFlags_ = flags;
    MarkVisualDirty();
  }

  void AddStateFlags(UiStateFlags flags) { SetStateFlags(stateFlags_ | flags); }
  void RemoveStateFlags(UiStateFlags flags) { SetStateFlags(stateFlags_ & ~flags); }
  void SetStateFlag(UiStateFlags flag, bool on) {
    if (on) {
      AddStateFlags(flag);
    } else {
      RemoveStateFlags(flag);
    }
  }

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

  // State overrides
  [[nodiscard]] UiStateOverrides& GetStateOverrides() {
    MarkVisualDirty();
    return stateOverrides_;
  }

  [[nodiscard]] const UiStateOverrides& GetStateOverrides() const { return stateOverrides_; }

  [[nodiscard]] UiVisualProps& GetStateOverride(UiState state) {
    MarkVisualDirty();
    return stateOverrides_[static_cast<size_t>(state)];
  }

  [[nodiscard]] const UiVisualProps& GetStateOverride(UiState state) const {
    return stateOverrides_[static_cast<size_t>(state)];
  }

  void SetStateOverride(UiState state, UiVisualProps props) {
    stateOverrides_[static_cast<size_t>(state)] = std::move(props);
    MarkVisualDirty();
  }

  void ApplyStyle(const UiStyle& s);

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

  void MarkVisualDirty() const {
    visualDirty_ = true;
    for (const auto& child : GetChildren()) 
      child->MarkVisualDirty();
  }
  
  virtual void UpdateVisual() const;
  glm::vec2 GetPivotPoint() const; 
  virtual void RenderBg(UiRenderer& renderer);
  virtual void RenderContent(UiRenderer& renderer);
  virtual void RenderBorder(UiRenderer& renderer);

  static UiRect ScaleAround(glm::vec2 pos, glm::vec2 size, glm::vec2 origin, float scale);
 protected:
  static constexpr std::array<float, static_cast<size_t>(UiAlignX::Count)> kAlignmentFactors{
      0.0f, 0.5f, 1.0f};
  static_assert(static_cast<size_t>(UiAlignX::Count) == static_cast<size_t>(UiAlignY::Count));

  inline static const UiResolvedVisual kVisualDefaults{};

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
  UiVisualProps baseVisual_;

  // Properties
  bool interactive_ = true;
  bool visible_ = true;
  UiStateFlags visibleOn_ = {};
  UiStateFlags stateFlags_ = {};

  Callbacks callbacks_;

  UiStateOverrides stateOverrides_;

  mutable bool layoutDirty_ = false;
  mutable bool childDirty_ = false;
  mutable UiLayout resolvedLayout_;

  mutable bool visualDirty_ = true;
  mutable UiResolvedVisual resolvedVisual_ = {};
};

}  // namespace gfx