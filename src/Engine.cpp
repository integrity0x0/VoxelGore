#include "Engine.h"

#include "vkcore/debug/DebugMessenger.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <android/log.h>
#include <android_native_app_glue.h>
#define LOG_TAG "VoxelGore"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#include <iostream>
#define LOGI(...)      \
  printf(__VA_ARGS__); \
  printf("\n")
#define LOGE(...)               \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
Engine::Engine(android_app* app)
    : app(app)
#else
Engine::Engine(GLFWwindow* window)
    : nativeWindow(window)
#endif
      ,
      rng(std::random_device{}()),
      colorDist(0.0f, 1.0f) {
  loadLibrary();
  createInstance();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createAllocators();
  createSwapchain();
  createDepthResources();
  createRenderPass();
  createFramebuffers();
  createSyncObjects();
  createCommandPool();
  createTransferCtxt();
  createCommandBuffers();

  LOGI("Engine initialized successfully");
}

Engine::~Engine() {
  if (device && device->handle() != VK_NULL_HANDLE) {
    device->dispatchTable().vkDeviceWaitIdle(device->handle());
  }
  LOGI("Engine destroyed");
}

void Engine::loadLibrary() { libraryLoader = std::make_unique<vkcore::LibraryLoader>(); }
void Engine::createInstance() {
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "VoxelGore";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "VoxelGore";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  std::vector<std::string> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};

#ifdef VK_USE_PLATFORM_ANDROID_KHR
  instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
  instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

  std::vector<std::string> validationLayers;
#if defined(_DEBUG) && !defined(DISABLE_VVLS)
  validationLayers.push_back("VK_LAYER_KHRONOS_validation");
#ifndef __ANDROID__
  instanceExtensions.push_back("VK_EXT_debug_utils");
#endif
#endif

  instance = std::make_unique<vkcore::Instance>(*libraryLoader, instanceExtensions,
                                                validationLayers, appInfo);

#if defined(_DEBUG) && !defined(VK_USE_PLATFORM_ANDROID_KHR)
  debugMessenger = std::make_unique<vkcore::DebugMessenger>(*instance);
#endif

  LOGI("Instance created: %p", (void*)instance->handle());
}

void Engine::createSurface() {
  LOGI("Creating surface...");

#ifdef VK_USE_PLATFORM_ANDROID_KHR
  surface = std::make_unique<vkcore::Surface>(*instance, app->window);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
  surface = std::make_unique<vkcore::Surface>(*instance, nativeWindow);
#endif
  LOGI("Surface created: %p", (void*)surface->handle());
}

void Engine::pickPhysicalDevice() {
  physDevice = vkcore::pick(*instance, std::vector<std::string>({VK_KHR_SWAPCHAIN_EXTENSION_NAME}),
                            std::vector<VkQueueFlags>({VK_QUEUE_GRAPHICS_BIT}));

  const VkPhysicalDeviceProperties& props = physDevice->getProperties();
  LOGI("Physical device: %s", props.deviceName);
  LOGI("API version: %u.%u.%u", VK_VERSION_MAJOR(props.apiVersion),
       VK_VERSION_MINOR(props.apiVersion), VK_VERSION_PATCH(props.apiVersion));
}

void Engine::createLogicalDevice() {
  vkcore::QueueFamilyIndices indices = physDevice->getQueueFamilyIndices(surface->handle());

  if (!indices.graphics.has_value()) {
    throw std::runtime_error("No graphics queue family found");
  }

  vkcore::DeviceCreator creator(*physDevice, *instance);

  // graphics всегда первая очередь -> индекс 0
  creator.addQueue(*indices.graphics);
  graphicsQueueIndex = 0;

  if (indices.present.has_value() && indices.present != indices.graphics) {
    // present family отличается от graphics -> отдельная запись, индекс 1
    creator.addQueue(*indices.present);
    presentQueueIndex = 1;
  } else if (indices.present.has_value()) {
    // одна и та же family — используем ту же очередь
    presentQueueIndex = graphicsQueueIndex;
  } else {
    throw std::runtime_error("No present queue family found");
  }

  creator.addExtensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME})
      .setEnabledFeatures(physDevice->getFeatures());

  device = std::make_unique<vkcore::Device>(creator.createDevice());
}

