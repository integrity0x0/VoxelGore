#pragma once

#include <vector>

namespace util {
template <typename T>
void SwapAndPop(std::vector<T>& vector, size_t index) {
  vector[index] = std::move(vector.back());
  vector.pop_back();
}
}  // namespace util