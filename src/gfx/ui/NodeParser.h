#pragma once

#include <tinyxml2.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "../../util/pathUtils.h"
#include "Container.h"
#include "Node.h"
#include "Panel.h"

namespace gfx::ui {
class NodeParser {
 public:
  static std::shared_ptr<Node> parse(const tinyxml2::XMLElement& xmlElement, const Node* parent,
                                     const std::string& basePath);

 private:
  using FactoryFn = std::shared_ptr<Node> (*)(const tinyxml2::XMLElement&, const Node*,
                                              const std::string& /*basePath*/);

  static const std::unordered_map<std::string, NodeParser::FactoryFn> factories_;

  static std::shared_ptr<Node> parseContainer(const tinyxml2::XMLElement&, const Node*,
                                              const std::string& basePath);
  static std::shared_ptr<Node> parsePanel(const tinyxml2::XMLElement&, const Node*,
                                          const std::string& basePath);
};
}  // namespace gfx::ui