#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>

#include "../texture/MaterialManager.h"
#include "Model.h"

namespace gfx {

class ModelLoader {
 public:
  ModelLoader() = delete;

  [[nodiscard]] static std::optional<Model> Load(ModelId id, const vkcore::Device& device,
                                                 vkcore::TransferContext& transferCtxt,
                                                 vkcore::BufferAllocator& bufferAllocator,
                                                 MaterialManager& materialCache,
                                                 std::string_view path);

 private:
  struct ModelVertexHash {
    size_t operator()(const Model::Vertex& v) const noexcept {
      size_t seed = 0;
      util::HashCombine(seed, v.pos.x);
      util::HashCombine(seed, v.pos.y);
      util::HashCombine(seed, v.pos.z);
      util::HashCombine(seed, v.uv.x);
      util::HashCombine(seed, v.uv.y);
      util::HashCombine(seed, v.normal.x);
      util::HashCombine(seed, v.normal.y);
      util::HashCombine(seed, v.normal.z);
      return seed;
    }
  };

  struct RawSubmesh {
    std::vector<Model::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Model::Vertex, uint32_t, ModelVertexHash> uniqueVertices;
    std::string texturePath;
  };
};

}  // namespace gfx