#pragma once

#include <vulkan/vulkan.h>

#include <utility>

namespace vkcore {

template <typename Handle, typename Deleter>
class UniqueHandle {
 public:
  UniqueHandle() noexcept = default;

  UniqueHandle(Handle handle, Deleter deleter) noexcept
      : handle_(handle), deleter_(std::move(deleter)) {}

  ~UniqueHandle() { reset(); }

  UniqueHandle(const UniqueHandle&) = delete;
  UniqueHandle& operator=(const UniqueHandle&) = delete;

  UniqueHandle(UniqueHandle&& other) noexcept
      : handle_(other.release()), deleter_(std::move(other.deleter_)) {}

  UniqueHandle& operator=(UniqueHandle&& other) noexcept {
    if (this != std::addressof(other)) {
      reset();
      handle_ = other.release();
      deleter_ = std::move(other.deleter_);
    }
    return *this;
  }

  Handle get() const noexcept { return handle_; }

  Handle release() noexcept {
    Handle h = handle_;
    handle_ = VK_NULL_HANDLE;
    return h;
  }

  void reset(Handle handle = VK_NULL_HANDLE) noexcept {
    if (handle_ != VK_NULL_HANDLE) {
      deleter_(handle_);
    }
    handle_ = handle;
  }

  void swap(UniqueHandle& other) noexcept {
    std::swap(handle_, other.handle_);
    std::swap(deleter_, other.deleter_);
  }

  explicit operator bool() const noexcept { return handle_ != VK_NULL_HANDLE; }

  Handle operator*() const noexcept { return handle_; }
  Handle* operator&() noexcept { return &handle_; }

 private:
  Handle handle_ = VK_NULL_HANDLE;
  Deleter deleter_ = {};
};

struct InstanceDeleter {
  PFN_vkDestroyInstance func = nullptr;
  void operator()(VkInstance inst) const {
    if (func) func(inst, nullptr);
  }
};
using UniqueInstance = UniqueHandle<VkInstance, InstanceDeleter>;

struct DeviceDeleter {
  PFN_vkDestroyDevice func = nullptr;
  void operator()(VkDevice dev) const {
    if (func) func(dev, nullptr);
  }
};
using UniqueDevice = UniqueHandle<VkDevice, DeviceDeleter>;

struct SurfaceDeleter {
  VkInstance instance = VK_NULL_HANDLE;
  PFN_vkDestroySurfaceKHR func = nullptr;
  void operator()(VkSurfaceKHR surface) const {
    if (func && instance) func(instance, surface, nullptr);
  }
};
using UniqueSurfaceKHR = UniqueHandle<VkSurfaceKHR, SurfaceDeleter>;

struct SwapchainDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroySwapchainKHR func = nullptr;
  void operator()(VkSwapchainKHR sc) const {
    if (func && device) func(device, sc, nullptr);
  }
};
using UniqueSwapchainKHR = UniqueHandle<VkSwapchainKHR, SwapchainDeleter>;

struct ShaderModuleDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyShaderModule func = nullptr;
  void operator()(VkShaderModule shader) const {
    if (func && device) func(device, shader, nullptr);
  }
};
using UniqueShaderModule = UniqueHandle<VkShaderModule, ShaderModuleDeleter>;

struct PipelineLayoutDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyPipelineLayout func = nullptr;
  void operator()(VkPipelineLayout layout) const {
    if (func && device) func(device, layout, nullptr);
  }
};
using UniquePipelineLayout = UniqueHandle<VkPipelineLayout, PipelineLayoutDeleter>;

struct RenderPassDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyRenderPass func = nullptr;
  void operator()(VkRenderPass rp) const {
    if (func && device) func(device, rp, nullptr);
  }
};
using UniqueRenderPass = UniqueHandle<VkRenderPass, RenderPassDeleter>;

struct PipelineDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyPipeline func = nullptr;
  void operator()(VkPipeline pipeline) const {
    if (func && device) func(device, pipeline, nullptr);
  }
};
using UniquePipeline = UniqueHandle<VkPipeline, PipelineDeleter>;

struct DescriptorSetLayoutDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyDescriptorSetLayout func = nullptr;
  void operator()(VkDescriptorSetLayout layout) const {
    if (func && device) func(device, layout, nullptr);
  }
};
using UniqueDescriptorSetLayout = UniqueHandle<VkDescriptorSetLayout, DescriptorSetLayoutDeleter>;

struct DescriptorPoolDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyDescriptorPool func = nullptr;
  void operator()(VkDescriptorPool pool) const {
    if (func && device) func(device, pool, nullptr);
  }
};
using UniqueDescriptorPool = UniqueHandle<VkDescriptorPool, DescriptorPoolDeleter>;

