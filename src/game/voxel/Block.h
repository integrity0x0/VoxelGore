#pragma once

#include <stdint.h>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "BlockDebrisConfig.h"

namespace gm {

class Block {
 public:
  enum class Face { North, South, West, East, Bottom, Top };
  enum class RenderLayer { Solid, Cutout, Translucent, Count };
  Block(uint32_t id) : id_(id) {}

  [[nodiscard]] uint32_t getId() const { return id_; }
  void setId(uint32_t id) { id_ = id; }

  [[nodiscard]] bool isObstacle() const { return obstacle_; }
  void setObstacle(bool obstacle) { obstacle_ = obstacle; }

  [[nodiscard]] bool isPassingLight() const { return passingLight_; }
  void setPassingLight(bool passingLight) { passingLight_ = passingLight; }

  [[nodiscard]] float getHardness() const { return hardness_; }
  void setHardness(float hardness) { hardness_ = hardness; }

  [[nodiscard]] const glm::ivec3& getEmission() const { return emission_; }
  void setEmission(const glm::ivec3& emission) { emission_ = emission; }

  [[nodiscard]] bool isIgnoreLighting() const { return ignoreLighting_; }
  void setIgnoreLighting(bool ignoreLighting) { ignoreLighting_ = ignoreLighting; }

  [[nodiscard]] const std::string& getSurface(size_t index) const { return surfaces_[index]; }
  [[nodiscard]] const std::array<std::string, 6u>& getSurfaces() const { return surfaces_; }

  void setSurface(size_t index, const std::string& surface) { surfaces_[index] = surface; }
  void setSurfaces(const std::array<std::string, 6u>& surfaces) { surfaces_ = surfaces; }
  void setAllSurfaces(const std::string& surfaces) { surfaces_.fill(surfaces); }

  void setDebrisConfig(const std::optional<BlockDebrisConfig> debrisConfig) {
    debrisConfig_ = debrisConfig;
  }
  const std::optional<BlockDebrisConfig>& debrisConfig() const { return debrisConfig_; }

  [[nodiscard]] const std::string& renderGroup() const { return renderGroup_; }

  void setRenderGroup(const std::string& renderGroup) { renderGroup_ = renderGroup; }

  [[nodiscard]] RenderLayer renderLayer() const { return renderLayer_; }

  void setRenderLayer(RenderLayer renderLayer) { renderLayer_ = renderLayer; }
  [[nodiscard]] float friction() const { return friction_; }
  void setFriction(float friction) { friction_ = friction; }

 private:
  uint32_t id_;
  bool obstacle_ = false;
  bool passingLight_ = false;
  float hardness_ = 0.0f;
  glm::ivec3 emission_ = {};
  bool ignoreLighting_ = false;
  std::array<std::string, 6u> surfaces_ = {};
  std::optional<BlockDebrisConfig> debrisConfig_ = std::nullopt;
  std::string renderGroup_;
  RenderLayer renderLayer_ = RenderLayer::Solid;
  float friction_ = 10.2f;
};

}  // namespace gm