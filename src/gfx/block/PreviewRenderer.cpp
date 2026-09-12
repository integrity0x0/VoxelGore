#include "PreviewRenderer.h"

#include "../../core/PathPrefixes.h"

namespace gfx::block {

#include "faceVertices.inl"

vkcore::Texture PreviewRenderer::CreateDepthTexture(const vkcore::Device& device,
                                                    vkcore::MemoryAllocator& memoryAllocator) {
  VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  imageCI.imageType = VK_IMAGE_TYPE_2D;
  imageCI.format = kPreviewDepthFormat;
  imageCI.extent = {kPreviewSize, kPreviewSize, 1};
  imageCI.mipLevels = 1;
  imageCI.arrayLayers = 1;
  imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  vkcore::Image image(device, imageCI, memoryAllocator, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VkImageViewCreateInfo viewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  viewCI.image = image.handle();
  viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCI.format = kPreviewDepthFormat;
  viewCI.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};

  vkcore::ImageView imageView(device, viewCI);
  return vkcore::Texture(device, std::move(image), std::move(imageView));
}

vkcore::RenderPass PreviewRenderer::BuildRenderPass(const vkcore::Device& device) {
  return vkcore::RenderPass(
      device,
      std::vector<VkAttachmentDescription>{
          VkAttachmentDescription{
              .format = kPreviewColorFormat,
              .samples = VK_SAMPLE_COUNT_1_BIT,
              .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
              .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
              .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
              .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
              .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          },
          VkAttachmentDescription{
              .format = kPreviewDepthFormat,
              .samples = VK_SAMPLE_COUNT_1_BIT,
              .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
              .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
              .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
              .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
              .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
          }},
      std::vector<vkcore::SubpassDescription>{[] {
        vkcore::SubpassDescription subpass;
        subpass.colorRefs = {VkAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
        subpass.depthStencilRef =
            VkAttachmentReference{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        return subpass;
      }()},
      std::vector<VkSubpassDependency>{
          VkSubpassDependency{
              .srcSubpass = VK_SUBPASS_EXTERNAL,
              .dstSubpass = 0,
              .srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
              .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
              .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
              .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
              .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
          },
          VkSubpassDependency{
              .srcSubpass = 0,
              .dstSubpass = VK_SUBPASS_EXTERNAL,
              .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
              .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
              .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
              .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
              .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
          }});
}

vkcore::DescriptorSetLayout PreviewRenderer::BuildDescriptorSetLayout(
    const vkcore::Device& device) {
  return vkcore::DescriptorSetLayout(
      device, std::vector<VkDescriptorSetLayoutBinding>{VkDescriptorSetLayoutBinding{
                  .binding = 0,
                  .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                  .descriptorCount = 1,
                  .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
              }});
}

vkcore::DescriptorPool PreviewRenderer::BuildDescriptorPool(const vkcore::Device& device) {
  return vkcore::DescriptorPool(device,
                                std::vector<VkDescriptorPoolSize>{VkDescriptorPoolSize{
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}},
                                1);
}

vkcore::PipelineLayout PreviewRenderer::BuildPipelineLayout(const vkcore::Device& device) {
  return vkcore::PipelineLayout(
      device, std::to_array({&descriptorSetLayout_}),
      std::array<VkPushConstantRange, 1u>{
          VkPushConstantRange{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants)}});
}

vkcore::Buffer PreviewRenderer::BuildVertexBuffer(uint32_t blockId) {
  std::array<PreviewVertex, 36> vertices;
  for (uint32_t face = 0; face < 6; face++) {
    gfx::block::SurfaceId surfaceId =
        blockRenderData_->surfaceId(blockId, static_cast<gm::Block::Face>(face));
    const gfx::UvRegion& uvRegion = blockRenderData_->surfaceRegistry().ExtractRegion(surfaceId);
    glm::vec2 uvMin = uvRegion.min;
    glm::vec2 uvScale = uvRegion.max - uvRegion.min;

    for (uint32_t v = 0; v < 6; ++v) {
      uint32_t idx = face * 6 + v;
      const PreviewVertex& base = kFaceVertices[idx];
      vertices[idx].pos = base.pos;
      vertices[idx].uv = uvMin + base.uv * uvScale;
    }
  }

  VkBufferCreateInfo bufferCI = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCI.size = sizeof(vertices);
  bufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vkcore::Buffer buffer(*device_, bufferCI,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void* mapped = buffer.memorySlice().map();
  std::memcpy(mapped, vertices.data(), sizeof(vertices));

  return buffer;
}

PreviewRenderer::PreviewRenderer(const vkcore::Device& device,
                                 const vkcore::CommandPool& commandPool,
                                 const vkcore::DeviceQueue& deviceQueue,
                                 vkcore::MemoryAllocator& memoryAllocator,
                                 RenderData& blockRenderData)
    : device_(&device),
      commandPool_(&commandPool),
      deviceQueue_(&deviceQueue),
      memoryAllocator_(&memoryAllocator),
      blockRenderData_(&blockRenderData),
      depthTexture(CreateDepthTexture(device, memoryAllocator)),
      renderPass_(BuildRenderPass(device)),
      descriptorSetLayout_(BuildDescriptorSetLayout(device)),
      descriptorPool_(BuildDescriptorPool(device)),
      descriptorSet_(descriptorPool_.Allocate(descriptorSetLayout_)),
      pipelineLayout_(BuildPipelineLayout(device)),
      pipeline_(
          vkcore::GraphicsPipelineCreator(device)
              .AddShaderStage(core::kAssetsPrefix + "shaders/blockPreview.vert.spv",
                              VK_SHADER_STAGE_VERTEX_BIT)
              .AddShaderStage(core::kAssetsPrefix + "shaders/blockPreview.frag.spv",
                              VK_SHADER_STAGE_FRAGMENT_BIT)
              .setCullMode(VK_CULL_MODE_NONE)
              .setDepthTest(true, true)
              .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
              .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
              .AddColorBlendAttachment(true)
              .AddVertexBinding(0, sizeof(gfx::block::PreviewVertex), VK_VERTEX_INPUT_RATE_VERTEX)
              .AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT,
                                  offsetof(gfx::block::PreviewVertex, pos))
              .AddVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT,
                                  offsetof(gfx::block::PreviewVertex, uv))
              .Build(pipelineLayout_.handle(), renderPass_.handle(), 0)) {
  VkDescriptorImageInfo imageInfo = {};
  imageInfo.sampler = blockRenderData.atlas().texture().sampler().handle();
  imageInfo.imageView = blockRenderData.atlas().texture().imageView().handle();
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write.dstSet = descriptorSet_.handle();
  write.dstBinding = 0;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.pImageInfo = &imageInfo;
  device_->dispatchTable().vkUpdateDescriptorSets(device_->handle(), 1, &write, 0, nullptr);
}

vkcore::SampledTexture PreviewRenderer::Render(uint32_t blockId) {
  vkcore::Buffer vertexBuffer = BuildVertexBuffer(blockId);

  VkImageCreateInfo colorCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  colorCI.imageType = VK_IMAGE_TYPE_2D;
  colorCI.format = kPreviewColorFormat;
  colorCI.extent = {kPreviewSize, kPreviewSize, 1};
  colorCI.mipLevels = 1;
  colorCI.arrayLayers = 1;
  colorCI.samples = VK_SAMPLE_COUNT_1_BIT;
  colorCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  colorCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  colorCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  colorCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  vkcore::Image colorImage(*device_, colorCI, *memoryAllocator_,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VkImageViewCreateInfo colorViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  colorViewCI.image = colorImage.handle();
  colorViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
  colorViewCI.format = kPreviewColorFormat;
  colorViewCI.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  vkcore::ImageView colorView(*device_, colorViewCI);

  vkcore::Framebuffer framebuffer = renderPass_.MakeFramebuffer(
      {&colorView, &depthTexture.imageView()}, kPreviewSize, kPreviewSize);

  VkSamplerCreateInfo samplerCI = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  samplerCI.magFilter = VK_FILTER_LINEAR;
  samplerCI.minFilter = VK_FILTER_LINEAR;
  samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerCI.maxLod = 1.0f;
  vkcore::Sampler sampler(*device_, samplerCI);

  vkcore::CommandBuffer cmd = commandPool_->beginSingleUse();

  std::vector<VkClearValue> clearValues(2);
  clearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
  clearValues[1].depthStencil = {1.0f, 0};

  VkRect2D renderArea = {{0, 0}, {kPreviewSize, kPreviewSize}};
  renderPass_.Begin(cmd.handle(), framebuffer, renderArea, clearValues);

  VkViewport viewport = {0.0f, 0.0f, float(kPreviewSize), float(kPreviewSize), 0.0f, 1.0f};
  VkRect2D scissor = {{0, 0}, {kPreviewSize, kPreviewSize}};
  device_->dispatchTable().vkCmdSetViewport(cmd.handle(), 0, 1, &viewport);
  device_->dispatchTable().vkCmdSetScissor(cmd.handle(), 0, 1, &scissor);

  device_->dispatchTable().vkCmdBindPipeline(cmd.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                             pipeline_.handle());

  descriptorSet_.Bind(cmd.handle(), pipelineLayout_.handle());

  VkBuffer vbHandle = vertexBuffer.handle();
  VkDeviceSize vbOffset = 0;
  device_->dispatchTable().vkCmdBindVertexBuffers(cmd.handle(), 0, 1, &vbHandle, &vbOffset);

  glm::mat4 mvp = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
  mvp[1][1] *= -1.0f;

  mvp *= glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  PushConstants pc;
  pc.mvp = mvp;
  pipelineLayout_.PushConstants(cmd.handle(), VK_SHADER_STAGE_VERTEX_BIT, pc);

  device_->dispatchTable().vkCmdDraw(cmd.handle(), 36, 1, 0, 0);

  renderPass_.End(cmd.handle());
  device_->dispatchTable().vkEndCommandBuffer(cmd.handle());

  deviceQueue_->submit(cmd.handle());
  deviceQueue_->waitIdle();

  vkcore::Texture resultTexture(*device_, std::move(colorImage), std::move(colorView));
  return vkcore::SampledTexture(std::move(resultTexture), std::move(sampler));
}

}  // namespace gfx::block