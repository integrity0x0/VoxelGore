#include "Ui.h"

#include "../../../core/PathPrefixes.h"

namespace gfx {
namespace {
static const std::string kBlankPath = core::kAssetsPrefix + "images/blank.png";
}  // namespace

UiRenderer Ui::CreateRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                          const vkcore::RenderPass& renderPass,
                          const ShaderCompiler& shaderCompiler, MaterialManager& materialManager,
                          uint32_t framesCount) {
  auto* material = materialManager.TryGet(materialManager.Load(kBlankPath));
  if (!material) throw std::runtime_error("Unable to load blank image:" + kBlankPath);
  UiTextureRegion reg(*material);
  return UiRenderer(device, bufferAllocator, materialManager.GetDescriptorSetLayout(), renderPass,
                    shaderCompiler, reg, framesCount);
}

Ui::Ui(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
       const vkcore::RenderPass& renderPass, const ShaderCompiler& shaderCompiler,
       script::LuaState& luaState, MaterialManager& materialManager, uint32_t framesCount)
    : parserCtxt_(luaState, materialManager, core::kAssetsPrefix),
      renderer_(CreateRenderer(device, bufferAllocator, renderPass, shaderCompiler, materialManager,
                               framesCount)),
      triggerDispatcher_(luaState) {}

void Ui::Push(std::string_view path) {
  const auto& it = cached_.find(path);

  const UiPage* page = nullptr;
  if (it != cached_.end()) {
    pages_.emplace_back(it->second.get());
    return;
  } else {
    UiPageParseResult result = UiPageParser::Parse(path, parserCtxt_);

    for (const auto& it : result.diagnostics) {
      // TODO: implement logger
    }

    if (result.page) {
      result.page->Relayout(screen_);
      page = result.page.get();
      pages_.emplace_back(result.page.get());
      cached_[std::string(path)] = std::move(result.page);
    }
  }
  stackView_ = BuildStackView();
}

Ui::StackView Ui::BuildStackView() const {
  StackView view = {};
  for (size_t i = pages_.size(); i-- > 0;) {
    const UiPageProps& props = pages_[i]->GetProps();

    if (!view.inputCaptured && props.input == UiInputMode::Capture) {
      view.inputCaptured = true;
      view.firstInput = i;
    }
    if (props.type == UiPageType::Screen) {
      view.firstRender = i;
      if (!view.inputCaptured) view.firstInput = i;  // UiPageType::Screen blocks input
      break;
    }
  }
  return view;
}

void Ui::Pop() {
  if (!pages_.empty()) pages_.pop_back();
  stackView_ = BuildStackView();
}

UiElement* Ui::FindElement(std::string_view id) {
  for (auto it = pages_.rbegin(); it != pages_.rend(); ++it) {
    if (UiElement* e = (*it)->FindById(id)) return e;
  }
  return nullptr;
}

void Ui::Clear() {
  pages_ = {};
  cached_.clear();
}

void Ui::UpdateLayout() {
  for (size_t i = stackView_.firstRender; i < pages_.size(); ++i) {
    pages_[i]->UpdateLayout(screen_);
  }
}

void Ui::Resize(VkExtent2D extent) { 
  glm::vec2 screenSize(extent.width, extent.height);
  screen_ = {glm::vec2(0.0f), screenSize};
  ndcScale_ = 2.0f / screenSize; 
  for (auto& it : cached_) {
    it.second->Relayout(screen_);
  }
}

void Ui::Render(VkCommandBuffer cmd, uint32_t currentFrame) {
  if (pages_.empty()) return;
  if (UiPage* page = pages_.front()) {
    renderer_.BeginFrame(cmd, currentFrame);
    renderer_.PushConstants(cmd, {ndcScale_});
    page->Render(renderer_);
  }
}

UiCameraSwipe Ui::RouteInput(const core::InputState& inputState) {
  UiCameraSwipe swipe =
      inputRouter_.RouteInput(inputState, pages_, stackView_.inputCaptured, events_);
  triggerDispatcher_.Dispatch(events_);
  return swipe;
}
}  // namespace gfx