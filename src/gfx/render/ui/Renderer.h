#pragma once

#include <glm/glm.hpp>

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/resource/Buffer.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "../../common/texture/TextureManager.h"
#include "../../common/shader/ShaderCompiler.h"
#include "DescriptorSetRegistry.h"

namespace gfx::ui {

struct Vertex {
  glm::vec2 pos;
  glm::vec2 uv;
  glm::vec4 color;
};

struct TextureRegion {
  std::string src;
  glm::vec2 pos{0.0f, 0.0f};
  glm::vec2 size{0.0f, 0.0f};
};

class Renderer {
 public:
  Renderer(const vkcore::Device& device, VkRenderPass renderPass, TextureManager& textureManager,
           const ShaderCompiler& shaderCompiler);

  void setScreenSize(glm::vec2 screenSize);

  void begin(VkCommandBuffer cmd);
  void addQuad(glm::vec2 pos, glm::vec2 size, const TextureRegion& region, glm::vec4 color);
  void end();

 private:
  static constexpr VkDeviceSize bufferCapacity = 64'000'000ll;
  static constexpr std::string_view kWhiteTextureKey = "ui.white";

  static vkcore::DescriptorSetLayout BuildDescriptorSetLayout(const vkcore::Device& device);
  static vkcore::PipelineLayout BuildPipelineLayout(const vkcore::Device& device,
                                                    const vkcore::DescriptorSetLayout& layout);
  static vkcore::Pipeline BuildPipeline(const vkcore::Device& device, VkRenderPass renderPass,
                                        const vkcore::PipelineLayout& pipelineLayout,
                                        const ShaderCompiler& shaderCompiler);

  void flushBatch();
  glm::vec4 resolveUvRect(const TextureRegion& region, const vkcore::SampledTexture& texture) const;

  const vkcore::Device* device_;
  TextureManager* textureManager_;

  vkcore::DescriptorSetLayout descriptorSetLayout_;
  DescriptorSetRegistry descriptorRegistry_;

  vkcore::Buffer vertexBuffer_;
  vkcore::PipelineLayout pipelineLayout_;
  vkcore::Pipeline pipeline_;
  Vertex* mapped_;

  glm::vec2 screenSize_{1.0f, 1.0f};
  uint32_t vertexIndex_ = 0;
  uint32_t batchStart_ = 0;

  VkCommandBuffer currentCommandBuffer_ = VK_NULL_HANDLE;
  VkDescriptorSet currentDescriptorSet_ = VK_NULL_HANDLE;
};

}  // namespace gfx::ui