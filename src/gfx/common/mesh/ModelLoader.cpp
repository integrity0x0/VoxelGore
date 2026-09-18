#include "ModelLoader.h"

#include <tiny_obj_loader.h>

#include <filesystem>
#include <span>
#include <sstream>

#include "../../../vkcore/resource/memoryUtils.h"

#ifdef __ANDROID__
#include <android/asset_manager.h>

extern AAssetManager* g_AAssetManager;

class AndroidMaterialReader : public tinyobj::MaterialReader {
 public:
  explicit AndroidMaterialReader(std::string_view baseDir) : baseDir_(baseDir) {}

  bool operator()(const std::string& matId, std::vector<tinyobj::material_t>* materials,
                  std::map<std::string, int>* matMap, std::string* warn,
                  std::string* err) override {
    const std::string path = baseDir_ + matId;

    AAsset* asset = AAssetManager_open(g_AAssetManager, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
      if (err) *err += "Failed to open MTL: " + path + "\n";
      return false;
    }

    const size_t size = static_cast<size_t>(AAsset_getLength(asset));
    const char* data = static_cast<const char*>(AAsset_getBuffer(asset));

    if (!data) {
      AAsset_close(asset);
      if (err) *err += "Failed to read MTL: " + path + "\n";
      return false;
    }

    std::string mtlData(data, size);
    AAsset_close(asset);

    std::istringstream stream(mtlData);
    tinyobj::LoadMtl(matMap, materials, &stream, warn, err);

    return true;
  }

 private:
  std::string baseDir_;
};
#endif

namespace gfx {

std::optional<Model> ModelLoader::Load(ModelId id, const vkcore::Device& device,
                                       vkcore::TransferContext& transferCtxt,
                                       vkcore::BufferAllocator& bufferAllocator,
                                       MaterialManager& materialCache, std::string_view path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

#ifdef __ANDROID__
  AAsset* asset = AAssetManager_open(g_AAssetManager, path.data(), AASSET_MODE_BUFFER);
  if (!asset) {
    throw std::runtime_error("Failed to open model asset: " + std::string(path));
  }

  const size_t size = static_cast<size_t>(AAsset_getLength(asset));
  const char* data = static_cast<const char*>(AAsset_getBuffer(asset));

  if (!data) {
    AAsset_close(asset);
    throw std::runtime_error("Failed to read model asset: " + std::string(path));
  }

  std::string modelData(data, size);
  AAsset_close(asset);

  std::istringstream stream(modelData);

  std::filesystem::path modelPath(path);
  std::string baseDir = modelPath.parent_path().string();
  if (!baseDir.empty()) baseDir += "/";

  AndroidMaterialReader materialReader(baseDir);

  bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &stream, &materialReader);
#else
  std::filesystem::path modelPath(path);
  std::string baseDir = modelPath.parent_path().string();
  if (!baseDir.empty()) baseDir += "/";

  bool ok =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.data(), baseDir.c_str());
#endif

  if (!warn.empty()) std::cout << warn << std::endl;
  if (!err.empty()) throw std::runtime_error(err);
  // if (!err.empty()) std::cerr << err << std::endl; TODO: implement logger
  if (!ok) return std::nullopt;
  std::unordered_map<int, RawSubmesh> buckets;

  for (const auto& shape : shapes) {
    size_t indexOffset = 0;
    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
      int faceVertexCount = shape.mesh.num_face_vertices[f];
      int matId = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[f];

      RawSubmesh& bucket = buckets[matId];
      if (bucket.texturePath.empty() && matId >= 0 && !materials[matId].diffuse_texname.empty()) {
        bucket.texturePath = baseDir + materials[matId].diffuse_texname;
      }

      for (int v = 0; v < faceVertexCount; ++v) {
        const auto& index = shape.mesh.indices[indexOffset + v];
        Model::Vertex vertex{};
        vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                      attrib.vertices[3 * index.vertex_index + 1],
                      attrib.vertices[3 * index.vertex_index + 2]};

        if (index.texcoord_index >= 0) {
          vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
                       1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
        }

        if (index.normal_index >= 0) {
          vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                           attrib.normals[3 * index.normal_index + 1],
                           attrib.normals[3 * index.normal_index + 2]};
        }

        auto it = bucket.uniqueVertices.find(vertex);
        if (it == bucket.uniqueVertices.end()) {
          uint32_t newIndex = static_cast<uint32_t>(bucket.vertices.size());
          bucket.uniqueVertices[vertex] = newIndex;
          bucket.vertices.push_back(vertex);
          bucket.indices.push_back(newIndex);
        } else {
          bucket.indices.push_back(it->second);
        }
      }
      indexOffset += faceVertexCount;
    }
  }

  if (buckets.empty()) return std::nullopt;

  std::vector<Model::Submesh> submeshes;
  submeshes.reserve(buckets.size());

  for (auto& [matId, bucket] : buckets) {
    if (bucket.vertices.empty() || bucket.indices.empty()) continue;

    transferCtxt.Begin();

    vkcore::BufferSlice vertexBuffer = bufferAllocator.Allocate(
        bucket.vertices.size() * sizeof(Model::Vertex),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkcore::LoadDataToBuffer(device, transferCtxt,
                             {reinterpret_cast<const std::byte*>(bucket.vertices.data()),
                              bucket.vertices.size() * sizeof(Model::Vertex)},
                             vertexBuffer.buffer(), vertexBuffer.offset());

    vkcore::BufferSlice indexBuffer = bufferAllocator.Allocate(
        bucket.indices.size() * sizeof(uint32_t),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkcore::LoadDataToBuffer(device, transferCtxt,
                             {reinterpret_cast<const std::byte*>(bucket.indices.data()),
                              bucket.indices.size() * sizeof(uint32_t)},
                             indexBuffer.buffer(), indexBuffer.offset());

    transferCtxt.Flush();

    const MaterialManager::Material* material =
        bucket.texturePath.empty() ? nullptr : materialCache.Require(bucket.texturePath);

    submeshes.push_back(Model::Submesh{
        Mesh(device, std::move(vertexBuffer), static_cast<uint32_t>(bucket.vertices.size()),
             std::move(indexBuffer), static_cast<uint32_t>(bucket.indices.size())),
        material});
  }

  return Model(id, device, std::move(submeshes));
}

}  // namespace gfx