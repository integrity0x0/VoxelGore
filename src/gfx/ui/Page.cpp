#include "Page.h"

#include <tinyxml2.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "NodeParser.h"

#ifdef __ANDROID__
#include <android/asset_manager.h>
extern AAssetManager* g_AAssetManager;
#else
#include <filesystem>
#endif

namespace gfx::ui {

static inline std::vector<char> loadFile(std::string_view path) {
#ifdef ANDROID
  // --- Android: загрузка из assets ---
  AAsset* asset = AAssetManager_open(g_AAssetManager, path.data(), AASSET_MODE_BUFFER);
  if (!asset) {
    throw std::runtime_error("Failed to open asset: " + std::string(path));
  }

  const void* buffer = AAsset_getBuffer(asset);
  const auto size = static_cast<size_t>(AAsset_getLength(asset));

  std::vector<char> bytes(size);
  std::memcpy(bytes.data(), buffer, size);
  AAsset_close(asset);
  return bytes;
#else

  std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + std::string(path));
  }

  const auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> bytes(static_cast<size_t>(size));
  file.read(bytes.data(), size);
  file.close();

  return bytes;
#endif
}

Page::Page(const vkcore::Device& device, const vkcore::CommandPool& commandPool,
           const script::LuaState& luaState, std::string_view xmlPath)
    : xmlPath_(xmlPath) {
  auto bytes = loadFile(xmlPath);
  tinyxml2::XMLDocument doc;
  if (doc.Parse(bytes.data(), bytes.size()) != tinyxml2::XML_SUCCESS) {
    throw std::runtime_error("Failed to parse XML: " + std::string(xmlPath));
  }

  try {
    std::string luaPath(xmlPath_);
    luaPath += ".lua";
    auto luaBytes = loadFile(luaPath);
    script_ = script::LuaScript(luaState, luaBytes, luaPath);
  } catch (/*const std::exception& e*/...) {
    // TODO: implement logger
    script_ = std::nullopt;
  }

  const tinyxml2::XMLElement* root = doc.RootElement();
  if (!root) {
    throw std::runtime_error("XML has no root element: " + std::string(xmlPath));
  }

  for (const tinyxml2::XMLElement* element = root->FirstChildElement(); element;
       element = element->NextSiblingElement()) {
    std::shared_ptr<Node> node = NodeParser::parse(*element, nullptr, std::string(xmlPath));
    nodes_.push_back(node);
  }
}

void Page::render(Renderer& renderer) {
  for (size_t i = 0; i < nodes_.size(); ++i) {
    nodes_[i]->render(renderer);
  }
}

void Page::Update(glm::vec2 screenSize) {
  for (const auto& node : nodes_) {
    node->layout({}, screenSize);
  }

  if (script_.has_value()) {
    triggerDispatcher_.dispatch(nodes_, script_.value());
  }
}

}  // namespace gfx::ui