#pragma once

#include <vector>
#include <sstream>
#include <iterator>

namespace db {

namespace util {

template <typename T>
std::string vector_to_str(const std::vector<T> &vec) {
  std::ostringstream oss; oss << "[";
  if(!vec.empty()) {
    std::copy(vec.begin(), vec.end() - 1,
              std::ostream_iterator<T>(oss, ","));
    oss << vec.back();
  }
  oss << "]";
  return oss.str();
}

} // namespace util

} // namespace db
 