#pragma once

#include <unordered_map>
#include <stack>

#include "UiPage.h"
#include "UiPageParser.h"
#include "UiRenderer.h"
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

  void Render(VkCommandBuffer cmd, uint32_t currentFrame);

  void Resize(VkExtent2D extent);
 private:
  [[nodiscard]] UiRenderer CreateRenderer(const vkcore::Device& device,
                                          vkcore::BufferAllocator& bufferAllocator,
                                          const vkcore::RenderPass& renderPass,
                                          const ShaderCompiler& shaderCompiler,
                                          MaterialManager& materialManager, uint32_t framesCount);
 private:
  UiPageParser::Context parserCtxt_;
  UiRenderer renderer_;
  std::unordered_map<std::string, std::unique_ptr<UiPage>, util::StringHash, std::equal_to<>> cached_;
  std::stack<UiPage*> pagesStack_;
  glm::vec2 ndcScale_ = glm::vec2(1.0f);
};
}  // namespace gfx
