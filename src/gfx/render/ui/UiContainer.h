#pragma once

#include <memory>

#include "UiElement.h"

namespace gfx {

class UiContainer final : public UiElement {
 public:
  UiContainer(std::string_view id, const UiElement* parent) : UiElement(id, parent) {}
  ~UiContainer() override {}

  [[nodiscard]] std::span<const std::unique_ptr<UiElement>> GetChildren() const override {
    return children_;
  }

  UiElement& AddChild(std::unique_ptr<UiElement>&& child) {
    assert(child);
    children_.emplace_back(std::move(child));
    return *children_.back();
  }

  virtual void Render(UiRenderer& renderer) override;

  virtual void Relayout(const UiRect& rect) override;
 private:
  std::vector<std::unique_ptr<UiElement>> children_;
};
}  // namespace gfx
