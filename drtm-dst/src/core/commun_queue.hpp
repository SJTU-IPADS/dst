#pragma once

#include <vector>
#include <string.h>

namespace nocc {

  namespace util {

    class SingleQueue {
      const int buf_size = 809600;
    public:
      SingleQueue():
        header(0),tailer(0),cycle_buf(new char[buf_size + 64])
      { }

      bool enqueue(char *msg,int size) {

        if((buf_size - (tailer - header)) < size) {
          return false;
        }
        char *buf_ptr = cycle_buf + tailer % buf_size;
        *((int8_t *)buf_ptr) = size;
        memcpy(buf_ptr + sizeof(int8_t),msg,size);
        asm volatile("" ::: "memory");
        *((int8_t *)(buf_ptr + sizeof(int8_t) + size)) = size;
        tailer += (size + 2 * sizeof(int8_t));
        return true;
      }

      bool front(char *entry) {

        char *buf = cycle_buf + header % buf_size;
        int size = *((int8_t *)buf);
        if(size != 0 && (*(int8_t *)(buf + sizeof(int8_t) + size)) == size){
          memcpy(entry,buf + sizeof(int8_t),size);
          return true;
        }
        return false;
      }

      void pop() {
        char *buf = cycle_buf + header % buf_size;
        int size = *((int8_t *)buf);
        memset(buf,0,size + 2 * sizeof(int8_t));
        header += (size + 2 * sizeof(int8_t));
      }

      uint64_t header;
      uint64_t tailer;
      char *   cycle_buf;
    };

    class CommQueue {
      const uint64_t buf_size = 67108864;
    public:
      CommQueue(int threads) {
        for(uint i = 0;i < threads;++i) {
          auto buf = new char[buf_size + 64];
          ASSERT(buf != nullptr);
          cycle_bufs.push_back(buf);
          headers.push_back(0);
          tailers.push_back(0);
        }
      }

      bool enqueue(int tid,char *msg,int size) {
        if( (buf_size - (headers[tid] - tailers[tid])) < size)
          return false;
        char *buf_ptr = cycle_bufs[tid] + tailers[tid] % buf_size;
        *((int8_t *)buf_ptr) = size;
        memcpy(buf_ptr + sizeof(int8_t),msg,size);
        asm volatile("" ::: "memory");
        *((int8_t *)(buf_ptr + sizeof(int8_t) + size)) = size;
        tailers[tid] += (size + 2 * sizeof(int8_t));
        return true;
      }

      bool front(char *entry) {
#if 1
        for(uint i = 0;i < headers.size();++i) {
          current_idx = (rolling_idx++) % headers.size();

          char *buf = cycle_bufs[current_idx] + headers[current_idx] % buf_size;
          int size = *((int8_t *)buf);
          if(size != 0 && (*(int8_t *)(buf + sizeof(int8_t) + size)) == size){
            memcpy(entry,buf + sizeof(int8_t),size);
            return true;
          }
        }
#endif
        return false;
      }

      void pop() {
        char *buf = cycle_bufs[current_idx] + headers[current_idx] % buf_size;
        int size = *((int8_t *)buf);
        memset(buf,0,size + 2 * sizeof(int8_t));
        headers[current_idx] += (size + 2 * sizeof(int8_t));
      }

    private:
      std::vector<char *> cycle_bufs;
      std::vector<uint64_t> headers;
      std::vector<uint64_t> tailers;
      uint64_t rolling_idx;
      int current_idx;
    }; // end class
  };   // end namespace util
};
 