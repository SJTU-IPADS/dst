#pragma once

#include <utility>
#include <functional>
#include <string>
#include <vector>
#include <sstream>

#include "dbheader.hpp"
#include "random.hpp"

namespace db {

/**
 * The result returned by a TX run
 */
typedef std::pair<bool, double> txn_result_t;

/**
 * Scheme of the TX function
 */
typedef std::function<txn_result_t (Ndb &myndb, FastRandom &) > test_func_t;

/**
 * A description of the test workload, including its name, ratio, and test function
 */
struct WorkloadDesc {
  WorkloadDesc() {}
  WorkloadDesc(const std::string &name, double frequency, test_func_t fn)
      : name(name), frequency(frequency), fn(fn) {
  }
  std::string name;
  double frequency;
  test_func_t fn;

  std::string to_str() const {
    std::ostringstream oss; oss << "workload [";
    oss << name << "]: frequency: " << frequency;
    return oss.str();
  }
};

typedef std::vector<WorkloadDesc> workload_desc_vec_t;

};
 