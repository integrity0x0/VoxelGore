#include "GraphicsPipelineCreator.h"

#include <stdexcept>

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <android/asset_manager.h>
extern AAssetManager* g_AAssetManager;
#endif

namespace vkcore {

GraphicsPipelineCreator::GraphicsPipelineCreator(const Device& device) : device_(&device) {}

Pipeline GraphicsPipelineCreator::Build(VkPipelineLayout pipelineLayout, VkRenderPass renderPass,
                                        uint32_t subpassIndex) {
  for (size_t i = 0; i < shaderStages_.size(); ++i)
    shaderStages_[i].pName = entryPointNames_[i].c_str();

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
      {0, 0},      // offset
      {1280, 720}  // extent (width, height)
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
  pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages_.size());
  pipelineInfo.pStages = shaderStages_.data();
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
  SystemError::check(
      device_->dispatchTable().vkCreateGraphicsPipelines(device_->handle(), VK_NULL_HANDLE, 1,
                                                         &pipelineInfo, nullptr, &rawPipeline),
      "failed to create pipeline");

  UniquePipeline pipeline(
      rawPipeline, PipelineDeleter{device_->handle(), device_->dispatchTable().vkDestroyPipeline});
  return Pipeline(*device_, std::move(pipeline));
}

UniqueShaderModule GraphicsPipelineCreator::LoadShaderModule(const Device& device,
                                                             const std::string_view& path) {
  std::vector<uint32_t> data;
  size_t fileSize = 0;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
  AAsset* asset = AAssetManager_open(g_AAssetManager, path.data(), AASSET_MODE_BUFFER);
  if (!asset) {
    throw std::runtime_error("failed to load shader module: unable to open asset " +
                             std::string(path));
  }

  fileSize = static_cast<size_t>(AAsset_getLength(asset));
  data.resize(fileSize / sizeof(uint32_t));

  int readBytes = AAsset_read(asset, data.data(), fileSize);
  AAsset_close(asset);

  if (readBytes < 0 || static_cast<size_t>(readBytes) != fileSize) {
    throw std::runtime_error("failed to load shader module: incomplete read " + std::string(path));
  }
#else
  std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error("failed to load shader module: unable to open file " +
                             std::string(path));
  }

  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  data.resize(fileSize / sizeof(uint32_t));
  file.read(reinterpret_cast<char*>(data.data()), fileSize);

  if (file.gcount() != fileSize) {
    throw std::runtime_error("failed to load shader module: incomplete read " + std::string(path));
  }
#endif

  VkShaderModuleCreateInfo shaderModuleCI = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  shaderModuleCI.pCode = data.data();
  shaderModuleCI.codeSize = fileSize;

  VkShaderModule shaderModuleRaw = VK_NULL_HANDLE;
  SystemError::check(device.dispatchTable().vkCreateShaderModule(device.handle(), &shaderModuleCI,
                                                                 nullptr, &shaderModuleRaw),
                     "failed to create shader module");

  return UniqueShaderModule(shaderModuleRaw,
                            {device.handle(), device.dispatchTable().vkDestroyShaderModule});
}

}  // namespace vkcore