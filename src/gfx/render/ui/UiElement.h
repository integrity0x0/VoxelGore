#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <span>

#include "../../UvRegion.h"
#include "../../common/texture/MaterialManager.h"
#include "UiRenderer.h"

namespace gfx {

enum class UiNodeType : uint8_t { Undefined, Container, Text, Image, Button, Checkbox, Slider };

class UiElement {
 public:
  UiElement(std::string_view id, const UiElement* parent);
  virtual ~UiElement() = default;

  virtual void Render(UiRenderer& renderer);
  [[nodiscard]] virtual UiNodeType GetNodeType() const { return UiNodeType::Undefined; }

  // Identity
  [[nodiscard]] std::string_view GetId() const { return id_; }
  [[nodiscard]] const UiElement* GetParent() const { return parent_; }
  
  // Transform
  [[nodiscard]] const UiLength2& GetPos() const { return pos_; }
  void SetPos(const UiLength2& position) { pos_ = position; }

  [[nodiscard]] const UiLength2& GetSize() const { return size_; }
  void SetSize(const UiLength2& size) { size_ = size; }

  [[nodiscard]] const UiPoint& GetAnchor() const { return anchor_; }
  void SetAnchor(UiPoint anchor) { anchor_ = anchor; }

  [[nodiscard]] const UiPoint& GetPivot() const { return pivot_; }
  void SetPivot(UiPoint pivot) { pivot_ = pivot; }

  // Appearance
  [[nodiscard]] const glm::vec4& GetColor() const { return color_; }
  void SetColor(const glm::vec4& color) { color_ = color; }

  [[nodiscard]] const std::optional<UiTextureRegion>& GetImage() const { return image_; }

  void SetImage(std::optional<UiTextureRegion> image) { image_ = std::move(image); }

  [[nodiscard]] const glm::vec4& GetBgColor() const { return bgColor_; }
  void SetBgColor(const glm::vec4& color) { bgColor_ = color; }

  [[nodiscard]] const std::optional<UiTextureRegion>& GetBgImage() const { return bgImage_; }

  void SetBgImage(std::optional<UiTextureRegion> image) { bgImage_ = std::move(image); }

  [[nodiscard]] const std::optional<float>& GetBgRadius() const {
    return bgRadius_;
  }

  void SetBgRadius(const std::optional<float>& bgRadius) {
    bgRadius_ = bgRadius;
  }

  [[nodiscard]] float GetRadius() const { return radius_; }
  void SetRadius(float radius) { radius_ = radius; }

  [[nodiscard]] const std::optional<UiPadding>& GetPadding() const { return padding_; }

  void SetPadding(std::optional<UiPadding> padding) { padding_ = std::move(padding); }\

  // Properties
  [[nodiscard]] bool IsVisible() const { return visible_; }
  void SetVisible(bool visible) { visible_ = visible; }

  [[nodiscard]] virtual std::span<const std::unique_ptr<UiElement>> GetChildren() const {
    return {};
  }

  [[nodiscard]] bool IsHovered() const { return hovered_; }

  void ApplyHover(bool hovered) {
    prevHovered_ = hovered_;
    hovered_ = hovered;
  }

  [[nodiscard]] bool Contains(glm::vec2 pos) const {
    const UiRect& rect = resolvedLayout_.rect;
    return pos.x >= rect.pos.x && pos.x <= rect.pos.x + rect.size.x && pos.y >= rect.pos.y &&
           pos.y <= rect.pos.y + rect.size.y;
  }

  [[nodiscard]] const UiLayout& GetResolvedLayout() const { return resolvedLayout_; }

  virtual void Relayout(const UiRect& rect);

 protected:
  virtual void RenderBg(UiRenderer& renderer);

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
  std::optional<UiTextureRegion> image_;
  glm::vec4 bgColor_ = glm::vec4(1.0f);
  std::optional<UiTextureRegion> bgImage_;
  float radius_ = 0.0f; 
  std::optional<float> bgRadius_;
  // Properties
  bool visible_ = true;

  bool hovered_ = false;
  bool prevHovered_ = false;

  mutable bool layoutDirty_ = false;
  mutable UiLayout resolvedLayout_;
};

}  // namespace gfx