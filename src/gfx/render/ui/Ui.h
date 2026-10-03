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
 private:
  [[nodiscard]] UiRenderer CreateRenderer(const vkcore::Device& device,
                                          vkcore::BufferAllocator& bufferAllocator,
                                          const vkcore::RenderPass& renderPass,
                                          const ShaderCompiler& shaderCompiler,
                                          MaterialManager& materialManager, uint32_t framesCount);
 private:
  UiPageParser::Context parserCtxt_;
  UiRenderer renderer_;
  std::unordered_map<std::string, std::unique_ptr<UiPage>> cached_;
  std::stack<UiPage*> pagesStack_;
};
}  // namespace gfx
