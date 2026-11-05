#pragma once

#include "../../../util/StringUtils.h"
#include "UiElement.h"

namespace gfx {

class UiText final : public UiElement {
 public:
  UiText(std::string_view id, const UiElement* parent) : UiElement(id, parent) {}

  [[nodiscard]] UiElementType GetNodeType() const override { return UiElementType::Text; }

  // ---- Text ----
  [[nodiscard]] const std::optional<std::u32string>& GetText() const { return baseVisual_.text; }

  [[nodiscard]] std::string GetTextUtf8() const {
    return baseVisual_.text ? util::Utf32ToUtf8(*baseVisual_.text) : std::string();
  }

  void SetText(const std::optional<std::u32string>& text) {
    baseVisual_.text = text;
    MarkVisualDirty();
  }

  void SetText(std::u32string&& text) {
    baseVisual_.text = std::move(text);
    MarkVisualDirty();
  }

  void SetText(std::string_view utf8) { SetText(util::Utf8ToUtf32(utf8)); }

  [[nodiscard]] FontFamily* GetFont() const { return baseVisual_.font; }

  void SetFont(FontFamily* font) {
    if (baseVisual_.font == font) return;
    baseVisual_.font = font;
    MarkVisualDirty();
  }

  [[nodiscard]] glm::vec4 GetShadowColor() const {
    return baseVisual_.shadowColor.value_or(kVisualDefaults.shadowColor);
  }

  void SetShadowColor(const glm::vec4& color) {
    baseVisual_.shadowColor = color;
    MarkVisualDirty();
  }


  void Render(UiRenderer& renderer) override;

 private:
  void RenderText(UiRenderer& renderer);

  void UpdateVisual() const override;
};

}  // namespace gfx