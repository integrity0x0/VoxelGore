#include "Renderer.h"

#include <stdexcept>

#include "../../../core/PathPrefixes.h"

namespace gfx::ui {
vkcore::DescriptorSetLayout Renderer::BuildDescriptorSetLayout(const vkcore::Device& device) {
  return vkcore::DescriptorSetLayout(
      device, std::vector<VkDescriptorSetLayoutBinding>{VkDescriptorSetLayoutBinding{
                  .binding = 0,
                  .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                  .descriptorCount = 1,
                  .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
              }});
}

vkcore::PipelineLayout Renderer::BuildPipelineLayout(const vkcore::Device& device,
                                                     const vkcore::DescriptorSetLayout& layout) {
  return vkcore::PipelineLayout(device, std::to_array({&layout}), {});
}

vkcore::Pipeline Renderer::BuildPipeline(const vkcore::Device& device, VkRenderPass renderPass,
                                         const vkcore::PipelineLayout& pipelineLayout,
                                         const ShaderCompiler& shaderCompiler) {
  vkcore::ShaderModule vert = CompileShaderModule(
      shaderCompiler, device, core::kShadersPrefix + "ui.vert", shaderc_vertex_shader);
  vkcore::ShaderModule frag = CompileShaderModule(
      shaderCompiler, device, core::kShadersPrefix + "ui.frag", shaderc_fragment_shader);

  return vkcore::GraphicsPipelineCreator(device)
      .AddShaderStage(vert, VK_SHADER_STAGE_VERTEX_BIT)
      .AddShaderStage(frag, VK_SHADER_STAGE_FRAGMENT_BIT)
      .AddColorBlendAttachment(true)
      .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .setCullMode(VK_CULL_MODE_NONE)
      .AddVertexBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
      .AddVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos))
      .AddVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv))
      .AddVertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color))
      .Build(pipelineLayout.handle(), renderPass);
}

Renderer::Renderer(const vkcore::Device& device, VkRenderPass renderPass,
                   TextureManager& textureManager, const ShaderCompiler& shaderCompiler)
    : device_(&device),
      textureManager_(&textureManager),
      descriptorSetLayout_(BuildDescriptorSetLayout(device)),
      descriptorRegistry_(device, descriptorSetLayout_, textureManager),
      vertexBuffer_(device,
                    VkBufferCreateInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                       .size = bufferCapacity,
                                       .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       .sharingMode = VK_SHARING_MODE_EXCLUSIVE},
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
      pipelineLayout_(BuildPipelineLayout(device, descriptorSetLayout_)),
      pipeline_(BuildPipeline(device, renderPass, pipelineLayout_, shaderCompiler)),
      mapped_(reinterpret_cast<Vertex*>(vertexBuffer_.memorySlice().map(0))) {}

void Renderer::setScreenSize(glm::vec2 screenSize) {
  screenSize_ = glm::max(screenSize, glm::vec2(1.0f));
}

void Renderer::begin(VkCommandBuffer cmd) {
  currentCommandBuffer_ = cmd;
  currentDescriptorSet_ = VK_NULL_HANDLE;
  vertexIndex_ = 0;
  batchStart_ = 0;

  device_->dispatchTable().vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                             pipeline_.handle());

  VkBuffer buffers[] = {vertexBuffer_.handle()};
  VkDeviceSize offsets[] = {0ll};
  device_->dispatchTable().vkCmdBindVertexBuffers(cmd, 0, 1u, buffers, offsets);
}

glm::vec4 Renderer::resolveUvRect(const TextureRegion& region,
                                  const vkcore::SampledTexture& texture) const {
  if (region.size.x <= 0.0f || region.size.y <= 0.0f) {
    return {0.0f, 0.0f, 1.0f, 1.0f};
  }
  VkExtent3D extent = texture.getTexture().extent();
  glm::vec2 uvPos = region.pos / glm::vec2(extent.width, extent.height);
  glm::vec2 uvSize = region.size / glm::vec2(extent.width, extent.height);
  return {uvPos.x, uvPos.y, uvSize.x, uvSize.y};
}

void Renderer::addQuad(glm::vec2 pos, glm::vec2 size, const TextureRegion& region,
                       glm::vec4 color) {
  constexpr VkDeviceSize maxVertices = bufferCapacity / sizeof(Vertex);
  if (vertexIndex_ + 6 > maxVertices) {
    return;
  }

  const std::string& key = region.src.empty() ? std::string("blank") : region.src;
  VkDescriptorSet set = descriptorRegistry_.Get(key);

  if (set != currentDescriptorSet_ && set) {
    flushBatch();
    device_->dispatchTable().vkCmdBindDescriptorSets(
        currentCommandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_.handle(), 0, 1,
        &set, 0, nullptr);
    currentDescriptorSet_ = set;
    batchStart_ = vertexIndex_;
  }

  const vkcore::SampledTexture* texture = textureManager_->Require(key);
  glm::vec4 uvRect = (texture) ? resolveUvRect(region, *texture) : glm::vec4();
  glm::vec2 uvPos{uvRect.x, uvRect.y};
  glm::vec2 uvSize{uvRect.z, uvRect.w};

  glm::vec2 ndcPos = (pos / screenSize_) * 2.0f - 1.0f;
  glm::vec2 ndcSize = (size / screenSize_) * 2.0f;

  glm::vec2 p0 = ndcPos;
  glm::vec2 p1 = {ndcPos.x + ndcSize.x, ndcPos.y};
  glm::vec2 p2 = {ndcPos.x + ndcSize.x, ndcPos.y + ndcSize.y};
  glm::vec2 p3 = {ndcPos.x, ndcPos.y + ndcSize.y};

  glm::vec2 uv0 = uvPos;
  glm::vec2 uv1 = {uvPos.x + uvSize.x, uvPos.y};
  glm::vec2 uv2 = {uvPos.x + uvSize.x, uvPos.y + uvSize.y};
  glm::vec2 uv3 = {uvPos.x, uvPos.y + uvSize.y};

  mapped_[vertexIndex_++] = {p0, uv0, color};
  mapped_[vertexIndex_++] = {p1, uv1, color};
  mapped_[vertexIndex_++] = {p2, uv2, color};
  mapped_[vertexIndex_++] = {p0, uv0, color};
  mapped_[vertexIndex_++] = {p2, uv2, color};
  mapped_[vertexIndex_++] = {p3, uv3, color};
}

void Renderer::flushBatch() {
  if (vertexIndex_ > batchStart_ && currentCommandBuffer_ != VK_NULL_HANDLE) {
    uint32_t count = vertexIndex_ - batchStart_;
    device_->dispatchTable().vkCmdDraw(currentCommandBuffer_, count, 1, batchStart_, 0);
  }
}

void Renderer::end() {
  flushBatch();
  currentCommandBuffer_ = VK_NULL_HANDLE;
}

}  // namespace gfx::ui