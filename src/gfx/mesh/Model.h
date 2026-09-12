#pragma once

#include <glm/glm.hpp>
#include <limits>
#include <vector>

#include "../../vkcore/pipeline/PipelineLayout.h"
#include "../texture/MaterialManager.h"
#include "Mesh.h"

namespace gfx {

using ModelId = uint32_t;
static constexpr ModelId kInvalidModelId = std::numeric_limits<ModelId>::max();

class Model {
 public:
  struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;

    [[nodiscard]] bool operator==(const Vertex& other) const {
      return pos == other.pos && uv == other.uv && normal == other.normal;
    }
  };

  struct Submesh {
    Mesh mesh;
    const MaterialManager::Material* material;
  };

  Model(ModelId id, const vkcore::Device& device, std::vector<Submesh>&& submeshes);

  void Draw(VkCommandBuffer cmd, const vkcore::PipelineLayout& pipelineLayout,
            VkBuffer instanceBuffer, VkDeviceSize instanceOffset, uint32_t instanceCount) const;

  ModelId id() const { return id_; }

 private:
  static constexpr uint32_t kMaterialBindingSet = 1u;
  ModelId id_;
  const vkcore::Device* device_;
  std::vector<Submesh> submeshes_;
};

}  // namespace gfx