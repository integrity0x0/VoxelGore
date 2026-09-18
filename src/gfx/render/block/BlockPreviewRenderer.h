#pragma once

#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../../vkcore/pipeline/GraphicsPipelineCreator.h"
#include "../../../vkcore/pipeline/Pipeline.h"
#include "../../../vkcore/pipeline/PipelineLayout.h"
#include "../../../vkcore/pipeline/RenderPass.h"
#include "../../../vkcore/resource/Buffer.h"
#include "../../../vkcore/resource/DescriptorPool.h"
#include "../../../vkcore/resource/DescriptorSet.h"
#include "../../../vkcore/resource/DescriptorSetLayout.h"
#include "../../../vkcore/resource/SampledTexture.h"
#include "../../common/shader/ShaderCompiler.h"
#include "BlockRenderData.h"

namespace gfx {

struct BlockPreviewVertex {
  glm::vec3 pos;
  glm::vec2 uv;
};

class BlockPreviewRenderer {
 public:
  BlockPreviewRenderer(const vkcore::Device& device, const vkcore::CommandPool& commandPool,
                  const vkcore::DeviceQueue& deviceQueue, vkcore::MemoryAllocator& memoryAllocator,
                  const ShaderCompiler& shaderCompiler,
                  BlockRenderData& blockRenderData);
  [[nodiscard]] vkcore::SampledTexture Render(uint32_t blockId);

 private:
  [[nodiscard]] vkcore::Texture CreateDepthTexture(const vkcore::Device& device,
                                     vkcore::MemoryAllocator& memoryAllocator);
  [[nodiscard]] vkcore::RenderPass BuildRenderPass(const vkcore::Device& device);
  [[nodiscard]] vkcore::DescriptorSetLayout BuildDescriptorSetLayout(const vkcore::Device& device);
  [[nodiscard]] vkcore::DescriptorPool BuildDescriptorPool(const vkcore::Device& device);
  [[nodiscard]] vkcore::PipelineLayout BuildPipelineLayout(const vkcore::Device& device);
  [[nodiscard]] vkcore::Pipeline BuildPipeline(const vkcore::Device& device,
                                               const ShaderCompiler& shaderCompiler);
  [[nodiscard]] vkcore::Buffer BuildVertexBuffer(uint32_t blockId);

  struct PushConstants {
    glm::mat4 mvp;
  };

 private:
  static constexpr VkFormat kPreviewColorFormat = VK_FORMAT_R8G8B8A8_SRGB;
  static constexpr VkFormat kPreviewDepthFormat = VK_FORMAT_D32_SFLOAT;
  static constexpr uint32_t kPreviewSize = 128u;

  const vkcore::Device* device_;
  const vkcore::CommandPool* commandPool_;
  const vkcore::DeviceQueue* deviceQueue_;
  vkcore::MemoryAllocator* memoryAllocator_;
  BlockRenderData* blockRenderData_;
  vkcore::Texture depthTexture;
  vkcore::RenderPass renderPass_;
  vkcore::DescriptorSetLayout descriptorSetLayout_;
  vkcore::DescriptorPool descriptorPool_;
  vkcore::DescriptorSet descriptorSet_;
  vkcore::PipelineLayout pipelineLayout_;
  vkcore::Pipeline pipeline_;

  const vkcore::SampledTexture* atlas_;
};

}  // namespace gfx