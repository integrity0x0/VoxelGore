#pragma once

#include "../devices/Device.h"

namespace vkcore {

class CommandPool;

class CommandBuffer {
 public:
  ~CommandBuffer() = default;

  VkCommandBuffer handle() const noexcept { return commandBuffer_.get(); }

  void begin(VkCommandBufferUsageFlags flags = 0);
  void end();

  CommandBuffer(const CommandBuffer&) = delete;
  CommandBuffer& operator=(const CommandBuffer&) = delete;

  CommandBuffer(CommandBuffer&&) noexcept = default;
  CommandBuffer& operator=(CommandBuffer&&) noexcept = default;

 private:
  CommandBuffer(const Device& device, UniqueCommandBuffer&& commandBuffer) noexcept
      : device_(&device), commandBuffer_(std::move(commandBuffer)) {}

  friend class CommandPool;

  const Device* device_;
  UniqueCommandBuffer commandBuffer_;
};

}  // namespace vkcore