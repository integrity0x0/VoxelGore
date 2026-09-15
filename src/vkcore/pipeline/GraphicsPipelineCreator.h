#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ShaderModule.h"
#include "Pipeline.h"

namespace vkcore {

class GraphicsPipelineCreator {
 public:
  GraphicsPipelineCreator(const Device& device);

  GraphicsPipelineCreator& AddShaderStage(const ShaderModule& shader, VkShaderStageFlagBits stage,
                                          std::string_view entryPoint = "main") {
    shaderStages_.push_back({shader, stage, std::string(entryPoint)});
    return *this;
  }

  GraphicsPipelineCreator& AddVertexBinding(uint32_t binding, uint32_t stride,
                                            VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX) {
    vertexBindings_.push_back({binding, stride, rate});
    return *this;
  }

  GraphicsPipelineCreator& AddVertexAttribute(uint32_t location, uint32_t binding, VkFormat format,
                                              uint32_t offset) {
    vertexAttributes_.push_back({location, binding, format, offset});
    return *this;
  }

  GraphicsPipelineCreator& setTopology(VkPrimitiveTopology topology) {
    inputAssembly_.topology = topology;
    return *this;
  }

  GraphicsPipelineCreator& setPrimitiveRestart(bool enable) {
    inputAssembly_.primitiveRestartEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
  }

  GraphicsPipelineCreator& setInputAssemblyState(
      const VkPipelineInputAssemblyStateCreateInfo& raw) {
    inputAssembly_ = raw;
    return *this;
  }

  GraphicsPipelineCreator& setPatchControlPoints(uint32_t points) {
    tessellation_.patchControlPoints = points;
    useTessellation_ = true;
    return *this;
  }

  GraphicsPipelineCreator& AddViewport(VkViewport vp) {
    viewports_.push_back(vp);
    return *this;
  }

  GraphicsPipelineCreator& AddScissor(VkRect2D scissor) {
    scissors_.push_back(scissor);
    return *this;
  }

  GraphicsPipelineCreator& setPolygonMode(VkPolygonMode mode) {
    rasterizer_.polygonMode = mode;
    return *this;
  }

  GraphicsPipelineCreator& setCullMode(VkCullModeFlags mode,
                                       VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE) {
    rasterizer_.cullMode = mode;
    rasterizer_.frontFace = frontFace;
    return *this;
  }

  GraphicsPipelineCreator& setLineWidth(float width) {
    rasterizer_.lineWidth = width;
    return *this;
  }

  GraphicsPipelineCreator& setDepthClamp(bool enable) {
    rasterizer_.depthClampEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
  }

  GraphicsPipelineCreator& setDepthBias(bool enable, float constantFactor = 0.0f,
                                        float clamp = 0.0f, float slopeFactor = 0.0f) {
    rasterizer_.depthBiasEnable = enable ? VK_TRUE : VK_FALSE;
    rasterizer_.depthBiasConstantFactor = constantFactor;
    rasterizer_.depthBiasClamp = clamp;
    rasterizer_.depthBiasSlopeFactor = slopeFactor;
    return *this;
  }

  GraphicsPipelineCreator& setRasterizationState(
      const VkPipelineRasterizationStateCreateInfo& raw) {
    rasterizer_ = raw;
    return *this;
  }

  GraphicsPipelineCreator& setSampleCount(VkSampleCountFlagBits samples) {
    multisampling_.rasterizationSamples = samples;
    return *this;
  }

  GraphicsPipelineCreator& setSampleShading(bool enable, float minSampleShading = 1.0f) {
    multisampling_.sampleShadingEnable = enable ? VK_TRUE : VK_FALSE;
    multisampling_.minSampleShading = minSampleShading;
    return *this;
  }

  GraphicsPipelineCreator& setAlphaToCoverage(bool enable) {
    multisampling_.alphaToCoverageEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
  }

  GraphicsPipelineCreator& setMultisampleState(const VkPipelineMultisampleStateCreateInfo& raw) {
    multisampling_ = raw;
    return *this;
  }

