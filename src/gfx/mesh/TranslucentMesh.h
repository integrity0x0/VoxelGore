#pragma once

#include <vector>

#include "Mesh.h"
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"

namespace gfx {

class TranslucentMesh {
 public:
  struct QuadEntry {
    glm::vec3 center;
    uint32_t indices[6];
  };

  TranslucentMesh(const vkcore::Device& device, Mesh&& mesh,
                  vkcore::BufferAllocator& bufferAllocator, uint32_t framesCount,
                  std::vector<QuadEntry> quads);

  void Sort(uint32_t currentFrame, const glm::vec3& cameraPos, float threshold = 1.0f);

  void Draw(VkCommandBuffer cmd, uint32_t currentFrame);

  bool IsEmpty() const { return mesh_.indexCount() == 0; }
  const Mesh& mesh() const { return mesh_; }

 private:
  struct FrameData {
    vkcore::BufferSlice indexBuffer;
    uint32_t* mapped;
    glm::vec3 lastSortPos{0.0f};
    bool sortedOnce = false;

    FrameData(vkcore::BufferSlice&& indexBuffer, uint32_t* mapped)
        : indexBuffer(std::move(indexBuffer)), mapped(mapped) {}
  };
  const vkcore::Device* device_;
  Mesh mesh_;
  std::vector<glm::vec3> centers_;
  std::vector<std::array<uint32_t, 6>> quadIndices_;
  std::vector<uint32_t> order_;
  std::vector<FrameData> frames_;
};

}  // namespace gfx