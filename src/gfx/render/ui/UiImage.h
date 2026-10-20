#pragma once

#include "UiElement.h"

namespace gfx {
class UiImage : public UiElement {
 public:
  UiImage(std::string_view id, const UiElement* parent)
      : UiElement(id, parent) {}

  const std::optional<UiTextureRegion>& GetSrc() const {
    return src_;
  }

  void SetSrc(const std::optional<UiTextureRegion>& src) {
    src_ = src;
  }

  void Render(UiRenderer& renderer) override;
 private:
  std::optional<UiTextureRegion> src_;
};
}  // namespace gfx
