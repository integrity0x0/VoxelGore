#include "GraphicsPipelineCreator.h"

namespace vkcore {

GraphicsPipelineCreator::GraphicsPipelineCreator(const Device& device) : device_(&device) {}

Pipeline GraphicsPipelineCreator::Build(VkPipelineLayout pipelineLayout, VkRenderPass renderPass,
                                        uint32_t subpassIndex) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  shaderStages.reserve(shaderStages_.size());

  for (const auto& shader : shaderStages_) {
    shaderStages.push_back({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = shader.stage,
        .module = shader.module.get().handle(),
        .pName = shader.entryPoint.c_str(),
        .pSpecializationInfo = nullptr,
    });
  }

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings_.size());
  vertexInputInfo.pVertexBindingDescriptions = vertexBindings_.data();
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes_.size());
  vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes_.data();

  VkPipelineViewportStateCreateInfo viewportState = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

  static const VkViewport defaultViewport = {
      0.0f,    0.0f,    // x, y
      1280.0f, 720.0f,  // width, height
      0.0f,    1.0f     // minDepth, maxDepth
  };

  static const VkRect2D defaultScissor = {
      {.x = 0, .y = 0},      // offset
      {.width = 1280, .height = 720}  // extent
  };

  if (viewports_.empty()) {
    viewportState.viewportCount = 1;
    viewportState.pViewports = &defaultViewport;
  } else {
    viewportState.viewportCount = static_cast<uint32_t>(viewports_.size());
    viewportState.pViewports = viewports_.data();
  }

  if (scissors_.empty()) {
    viewportState.scissorCount = 1;
    viewportState.pScissors = &defaultScissor;
  } else {
    viewportState.scissorCount = static_cast<uint32_t>(scissors_.size());
    viewportState.pScissors = scissors_.data();
  }

  VkPipelineColorBlendStateCreateInfo colorBlending = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlending.logicOpEnable = logicOpEnable_ ? VK_TRUE : VK_FALSE;
  colorBlending.logicOp = logicOp_;
  colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments_.size());
  colorBlending.pAttachments = colorBlendAttachments_.data();
  colorBlending.blendConstants[0] = blendConstants_[0];
  colorBlending.blendConstants[1] = blendConstants_[1];
  colorBlending.blendConstants[2] = blendConstants_[2];
  colorBlending.blendConstants[3] = blendConstants_[3];

  VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates_.size());
  dynamicStateInfo.pDynamicStates = dynamicStates_.data();

  VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pipelineInfo.flags = flags_;
  pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  pipelineInfo.pStages = shaderStages.data();
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly_;
  pipelineInfo.pTessellationState = useTessellation_ ? &tessellation_ : nullptr;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer_;
  pipelineInfo.pMultisampleState = &multisampling_;
  pipelineInfo.pDepthStencilState = &depthStencil_;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = dynamicStates_.empty() ? nullptr : &dynamicStateInfo;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = subpassIndex;
  pipelineInfo.basePipelineHandle = basePipelineHandle_;
  pipelineInfo.basePipelineIndex = basePipelineIndex_;

  VkPipeline rawPipeline;

  SystemError::Check(
      device_->dispatchTable().vkCreateGraphicsPipelines(device_->handle(), VK_NULL_HANDLE, 1,
                                                         &pipelineInfo, nullptr, &rawPipeline),
      "failed to create pipeline");

  UniquePipeline pipeline(
      rawPipeline, PipelineDeleter{device_->handle(), device_->dispatchTable().vkDestroyPipeline});

  return Pipeline(*device_, std::move(pipeline));
}

}  // namespace vkcore