#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "../../common/texture/Skybox.h"

namespace gfx {
class Sky {
 public:
  [[nodiscard]] static std::optional<Sky> Load(
      const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
      vkcore::MemoryAllocator& memoryAllocator, const vkcore::DescriptorPool& descriptorPool,
      const vkcore::DescriptorSetLayout& descriptorSetLayout, const std::string& assetsPrefix,
      uint32_t mipLevels = 4u);

  void Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const;

 private:
  static constexpr uint32_t kDaySet = 1u;
  static constexpr uint32_t kSunsetSet = 2u;
  static constexpr uint32_t kNightSet = 3u;

  Sky(Skybox&& day, Skybox&& sunset, Skybox&& night)
      : day_(std::move(day)), sunset_(std::move(sunset)), night_(std::move(night)) {}

  Skybox day_;
  Skybox sunset_;
  Skybox night_;
};
}  // namespace gfx