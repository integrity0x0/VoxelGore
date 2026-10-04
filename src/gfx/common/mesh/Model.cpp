#include "Model.h"

#include <cassert>

namespace gfx {

Model::Model(ModelId id, const vkcore::Device& device, std::vector<Submesh>&& submeshes)
    : id_(id), device_(&device), submeshes_(std::move(submeshes)) {}

void Model::Draw(VkCommandBuffer cmd, const vkcore::PipelineLayout& pipelineLayout,
                 VkBuffer instanceBuffer, VkDeviceSize instanceOffset,
                 uint32_t instanceCount) const {
  const Material* lastMaterial = nullptr;

  for (const auto& sub : submeshes_) {
    if (sub.material != lastMaterial && sub.material) {
      VkDescriptorSet matSet = sub.material->descriptorSet.GetHandle();
      device_->GetDispatchTable().vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipelineLayout.GetHandle(), kMaterialBindingSet,
                                                       1, &matSet, 0, nullptr);
      lastMaterial = sub.material;
    }

    VkBuffer vertexBuffers[] = {sub.mesh.vertexBuffer().GetHandle(), instanceBuffer};
    VkDeviceSize offsets[] = {sub.mesh.vertexBuffer().GetOffset(), instanceOffset};
    device_->GetDispatchTable().vkCmdBindVertexBuffers(cmd, 0, 2, vertexBuffers, offsets);
    sub.mesh.indexBuffer()->BindIndex(cmd);
    device_->GetDispatchTable().vkCmdDrawIndexed(cmd, sub.mesh.indexCount(), instanceCount, 0, 0, 0);
  }
}

}  // namespace gfx