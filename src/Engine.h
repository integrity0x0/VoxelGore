#pragma once

#include <memory>
#include <random>
#include <vector>

#include "vkcore/commands/CommandBuffer.h"
#include "vkcore/commands/CommandPool.h"
#include "vkcore/debug/DebugMessenger.h"
#include "vkcore/devices/Device.h"
#include "vkcore/devices/DeviceCreator.h"
#include "vkcore/devices/PhysicalDevice.h"
#include "vkcore/instances/Instance.h"
#include "vkcore/pipeline/Framebuffer.h"
#include "vkcore/pipeline/RenderPass.h"
#include "vkcore/present/Surface.h"
#include "vkcore/present/Swapchain.h"
#include "vkcore/resource/BufferAllocator.h"
#include "vkcore/resource/Image.h"
#include "vkcore/resource/MemoryAllocator.h"
#include "vkcore/resource/Texture.h"
#include "vkcore/resource/TransferContext.h"
#include "vkcore/sync/Fence.h"
#include "vkcore/sync/Semaphore.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct android_app;
#else
#include <GLFW/glfw3.h>
#endif

class Engine {
 public:
#ifdef VK_USE_PLATFORM_ANDROID_KHR
  Engine(android_app* app);
#else
  Engine(GLFWwindow* window);
#endif
  ~Engine();

  bool beginFrame(uint32_t& imageIndex);
  void BeginRenderPass(uint32_t imageIndex, float r, float g, float b);
  void endRenderPass();
  void endFrame(uint32_t imageIndex);
  uint32_t getCurrentFrameIndex() const { return currentFrame; }
  uint32_t getFramesInFlightCount() const { return framesInFlight; }
  VkExtent2D extent() const;
  const vkcore::CommandBuffer& getCommandBuffer() const { return *commandBuffers[currentFrame]; }
  const vkcore::CommandPool& getCommandPool() const { return *commandPool; }
  const vkcore::Device& getDevice() const { return *device; }
  const vkcore::RenderPass& getRenderPass() const { return *renderPass; }
  vkcore::MemoryAllocator& memoryAllocator() { return *memoryAllocator_; }
  vkcore::BufferAllocator& bufferAllocator() { return *bufferAllocator_; }
  const vkcore::DeviceQueue& getGraphicsQueue() const {
    return device->getQueues()[graphicsQueueIndex];
  }

  const vkcore::DeviceQueue& getPresentQueue() const {
    return device->getQueues()[presentQueueIndex];
  }

  vkcore::TransferContext& transferContext() { return *transferCtxt; }

  void recreateSwapchain();
  void destroySurface();
  void restoreSurface();
  bool IsRenderable() const;

 private:
  static constexpr uint32_t framesInFlight = 1;
  void loadLibrary();
  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createAllocators();
  void createSwapchain();

  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();
  void createCommandPool();
  void createTransferCtxt();
  void createCommandBuffers();

#ifdef VK_USE_PLATFORM_ANDROID_KHR
  android_app* app = nullptr;
#else
  GLFWwindow* nativeWindow = nullptr;
#endif
  std::unique_ptr<vkcore::LibraryLoader> libraryLoader;
  std::unique_ptr<vkcore::Instance> instance;

#if !defined(DISABLE_VVLS)
  std::unique_ptr<vkcore::DebugMessenger> debugMessenger;
#endif
  std::unique_ptr<vkcore::Surface> surface;
  std::unique_ptr<vkcore::PhysicalDevice> physDevice;
  std::unique_ptr<vkcore::Device> device;
  std::unique_ptr<vkcore::Swapchain> swapchain;
  std::unique_ptr<vkcore::RenderPass> renderPass;
  std::vector<vkcore::Framebuffer> framebuffers;

  std::vector<vkcore::Fence> inFlightFences;

  std::vector<vkcore::Semaphore> imageAvailableSemaphores;
  std::vector<vkcore::Semaphore> renderFinishedSemaphores;

  std::unique_ptr<vkcore::CommandPool> commandPool;
  std::unique_ptr<vkcore::TransferContext> transferCtxt;

  std::vector<std::unique_ptr<vkcore::CommandBuffer>> commandBuffers;

  std::unique_ptr<vkcore::MemoryAllocator> memoryAllocator_;
  std::unique_ptr<vkcore::BufferAllocator> bufferAllocator_;
  std::unique_ptr<vkcore::Texture> depthTexture;
  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

  uint32_t currentFrame = 0;

  uint32_t graphicsQueueIndex = 0;
  uint32_t presentQueueIndex = 0;

  std::mt19937 rng;
  std::uniform_real_distribution<float> colorDist;
};