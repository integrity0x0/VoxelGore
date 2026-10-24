#pragma once

#include "UiElement.h"

namespace gfx {
class UiImage final : public UiElement {
 public:
  UiImage(std::string_view id, const UiElement* parent)
      : UiElement(id, parent) {}
  [[nodiscard]] UiElementType GetNodeType() const override { return UiElementType::Image; }

  const std::optional<UiTextureRegion>& GetSrc() const {
    return src_;
  }

  void SetSrc(const std::optional<UiTextureRegion>& src) {
    src_ = src;
  }

 private:
  std::optional<UiTextureRegion> src_;
};
}  // namespace gfx
