#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "../../UvRegion.h"
#include "../../common/texture/MaterialManager.h"
#include "UiStateOverrides.h"
#include "UiRenderer.h"

namespace gfx {

enum class UiNodeType : uint8_t { Undefined, Container, Text, Image, Button, Checkbox, Slider };

class UiElement {
 public:
  using StateFlags = std::array<bool, static_cast<size_t>(UiState::Count)>;
  using StateOverrideTable = std::array<UiStateOverrides, static_cast<size_t>(UiState::Count)>;

  UiElement(std::string_view id, const UiElement* parent);
  
  virtual void Render(UiRenderer& renderer);

  virtual ~UiElement();

  [[nodiscard]] virtual UiNodeType GetNodeType() const = 0;

  [[nodiscard]] const UiSize& GetPos() const { return pos_; }
  [[nodiscard]] const UiSize& GetSize() const { return size_; }

  void SetPos(const UiSize& position) { pos_ = position; }
  void SetSize(const UiSize& size) { size_ = size; }

  [[nodiscard]] const UiPoint& GetAnchor() const { return anchor_; }
  [[nodiscard]] const UiPoint& GetPivot() const { return pivot_; }

  void SetAnchor(UiPoint anchor) { anchor_ = anchor; }
  void SetPivot(UiPoint pivot) { pivot_ = pivot; }

  [[nodiscard]] bool IsVisible() const { return visible_; }
  void SetVisible(bool visible) { visible_ = visible; }

  [[nodiscard]] bool IsEnabled() const { return enabled_; }
  void SetEnabled(bool enabled) { enabled_ = enabled; }

  [[nodiscard]] bool IsChecked() const { return checked_; }
  
  void SetChecked(bool checked) { checked_ = checked; }

  [[nodiscard]] const UiElement* GetParent() const { return parent_; }

  [[nodiscard]] const std::vector<std::unique_ptr<UiElement>>& GetChildren() const {
    return children_;
  }

  [[nodiscard]] UiElement& AddChild(std::unique_ptr<UiElement>&& child) {
    assert(child);
    children_.emplace_back(std::move(child));
  }

  void UpdateState(UiState state, bool value) {
    assert(state < UiState::Count);
    states_[static_cast<size_t>(state)] = value;
  }

  [[nodiscard]] StateOverrideTable& GetOverrideTable() { return overrideTable_; }

  [[nodiscard]] const StateOverrideTable& GetOverrideTable() const { return overrideTable_; }

  virtual void Render(UiRenderer& renderer);

 protected:
  void SetParent(UiElement* parent) { parent_ = parent; }

 private:
  StateFlags states_ = {};

  std::string id_;

  const UiElement* parent_ = nullptr;
  std::vector<std::unique_ptr<UiElement>> children_;

  UiSize pos_;
  UiSize size_;

  std::optional<UiTextureRegion> image_;

  glm::vec4 color_;

  UiPoint anchor_;
  UiPoint pivot_;

  bool visible_ = true;
  bool enabled_ = true;
  bool checked_ = false;

  UiPadding padding_;
  std::optional<UiTextureRegion> bgImage_;

  StateOverrideTable overrideTable_;
};

}  // namespace gfx