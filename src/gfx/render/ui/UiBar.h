#pragma once

#include "UiContainer.h"

namespace gfx {
class UiBar : public UiContainer {
 public:
  UiBar(std::string_view id, const UiElement* parent)
      : UiContainer(id, parent) {}

  [[nodiscard]] virtual UiElementType GetNodeType() const override {
    return UiElementType::Bar;
  }
  
  [[nodiscard]] float GetValue() const { return value_; }
  void SetValue(float value) {
    value = std::clamp(value, 0.0f, max_);
    if (value_ != value) {
      value_ = value;
      MarkVisualDirty();
    }
  }

  [[nodiscard]] float GetMax() const { return max_; }
  void SetMax(float max) {
    max = std::max(max, 0.0f);
    if (max_ != max) {
      max_ = max;
      value_ = std::clamp(value_, 0.0f, max_);
      MarkVisualDirty();
    }
  }

  void SetFillImage(const std::optional<UiTextureRegion>& image) {
    baseVisual_.fillImage = image;
    MarkVisualDirty();
  }

  void SetFillRadius(const std::optional<float>& radius) {
    baseVisual_.fillRadius = radius;
    MarkVisualDirty();
  }

  void SetFillColor(const glm::vec4& color) {
    baseVisual_.fillColor = color;
    MarkVisualDirty();
  }

  [[nodiscard]] const std::optional<UiTextureRegion>& GetFillImage() const { return baseVisual_.fillImage; }
  [[nodiscard]] std::optional<float> GetFillRadius() const { return baseVisual_.fillRadius; }
  [[nodiscard]] glm::vec4 GetFillColor() const { return baseVisual_.fillColor.value_or(kVisualDefaults.fillColor); }

  void Render(UiRenderer& renderer) override;

 private:
  void RenderFill(UiRenderer& renderer);

 private:
  float value_ = 0.0f;
  float max_ = 1.0f;
};
}  // namespace gfx