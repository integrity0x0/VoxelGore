#pragma once

#include <unordered_map>
#include <stack>

#include "UiPage.h"
#include "UiPageParser.h"
#include "UiRenderer.h"
#include "UiInputRouter.h"
#include "UiTriggerDispatcher.h"
#include "../../../util/hashers.h"

namespace gfx {
class Ui {
 public:
  Ui(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
     const vkcore::RenderPass& renderPass, const ShaderCompiler& shaderCompiler,
     script::LuaState& luaState, MaterialManager& materialManager, uint32_t framesCount);

  void Push(std::string_view path);

  void Pop();

  void Clear();
  
  UiElement* FindElement(std::string_view id);

  void Render(VkCommandBuffer cmd, uint32_t currentFrame);

  void UpdateLayout();
  void Resize(VkExtent2D extent);

  [[nodiscard]] UiCameraSwipe RouteInput(const core::InputState& inputState);

 private:
  struct StackView {
    size_t firstRender;
    size_t firstInput;
    bool inputCaptured;
  };


  [[nodiscard]] UiRenderer CreateRenderer(const vkcore::Device& device,
                                          vkcore::BufferAllocator& bufferAllocator,
                                          const vkcore::RenderPass& renderPass,
                                          const ShaderCompiler& shaderCompiler,
                                          MaterialManager& materialManager, uint32_t framesCount);
  
  [[nodiscard]] StackView BuildStackView() const;
 private:
  UiPageParser::Context parserCtxt_;
  UiRenderer renderer_;
  std::unordered_map<std::string, std::unique_ptr<UiPage>, util::StringHash, std::equal_to<>>
      cached_;
  std::vector<UiPage*> pages_;
  StackView stackView_ = {};
  UiRect screen_;
  glm::vec2 ndcScale_ = glm::vec2(1.0f);
  UiEventQueue events_;
  UiInputRouter inputRouter_;
  UiTriggerDispatcher triggerDispatcher_;
};
}  // namespace gfx
