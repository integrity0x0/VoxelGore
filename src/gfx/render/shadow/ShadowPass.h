#pragma once

#include "../../../vkcore/pipeline/RenderPass.h"

namespace gfx {

class ShadowPass final : public vkcore::RenderPass {
 public:
  static constexpr VkFormat kDepthFormat = VK_FORMAT_D32_SFLOAT;

  explicit ShadowPass(const vkcore::Device& device);

 private:
  static vkcore::RenderPass Build(const vkcore::Device& device);
};

}  // namespace gfx