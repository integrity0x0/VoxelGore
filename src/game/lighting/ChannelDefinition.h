#pragma once

#include <string>
#include <glm/glm.hpp>

namespace gm {
struct ChannelDefinition {
  std::string name;
  glm::vec3 color;
  uint8_t decay;
  ChannelDefinition() : color(1.0f), decay(0) {};
  ChannelDefinition(std::string&& id, const glm::vec3& color, uint8_t decay = 1) 
    : name(std::move(id)), color(color), decay(decay) {}

  ChannelDefinition(const std::string& id, const glm::vec3& color, uint8_t decay = 1)
      : name(id), color(color), decay(decay) {}
};
}  // namespace gm