struct SamplerDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroySampler func = nullptr;
  void operator()(VkSampler sampler) const {
    if (func && device) func(device, sampler, nullptr);
  }
};
using UniqueSampler = UniqueHandle<VkSampler, SamplerDeleter>;

struct FenceDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyFence func = nullptr;
  void operator()(VkFence fence) const {
    if (func && device) func(device, fence, nullptr);
  }
};
using UniqueFence = UniqueHandle<VkFence, FenceDeleter>;

struct SemaphoreDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroySemaphore func = nullptr;
  void operator()(VkSemaphore semaphore) const {
    if (func && device) func(device, semaphore, nullptr);
  }
};
using UniqueSemaphore = UniqueHandle<VkSemaphore, SemaphoreDeleter>;

struct ImageDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyImage func = nullptr;
  void operator()(VkImage image) const {
    if (func && device) func(device, image, nullptr);
  }
};
using UniqueImage = UniqueHandle<VkImage, ImageDeleter>;

struct ImageViewDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyImageView func = nullptr;
  void operator()(VkImageView view) const {
    if (func && device) func(device, view, nullptr);
  }
};
using UniqueImageView = UniqueHandle<VkImageView, ImageViewDeleter>;

struct BufferDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyBuffer func = nullptr;
  void operator()(VkBuffer buffer) const {
    if (func && device) func(device, buffer, nullptr);
  }
};
using UniqueBuffer = UniqueHandle<VkBuffer, BufferDeleter>;

struct DeviceMemoryDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkFreeMemory func = nullptr;
  void operator()(VkDeviceMemory memory) const {
    if (func && device) func(device, memory, nullptr);
  }
};
using UniqueDeviceMemory = UniqueHandle<VkDeviceMemory, DeviceMemoryDeleter>;

struct FramebufferDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyFramebuffer func = nullptr;
  void operator()(VkFramebuffer fb) const {
    if (func && device) func(device, fb, nullptr);
  }
};
using UniqueFramebuffer = UniqueHandle<VkFramebuffer, FramebufferDeleter>;

struct CommandPoolDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyCommandPool func = nullptr;
  void operator()(VkCommandPool pool) const {
    if (func && device) func(device, pool, nullptr);
  }
};
using UniqueCommandPool = UniqueHandle<VkCommandPool, CommandPoolDeleter>;

struct CommandBufferDeleter {
  VkDevice device = VK_NULL_HANDLE;
  VkCommandPool pool = VK_NULL_HANDLE;
  PFN_vkFreeCommandBuffers func = nullptr;
  void operator()(VkCommandBuffer cmdBuf) const {
    if (func && device && pool) func(device, pool, 1, &cmdBuf);
  }
};
using UniqueCommandBuffer = UniqueHandle<VkCommandBuffer, CommandBufferDeleter>;

struct QueryPoolDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyQueryPool func = nullptr;
  void operator()(VkQueryPool pool) const {
    if (func && device) func(device, pool, nullptr);
  }
};
using UniqueQueryPool = UniqueHandle<VkQueryPool, QueryPoolDeleter>;

struct DebugUtilsMessengerDeleter {
  VkInstance instance = VK_NULL_HANDLE;
  PFN_vkDestroyDebugUtilsMessengerEXT func = nullptr;
  void operator()(VkDebugUtilsMessengerEXT messenger) const {
    if (func && instance) func(instance, messenger, nullptr);
  }
};
using UniqueDebugUtilsMessengerEXT =
    UniqueHandle<VkDebugUtilsMessengerEXT, DebugUtilsMessengerDeleter>;

struct DescriptorUpdateTemplateDeleter {
  VkDevice device = VK_NULL_HANDLE;
  PFN_vkDestroyDescriptorUpdateTemplate func = nullptr;
  void operator()(VkDescriptorUpdateTemplate updateTemplate) const {
    if (func && device) func(device, updateTemplate, nullptr);
  }
};
using UniqueDescriptorUpdateTemplate =
    UniqueHandle<VkDescriptorUpdateTemplate, DescriptorUpdateTemplateDeleter>;

struct DescriptorSetDeleter {
  VkDevice device = VK_NULL_HANDLE;
  VkDescriptorPool pool = VK_NULL_HANDLE;
  PFN_vkFreeDescriptorSets func = nullptr;
  void operator()(VkDescriptorSet descriptorSet) const {
    if (func && device && pool) func(device, pool, 1, &descriptorSet);
  }
};
using UniqueDescriptorSet = UniqueHandle<VkDescriptorSet, DescriptorSetDeleter>;

}  // namespace vkcore