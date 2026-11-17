// BlockManager.cpp
#include "BlockManager.h"

#include <algorithm>
#include <stdexcept>

#include "../../util/files.h"

namespace gm {
namespace {

std::string StemFromPath(std::string_view path) {
  const size_t slash = path.find_last_of("/\\");
  const std::string_view file = (slash == std::string_view::npos) ? path : path.substr(slash + 1);

  const size_t dot = file.rfind('.');
  const std::string_view stem = (dot == std::string_view::npos) ? file : file.substr(0, dot);

  return std::string(stem);
}

}  // namespace

void BlockManager::Load(std::string_view path) {
  const std::string name = StemFromPath(path);

  if (ids_.contains(name)) {
    throw std::runtime_error("BlockManager: duplicate block id \"" + name + "\"");
  }

  const uint32_t id = static_cast<uint32_t>(blocks_.size());
  auto block = std::make_unique<Block>(BlockParser::Parse(id, path));

  ids_.emplace(name, id);
  blocks_.push_back(std::move(block));
}

void BlockManager::LoadDirectory(std::string_view dir) {
  std::string base(dir);
  if (!base.empty() && base.back() != '/') base += '/';

  auto names = util::ListFiles(base);
  std::erase_if(names, [](const std::string& n) { return !n.ends_with(".json"); });
  std::sort(names.begin(), names.end());

  const auto air = std::find(names.begin(), names.end(), "air.json");
  if (air == names.end()) throw std::runtime_error("air.json not found in " + base);
  std::rotate(names.begin(), air, air + 1);

  for (const auto& name : names) Load(base + name);
}
}  // namespace gm