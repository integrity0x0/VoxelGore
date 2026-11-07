#pragma once

#include "../../../util/StringUtils.h"
#include "UiElement.h"

namespace gfx {

struct UiGlyphQuad {
  glm::vec2 pos;
  glm::vec2 size;
  GlyphSlot slot;
};

struct UiTextLayout {
  const Font* font = nullptr;
  uint32_t pixelSize = 0;
  std::u32string text;

  std::vector<UiGlyphQuad> quads;
  glm::vec2 size{0.0f};
};

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
    OnTextContentChanged();
  }

  void SetText(std::u32string&& text) {
    baseVisual_.text = std::move(text);
    OnTextContentChanged();
  }

  void SetText(std::string_view utf8) { 
    SetText(util::Utf8ToUtf32(utf8)); 
    OnTextContentChanged();
  }

  [[nodiscard]] FontFamily* GetFont() const { return baseVisual_.font; }

  void SetFont(FontFamily* font) {
    if (baseVisual_.font == font) return;
    baseVisual_.font = font;
    MarkVisualDirty();
  }

  void SetShadowEnabled(bool enabled) {
    baseVisual_.textShadow = enabled;
    MarkVisualDirty();
  }

  [[nodiscard]] glm::vec4 GetShadowColor() const {
    return baseVisual_.shadowColor.value_or(kVisualDefaults.shadowColor);
  }

  void SetShadowColor(const glm::vec4& color) {
    baseVisual_.shadowColor = color;
    MarkVisualDirty();
  }

  [[nodiscard]] glm::vec2 GetShadowOffset() const {
    return baseVisual_.shadowOffset.value_or(kVisualDefaults.shadowOffset);
  }

  void SetShadowOffset(const glm::vec2& offset) {
    baseVisual_.shadowOffset = offset;
    MarkVisualDirty();
  }

  [[nodiscard]] bool IsItalic() const { return baseVisual_.italic ? *baseVisual_.italic : false; }

  void SetItalic(bool italic) {
    baseVisual_.italic = italic;
    MarkVisualDirty();
  }

  [[nodiscard]] bool IsShadowEnabled() const {
    return baseVisual_.textShadow.value_or(kVisualDefaults.textShadow);
  }

  void Render(UiRenderer& renderer) override;

  private:
  void RenderText(UiRenderer& renderer);

  void UpdateVisual() const override;
  void UpdateTextLayout() const;

  glm::vec2 ResolveSize(const UiRect& parent) const override;

  void OnTextContentChanged() {
    MarkVisualDirty();
    if (size_.x.unit == UiUnit::Auto || size_.y.unit == UiUnit::Auto) MarkLayoutDirty();
  }

  mutable UiTextLayout resolvedTextLayout_;
};

}  // namespace gfx