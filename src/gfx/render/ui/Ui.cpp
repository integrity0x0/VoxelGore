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
                               framesCount)) {}

void Ui::Push(std::string_view path) {
  const auto& it = cached_.find(path);

  if (it != cached_.end()) {
    pagesStack_.push(it->second.get());
    return;
  }

  UiPageParseResult result = UiPageParser::Parse(path, parserCtxt_);

  for (const auto& it : result.diagnostics) {
    // TODO: add logger
  }

  if (result.page) {
    result.page->Relayout(screen_);
    pagesStack_.push(result.page.get());
    cached_[std::string(path)] = std::move(result.page);    
  }
}

void Ui::Pop() {
  if (!pagesStack_.empty()) pagesStack_.pop();
}

void Ui::Clear() {
  pagesStack_ = {};
  cached_.clear();
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
  if (UiPage* page = pagesStack_.top()) {
    renderer_.BeginFrame(cmd, currentFrame);
    renderer_.PushConstants(cmd, {ndcScale_});
    page->Render(renderer_);
  }
}
}  // namespace gfx\