#pragma once

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace util {

static std::string NormalizePath(const std::string& raw) {
  std::vector<std::string> parts;
  std::stringstream ss(raw);
  std::string segment;

  while (std::getline(ss, segment, '/')) {
    if (segment.empty() || segment == ".") {
      continue;
    }

    if (segment == "..") {
      if (parts.empty()) {
        throw std::runtime_error("Path escapes root: " + raw);
      }
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