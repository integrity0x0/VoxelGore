#include "Ui.h"

namespace gfx {
UiRenderer CreateRenderer(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
                          const vkcore::RenderPass& renderPass,
                          const ShaderCompiler& shaderCompiler, MaterialManager& materialManager,
                          uint32_t framesCount) {

  UiTextureRegion reg();
  return UiRenderer(device, bufferAllocator, materialManager.GetDescriptorSetLayout(), renderPass,
                    shaderCompiler, , framesCount);
}
Ui::Ui(const vkcore::Device& device, vkcore::BufferAllocator& bufferAllocator,
       script::LuaState& luaState, MaterialManager& materialManager, uint32_t framesCount)
    : parserCtxt_(luaState, materialManager),
      renderer_(device, , materialManager.GetDescriptorSetLayout(), renderPass, shaderCompiler, ){

}
}  // namespace gfx