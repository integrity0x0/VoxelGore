#include "UiPage.h"

#include <cassert>

namespace gfx {

namespace {
void BindElement(UiElement& element, const script::LuaScript& script,
                 std::vector<std::string>& problems) {
  for (UiCallback& callback : element.GetCallbacks()) {
    if (callback.functionName.empty()) continue;

    callback.ref = script.FindFunction(callback.functionName.c_str());
    if (!callback.ref) {
      problems.push_back("element '" + std::string(element.GetId()) + "': function '" +
                         callback.functionName + "' not found");
    }
  }
  for (const auto& child : element.GetChildren()) BindElement(*child, script, problems);
}
}  // namespace

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

void UiPage::UpdateLayout(const UiRect& screen) {
  for (auto& root : roots_) root->UpdateLayout(screen);
}

void UiPage::Relayout(const UiRect& screen) {
  for (auto& root : roots_) root->Relayout(screen);
}

void UiPage::Render(UiRenderer& renderer) {
  for (auto& root : roots_) root->Render(renderer);
  renderer.Render();
}

std::vector<std::string> UiPage::AttachScript(script::LuaScript&& script) {
  std::vector<std::string> problems;
  for (const auto& root : roots_) BindElement(*root, script, problems);
  script_.emplace(std::move(script));
  return problems;
}

}  // namespace gfx