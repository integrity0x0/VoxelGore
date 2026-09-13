#pragma once

#include <string>
#include <glm/glm.hpp>

namespace gm::lighting {
struct ChannelDefinition {
  std::string id;
  glm::vec3 color;
  uint8_t decay;
  ChannelDefinition() : color(1.0f), decay(0) {};
  ChannelDefinition(std::string&& id, const glm::vec3& color, uint8_t decay = 1) 
    : id(std::move(id)), color(color), decay(decay) {}

  ChannelDefinition(const std::string& id, const glm::vec3& color, uint8_t decay = 1)
      : id(id), color(color), decay(decay) {}
};
}  // namespace gm::lighting