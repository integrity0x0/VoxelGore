#include "Sky.h"

#include <array>

namespace gfx {

namespace {
std::array<std::string, 6> FacePaths(const std::string& dir) {
  return {dir + "right.png",  dir + "left.png",  dir + "top.png",
          dir + "bottom.png", dir + "front.png", dir + "back.png"};
}
}  // namespace

std::optional<Sky> Sky::Load(const vkcore::Device& device, vkcore::TransferContext& transferCtxt,
                             vkcore::MemoryAllocator& memoryAllocator,
                             const vkcore::DescriptorPool& descriptorPool,
                             const vkcore::DescriptorSetLayout& descriptorSetLayout,
                             const std::string& assetsPrefix, uint32_t mipLevels) {
  const auto load = [&](const char* name) {
    return Skybox::Load(device, transferCtxt, memoryAllocator, descriptorPool, descriptorSetLayout,
                        FacePaths(assetsPrefix + "skybox/" + name + "/"), mipLevels);
  };

  auto day = load("day");
  auto sunset = load("sunset");
  auto night = load("night");
  if (!day || !sunset || !night) return std::nullopt;

  return Sky(std::move(*day), std::move(*sunset), std::move(*night));
}

void Sky::Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const {
  day_.Bind(cmd, pipelineLayout, kDaySet);
  sunset_.Bind(cmd, pipelineLayout, kSunsetSet);
  night_.Bind(cmd, pipelineLayout, kNightSet);
}

}  // namespace gfx