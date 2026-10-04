#include "UiPage.h"

#include <cassert>

namespace gfx {
void UiPage::IndexById(UiElement& element, ElementsById& out) {
  if (!element.GetId().empty()) {
    [[maybe_unused]] const bool inserted = out.emplace(element.GetId(), &element).second;
    assert(inserted && "duplicate ui id");
  }
  for (const auto& child : element.GetChildren()) IndexById(*child, out);
}

UiPage::UiPage(UiPageProps props, std::vector<std::unique_ptr<UiElement>>&& roots)
    : props_(std::move(props)), roots_(std::move(roots)) {
  for (const auto& root : roots_) IndexById(*root, byId_);
}

UiElement* UiPage::FindById(std::string_view id) const {
  const auto it = byId_.find(id);
  return it == byId_.end() ? nullptr : it->second;
}

void UiPage::Relayout(const UiRect& screen) {
  for (auto& root : roots_) {
    root->Relayout(screen);
  }
}

void UiPage::Render(UiRenderer& renderer) {
  for (auto& root : roots_) root->Render(renderer);
  renderer.Render();
}

}  // namespace gfx