#pragma once

#include "common.h"

#include <pthread.h>
#include <assert.h>

#include <functional>

namespace r2 {

/**
 * A simple wrapper over pthread APIs
 */
typedef std::function<void *(void) > run_func_t;

class Thread {
 public:
  Thread()
  {
  }

  void start() {
    running_ = true;
    pthread_attr_t attr;
    assert(pthread_attr_init(&attr) == 0);
    assert(pthread_create(&pid_, &attr, pthread_bootstrap, (void *) this) == 0);
    assert(pthread_attr_destroy(&attr) == 0);
  }

  void join() {
    running_ = false;
    assert(pthread_join(pid_,nullptr) == 0);
  }

  virtual void *run_body() = 0;

 protected:
  bool running_ = false;

 private:
  pthread_t  pid_;      // pthread id

  // a simple wrapper to call
  static void *pthread_bootstrap(void *p) {
    Thread *self = static_cast<Thread *>(p);
    self->run_body();
    return nullptr;
  }

  DISABLE_COPY_AND_ASSIGN(Thread);
}; // class thread

} // namespace r2
 