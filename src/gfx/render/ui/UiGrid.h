#pragma once

#include "UiContainer.h"

namespace gfx {
class UiGrid : public UiContainer {
 public:
  UiGrid(std::string_view id, const UiElement* parent) : UiContainer(id, parent) {}

  [[nodiscard]] UiElementType GetNodeType() const override {
    return UiElementType::Grid;
  }

  void ApplyStyle(const UiStyle& style) override;

  [[nodiscard]] const glm::vec2& GetCell() const { return cell_; }
  void SetCell(const glm::vec2& cell) {
    cell_ = cell;
    ArrangeChildren();
  }

  [[nodiscard]] const glm::vec2& GetGap() const { return gap_; }
  void SetGap(const glm::vec2& gap) {
    gap_ = gap;
    ArrangeChildren();
  }

  [[nodiscard]] const glm::vec2& GetInset() const { return inset_; }
  void SetInset(const glm::vec2& value) {
    inset_ = value;
    ArrangeChildren();
  }

  [[nodiscard]] size_t GetColumns() const { return columns_; }
  void SetColumns(size_t columns) {
    columns_ = columns;
    ArrangeChildren();
  }

  UiElement& AddChild(std::unique_ptr<UiElement>&& element) {
    auto& child = UiContainer::AddChild(std::move(element));
    ArrangeChildren();
    return child;
  }

 private:
  void ArrangeChildren();

  [[nodiscard]] glm::vec2 ResolveSize(const UiRect& parent) const override;

  glm::vec2 cell_;
  glm::vec2 gap_;
  glm::vec2 inset_;
  size_t columns_ = 0;
};
}  // namespace gfx