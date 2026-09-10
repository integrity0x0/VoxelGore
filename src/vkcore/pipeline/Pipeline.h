#pragma once

#include "../devices/Device.h"

namespace vkcore {

class Pipeline {
 public:
  Pipeline(const Device& device, UniquePipeline&& pipeline)
      : device_(&device), pipeline_(std::move(pipeline)) {}

  void Bind(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const {
    device_->dispatchTable().vkCmdBindPipeline(cmd, bindPoint, pipeline_.get());
  }

  VkPipeline handle() const { return pipeline_.get(); }

 private:
  const Device* device_;
  UniquePipeline pipeline_;
};

}  // namespace vkcore