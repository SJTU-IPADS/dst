#ifndef NOCC_UTIL_TIMER
#define NOCC_UTIL_TIMER

#include <stdint.h>
#include <vector>
#include <math.h>
#include <fstream>

#include "core/logging.h"

namespace nocc {
namespace util {

class BreakdownTimer {
  const uint64_t max_elems = 1000000;
 public:
  uint64_t sum;
  uint64_t count;
  uint64_t temp;
  std::vector<uint64_t> buffer;
  BreakdownTimer(): sum(0), count(0) {}
  void start() { temp = rdtsc(); }
  void end() { auto res = (rdtsc() - temp);sum += res; count += 1;
    emplace(res);
  }
  void emplace(uint64_t res) {
    if(buffer.size() >= max_elems) return;
    buffer.push_back(res);
  }

  void merge(const std::vector<uint64_t> &b) {
    if(b.size() > 0) {
      LOG(3) << "merge: " << b.size() << " to " << "[" << buffer.size();
      buffer.insert(std::end(buffer),std::begin(b),std::end(b));
    }
  }

  double report() {
    if(count == 0) return 0.0; // avoids divided by zero
    double ret =  (double) sum / (double)count;
    // clear the info
    //sum = 0;
    //count = 0;
    return ret;
  }

  void calculate_detailed(double size = 0.2) {

    if(buffer.size() == 0) {
      fprintf(stderr,"no timer!\n");
      return;
    }
    // first erase some items
    int temp_size = buffer.size();
    int idx = std::floor(buffer.size() * size / 100.0);
    buffer.erase(buffer.begin() + temp_size - idx, buffer.end());
    buffer.erase(buffer.begin(),buffer.begin() + idx );

  }

  void dump() {
    std::ofstream outfile;
    outfile.open("lat.res", std::ios::out);
    for(uint i = 0;i < buffer.size();++i) {
      outfile << i << " " << buffer[i] << "\n";
    }
    outfile.close();
  }

  void sort() {
    // then sort
    std::sort(buffer.begin(),buffer.end());
    //LOG(3) << "time from: " << buffer[0] << " to " << buffer[buffer.size() - 1];
  }

  double report_medium() {
    if(buffer.size() == 0) return 0;
    return buffer[buffer.size() / 2];
  }

  double report_90(){
    if(buffer.size() == 0) return 0;
    int idx = std::floor( buffer.size() * 90 / 100.0);
    return buffer[idx];
  }

  double report_95(){
    if(buffer.size() == 0) return 0;
    int idx = std::floor( buffer.size() * 95 / 100.0);
    return buffer[idx];
  }

  double report_99() {
    if(buffer.size() == 0) return 0;
    int idx = std::floor(buffer.size() * 99 / 100.0);
    return buffer[idx];
  }

  double report_avg() {
    if(buffer.size() == 0) return 0;
    double average = 0;
    uint64_t count  = 0;
    uint idx = std::floor(buffer.size() * 95 / 100.0);
    if (idx == 0)
      return 0;
    ASSERT(idx <= buffer.size()) << idx;
    for(uint i = 0;i < idx;++i) {
      average += (buffer[i] - average) / (++count);
    }
    return average;
  }

  static uint64_t get_one_second_cycle() {
    uint64_t begin = rdtsc();
    sleep(1);
    return rdtsc() - begin;
  }

  static uint64_t get_one_usec_cycles() {
    auto res = get_one_second_cycle();
    double ret =  static_cast<double>(res) / 1000000;
    return static_cast<uint64_t>(ceil(ret));
  }

  static void sleep_us(uint64_t us, uint64_t us_per_cycle) {
    auto start = rdtsc();
    auto gap   = us * us_per_cycle;
    while(gap > (rdtsc() - start)) {
      // pass
    }
    return;
  }

  static double rdtsc_to_ms(uint64_t rdts, uint64_t one_second_cycle) {
    return ((double)rdts / (double)one_second_cycle) * 1000;
  }
};
} // namespace util
}   // namespace nocc
#endif
 