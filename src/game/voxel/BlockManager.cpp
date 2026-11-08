#include "BlockManager.h"

#include <algorithm>

#include "../../util/files.h"

namespace gm {
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