#pragma once

namespace r2 {

#define DISABLE_COPY_AND_ASSIGN(classname)          \
  private:                                          \
  classname(const classname&) = delete;             \
  classname& operator=(const classname&) = delete

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)

#define ALWAYS_INLINE __attribute__((always_inline))

};
 