  GraphicsPipelineCreator& setDepthTest(bool testEnable, bool writeEnable,
                                        VkCompareOp op = VK_COMPARE_OP_LESS) {
    depthStencil_.depthTestEnable = testEnable ? VK_TRUE : VK_FALSE;
    depthStencil_.depthWriteEnable = writeEnable ? VK_TRUE : VK_FALSE;
    depthStencil_.depthCompareOp = op;
    return *this;
  }

  GraphicsPipelineCreator& setDepthBounds(bool enable, float min = 0.0f, float max = 1.0f) {
    depthStencil_.depthBoundsTestEnable = enable ? VK_TRUE : VK_FALSE;
    depthStencil_.minDepthBounds = min;
    depthStencil_.maxDepthBounds = max;
    return *this;
  }

  GraphicsPipelineCreator& setStencilTest(bool enable, VkStencilOpState front,
                                          VkStencilOpState back) {
    depthStencil_.stencilTestEnable = enable ? VK_TRUE : VK_FALSE;
    depthStencil_.front = front;
    depthStencil_.back = back;
    return *this;
  }

  GraphicsPipelineCreator& setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo& raw) {
    depthStencil_ = raw;
    return *this;
  }

  GraphicsPipelineCreator& AddColorBlendAttachment(bool blendEnable = false) {
    VkPipelineColorBlendAttachmentState state{};
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    state.blendEnable = blendEnable ? VK_TRUE : VK_FALSE;

    if (blendEnable) {
      state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      state.colorBlendOp = VK_BLEND_OP_ADD;

      state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      state.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    colorBlendAttachments_.push_back(state);
    return *this;
  }

  GraphicsPipelineCreator& AddColorBlendAttachment(const VkPipelineColorBlendAttachmentState& raw) {
    colorBlendAttachments_.push_back(raw);
    return *this;
  }

  GraphicsPipelineCreator& setLogicOp(bool enable, VkLogicOp op = VK_LOGIC_OP_COPY) {
    logicOpEnable_ = enable;
    logicOp_ = op;
    return *this;
  }

  GraphicsPipelineCreator& setBlendConstants(float r, float g, float b, float a) {
    blendConstants_[0] = r;
    blendConstants_[1] = g;
    blendConstants_[2] = b;
    blendConstants_[3] = a;
    return *this;
  }

  GraphicsPipelineCreator& AddDynamicState(VkDynamicState state) {
    dynamicStates_.push_back(state);
    return *this;
  }

  GraphicsPipelineCreator& setFlags(VkPipelineCreateFlags flags) {
    flags_ = flags;
    return *this;
  }

  GraphicsPipelineCreator& setBasePipeline(VkPipeline handle, int32_t index = -1) {
    basePipelineHandle_ = handle;
    basePipelineIndex_ = index;
    return *this;
  }

  Pipeline Build(VkPipelineLayout pipelineLayout, VkRenderPass renderPass,
                 uint32_t subpassIndex = 0);

 private:
  struct ShaderStage {
    std::reference_wrapper<const ShaderModule> module;
    VkShaderStageFlagBits stage;
    std::string entryPoint;
  };

  const Device* device_;

  std::vector<ShaderStage> shaderStages_;

  std::vector<VkVertexInputBindingDescription> vertexBindings_;
  std::vector<VkVertexInputAttributeDescription> vertexAttributes_;

  std::vector<VkViewport> viewports_;
  std::vector<VkRect2D> scissors_;

  std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments_;

  std::vector<VkDynamicState> dynamicStates_;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly_ = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  VkPipelineTessellationStateCreateInfo tessellation_ = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};

  bool useTessellation_ = false;

  VkPipelineRasterizationStateCreateInfo rasterizer_ = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f};

  VkPipelineMultisampleStateCreateInfo multisampling_ = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE};

  VkPipelineDepthStencilStateCreateInfo depthStencil_ = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_FALSE,
      .depthWriteEnable = VK_FALSE,
      .depthCompareOp = VK_COMPARE_OP_LESS};

  bool logicOpEnable_ = false;
  VkLogicOp logicOp_ = VK_LOGIC_OP_COPY;
  float blendConstants_[4] = {0.0f, 0.0f, 0.0f, 0.0f};

  VkPipelineCreateFlags flags_ = 0;
  VkPipeline basePipelineHandle_ = VK_NULL_HANDLE;
  int32_t basePipelineIndex_ = -1;
};

}  // namespace vkcore