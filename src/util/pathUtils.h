#pragma once

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace util {

static std::string NormalizePath(std::string_view raw) {
  std::vector<std::string> parts;
  std::stringstream ss(raw.data());
  std::string segment;

  while (std::getline(ss, segment, '/')) {
    if (segment.empty() || segment == ".") {
      continue;
    }

    if (segment == "..") {
      parts.pop_back();
      continue;
    }

    parts.push_back(segment);
  }

  std::string result;
  for (size_t i = 0; i < parts.size(); ++i) {
    if (i > 0) {
      result += '/';
    }
    result += parts[i];
  }
  return result;
}

}  // namespace util