void Engine::createAllocators() {
  memoryAllocator = std::make_unique<vkcore::MemoryAllocator>(*device);
  bufferAllocator = std::make_unique<vkcore::BufferAllocator>(*device, *memoryAllocator);
}

void Engine::createSwapchain() {
  LOGI("Creating swapchain...");
  swapchain = std::make_unique<vkcore::Swapchain>(*device, *surface);
  LOGI("Swapchain created: %p", (void*)swapchain->handle());
  LOGI("Swapchain extent: %ux%u", swapchain->extent().width, swapchain->extent().height);
  LOGI("Swapchain image count: %u", swapchain->getImageCount());
}

void Engine::createRenderPass() {
  LOGI("Creating render pass...");

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapchain->getImageFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = VK_FORMAT_D16_UNORM;
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  vkcore::SubpassDescription subpass;
  subpass.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorRefs.push_back({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
  subpass.depthStencilRef = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};

  std::vector<vkcore::SubpassDescription> subpasses = {subpass};

  std::vector<VkSubpassDependency> dependencies = {dependency};

  renderPass = std::make_unique<vkcore::RenderPass>(*device, attachments, subpasses, dependencies);

  LOGI("Render pass created: %p", (void*)renderPass->handle());
}

void Engine::createDepthResources() {
  VkExtent2D ext = extent();

  VkImageCreateInfo imageCI{};
  imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCI.imageType = VK_IMAGE_TYPE_2D;
  imageCI.extent = {ext.width, ext.height, 1};
  imageCI.mipLevels = 1;
  imageCI.arrayLayers = 1;
  imageCI.format = depthFormat;
  imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vkcore::Image depthImage(*device, imageCI, *memoryAllocator, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VkImageViewCreateInfo viewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  viewCI.image = depthImage.handle();
  viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCI.format = depthFormat;
  viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  viewCI.subresourceRange.baseMipLevel = 0;
  viewCI.subresourceRange.levelCount = 1;
  viewCI.subresourceRange.baseArrayLayer = 0;
  viewCI.subresourceRange.layerCount = 1;

  vkcore::ImageView depthImageView(*device, viewCI);

  depthTexture =
      std::make_unique<vkcore::Texture>(*device, std::move(depthImage), std::move(depthImageView));
}

void Engine::createFramebuffers() {
  LOGI("Creating framebuffers...");

  const auto& imageViews = swapchain->getImageViews();
  if (imageViews.empty()) throw std::runtime_error("No image views available in swapchain");

  framebuffers.reserve(imageViews.size());

  for (const auto& imageView : imageViews) {
    std::vector<const vkcore::ImageView*> attachments;
    attachments.push_back(&imageView);
    attachments.push_back(&depthTexture->imageView());

    framebuffers.push_back(renderPass->MakeFramebuffer(attachments, swapchain->extent().width,
                                                       swapchain->extent().height, 1, 0));

    LOGI("Framebuffer created: %p", (void*)framebuffers.back().handle());
  }
}

void Engine::createSyncObjects() {
  LOGI("Creating sync objects...");
  uint32_t count = framesInFlight;

  for (uint32_t i = 0; i < count; ++i) {
    inFlightFences.emplace_back(*device, VK_FENCE_CREATE_SIGNALED_BIT);
    imageAvailableSemaphores.emplace_back(*device);
    renderFinishedSemaphores.emplace_back(*device);
  }
}

void Engine::createCommandPool() {
  LOGI("Creating command pool...");
  commandPool =
      std::make_unique<vkcore::CommandPool>(*device, getGraphicsQueue().getQueueFamilyIndex(),
                                            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  LOGI("Command pool: %p", (void*)commandPool->handle());
}

void Engine::createTransferCtxt() {
  transferCtxt = std::make_unique<vkcore::TransferContext>(*device, getGraphicsQueue(),
                                                           *commandPool, *bufferAllocator);
}

void Engine::createCommandBuffers() {
  LOGI("Creating command buffers...");
  uint32_t count = framesInFlight;
  commandBuffers.resize(count);

  for (uint32_t i = 0; i < count; ++i) {
    commandBuffers[i] = std::make_unique<vkcore::CommandBuffer>(commandPool->Allocate());
    LOGI("Command buffer %u created: %p", i, (void*)commandBuffers[i]->handle());
  }
}

bool Engine::beginFrame(uint32_t& imageIndex) {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
  int32_t w = ANativeWindow_getWidth(app->window);
  int32_t h = ANativeWindow_getHeight(app->window);

  if (static_cast<uint32_t>(w) != swapchain->extent().width ||
      static_cast<uint32_t>(h) != swapchain->extent().height) {
    recreateSwapchain();
  }
#endif

  VkFence fence = inFlightFences[currentFrame].handle();
  device->dispatchTable().vkWaitForFences(device->handle(), 1, &fence, VK_TRUE, UINT64_MAX);

  VkResult result = device->dispatchTable().vkAcquireNextImageKHR(
      device->handle(), swapchain->handle(), UINT64_MAX,
      imageAvailableSemaphores[currentFrame].handle(), VK_NULL_HANDLE, &imageIndex);

  device->dispatchTable().vkResetFences(device->handle(), 1, &fence);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    LOGI("Swapchain out of date");
    recreateSwapchain();
    return false;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    LOGE("Failed to acquire swapchain image: %d", result);
    recreateSwapchain();
    return false;
  }

  auto& cmdBuffer = commandBuffers[currentFrame];
  device->dispatchTable().vkResetCommandBuffer(cmdBuffer->handle(), 0);

  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VkCommandBuffer cmd = cmdBuffer->handle();
  if (device->dispatchTable().vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin command buffer");
  }

  return true;
}

void Engine::recreateSwapchain() {
  getGraphicsQueue().waitIdle();

  framebuffers.clear();
  depthTexture.reset();
  swapchain.reset();

  createSwapchain();
  createDepthResources();
  createFramebuffers();
}
void Engine::destroySurface() {
  device->WaitIdle();
  framebuffers.clear();
  depthTexture.reset();
  swapchain.reset();
  surface.reset();
}
void Engine::restoreSurface() {
  if (surface) return;

  createSurface();
  createSwapchain();
  createDepthResources();
  createFramebuffers();
}

bool Engine::IsRenderable() const { return surface != nullptr; }

void Engine::beginRenderPass(uint32_t imageIndex, float r, float g, float b) {
  VkRect2D renderArea = {};
  renderArea.offset = {0, 0};
  renderArea.extent = swapchain->extent();

  std::vector<VkClearValue> clearValues(2);

  clearValues[0].color = {{r, g, b, 1.0f}};

  clearValues[1].depthStencil = {1.0f, 0};

  renderPass->Begin(commandBuffers[currentFrame]->handle(), framebuffers[imageIndex], renderArea,
                    clearValues);
}

void Engine::endRenderPass() { renderPass->End(commandBuffers[currentFrame]->handle()); }

void Engine::endFrame(uint32_t imageIndex) {
  VkCommandBuffer cmd = commandBuffers[currentFrame]->handle();

  if (device->dispatchTable().vkEndCommandBuffer(cmd) != VK_SUCCESS) {
    throw std::runtime_error("Failed to end command buffer");
  }

  VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

  VkSemaphore waitSemas[] = {imageAvailableSemaphores[currentFrame].handle()};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemas;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;

  VkSemaphore signalSemas[] = {renderFinishedSemaphores[currentFrame].handle()};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemas;

  VkResult res = device->dispatchTable().vkQueueSubmit(getGraphicsQueue().handle(), 1, &submitInfo,
                                                       inFlightFences[currentFrame].handle());
  if (res) {
    throw std::runtime_error("Failed to submit draw command buffer");
  }

  VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemas;

  VkSwapchainKHR swapchains[] = {swapchain->handle()};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapchains;
  presentInfo.pImageIndices = &imageIndex;

  VkResult result =
      device->dispatchTable().vkQueuePresentKHR(getPresentQueue().handle(), &presentInfo);

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    recreateSwapchain();
  }

  currentFrame = (currentFrame + 1) % framesInFlight;
}

VkExtent2D Engine::extent() const { return swapchain->extent(); }