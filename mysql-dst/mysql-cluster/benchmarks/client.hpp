#pragma once

#include <pthread.h>
#include <assert.h>

#include <functional>

namespace db {

/**
 * A simple wrapper over pthread APIs
 */
typedef std::function<void *(void) > run_func_t;

class Client {
 public:
  Client()
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
    Client *self = static_cast<Client *>(p);
    self->run_body();
    return nullptr;
  }

}; // class client

} // namespace
 