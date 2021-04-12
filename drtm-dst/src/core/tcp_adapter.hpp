#pragma once

#include "rworker.h"

#include "msg_handler.h" // abstract interface

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

#include <zmq.hpp>    // a wrapper over zeromq

using namespace nocc::util;

namespace nocc {

// whether to use a dedicated send socket per thread
#define DEDICATED 0
// these things needs to be refined. Since it's TCP, I will do this later.
class AdapterPoller;
extern zmq::context_t recv_context;
extern zmq::context_t send_context;
extern AdapterPoller *poller;
extern std::vector<SingleQueue *>   local_comm_queues;

/* The poller is usd to receive all in-coming messages at a given port.
 * It acts like a thread, and it is imlemented using *nocc_worker*.
 * It dispatches the received message to other adapter, which serves as a message channel.
 */
class AdapterPoller : public oltp::RWorker {

  // communication queue used to communicate with other threads
  std::vector<SingleQueue *> local_queues;

 public:
  AdapterPoller(int id,std::vector<SingleQueue *> &vec,int port) :
      RWorker(id,NULL),local_queues(vec),base_port_(port) {
    running = true;
    inited  = true;
  }

  void create_recv_socket(zmq::context_t &context) {
    recv_socket = new zmq::socket_t(context, ZMQ_PULL);
    //recv_socket1 = new zmq::socket_t(context, ZMQ_PULL);

    char address[32] = "";
    snprintf(address, 32, "tcp://*:%d", base_port_);
    LOG(4) << "poller bind address " << address;
    try {
      recv_socket->bind(address);
    } catch (...) {
      assert(false);
    }
  }
  void worker_routine(yield_func_t &yield) {
    // It does not send other request. No need for a worker routine.
  }

  void run2() {
#if 0
    LOG(4) << "poller running!";

    // first we create 2 sockets
    const int num_recv_sockets = 2;
    zmq::socket_t *recv_sockets[num_recv_sockets];

    for(uint i = 0;i < num_recv_sockets;++i) {

      recv_sockets[i] = new zmq::socket_t(recv_context, ZMQ_PULL);

      char address[32] = "";
      snprintf(address, 32, "tcp://*:%d", base_port_ + i);

      try {
        recv_sockets[i]->bind(address);
      } catch (...) {
        assert(false);
      }
    }

    zmq::pollitem_t *items = new zmq::pollitem_t[num_recv_sockets];
    for(uint i = 0;i < num_recv_sockets;++i) {
      items[i] = { *(recv_sockets[i]),0,ZMQ_POLLIN,0 };
    };

    while(running) {
      zmq::poll(&items[0],num_recv_sockets,-1);
      for(uint i = 0;i < num_recv_sockets;++i) {
        if(items[i].revents & ZMQ_POLLIN) {
          zmq::message_t *msg = new zmq::message_t();
          recv_sockets[i]->recv(msg);
          int tid = *((char *)msg->data());
          int nid = *((char *)msg->data() + sizeof(int));
          ASSERT(tid >= 0 && tid < local_queues.size())
              << "fetch tid " << tid;

          local_queues[tid]->enqueue((char *)(&msg),sizeof(zmq::message_t *));
        }
      }
    }
#endif
  }

  // Thread running function
  // Used to receive all message
  void run() {
    fprintf(stdout,"[NOCC] poller running!\n");
    //BindTo
    while(running) {
      zmq::message_t *msg = new zmq::message_t();

      if(recv_socket->recv(msg,ZMQ_NOBLOCK)) {
        int tid = *((char *)msg->data());
        int nid = *((char *)msg->data() + sizeof(int));
        ASSERT(tid >= 0 && tid < local_queues.size())
            << "fetch tid " << tid;

        local_queues[tid]->enqueue((char *)(&msg),sizeof(zmq::message_t *));
      } // end dispatch one message
#if 0
      zmq::message_t *msg1 = new zmq::message_t();

      if(recv_socket1->recv(msg1,ZMQ_NOBLOCK)) {
        int tid = *((char *)msg1->data());
        int nid = *((char *)msg1->data() + sizeof(int));
        ASSERT(tid >= 0 && tid < local_queues.size())
            << "fetch tid " << tid;

        local_queues[tid]->enqueue((char *)(&msg1),sizeof(zmq::message_t *));
      }
#endif
    }
    recv_socket->close();
    //recv_socket1->close();
    LOG(2) << "poller exit.";
  }

 private:
  int base_port_;
  zmq::socket_t *recv_socket;
};

class Adapter : public MsgHandler {

  static std::vector<zmq::socket_t *> sockets;
  // prevent threads from concurrently accessing the sockets
  static std::vector<std::mutex *>   locks;

 public:
  static void create_shared_sockets(const std::vector<std::string> &network,int tcp_port,
                                    zmq::context_t &context) {

    assert(sockets.size() == 0 && locks.size() == 0);
    for(uint i = 0;i < network.size();++i) {
      auto s = new zmq::socket_t(context, ZMQ_PUSH);
      char address[32] = "";
      snprintf(address, 32, "tcp://%s:%d", network[i].c_str(), tcp_port);
      s->connect(address);
      sockets.push_back(s);
      locks.push_back(new std::mutex());
    }
  }

  static void close_shared_sockets() {
    for(uint i = 0;i < sockets.size();++i)
      sockets[i]->close();
  }

  Adapter(msg_func_t callback,int node_id,int thread_id,SingleQueue *q) :
      callback_(callback),thread_id_(thread_id),queue_(q),node_id_(node_id)
  {
    assert(queue_ != NULL);
  }

  void create_dedicated_sockets(const std::vector<std::string> &network,int port,zmq::context_t &context) {
    for(uint i = 0;i < network.size();++i) {
      auto s = new zmq::socket_t(context, ZMQ_PUSH);
      char address[32] = "";
      snprintf(address, 32, "tcp://%s:%d", network[i].c_str(), port);
      s->connect(address);
      sockets_.push_back(s);
    }
    LOG(4) << "worker " << thread_id_ << " create dedacted socket done.";
  }

  Qp::IOStatus send_to(int node_id,char *msg,int len) { return send_to(node_id,thread_id_,msg,len);}

  Qp::IOStatus send_to(int node_id,int tid,char *msg,int len) {
    zmq::message_t m(len + sizeof(char) + sizeof(char));
    *((char *)(m.data())) = tid;
    *((char *)(m.data()) + sizeof(char)) = node_id_;
    memcpy((char *)(m.data()) + sizeof(char) + sizeof(char),msg,len);
#if DEDICATED
    auto s = sockets_[node_id];
 retry:
    bool res = s->send(m,ZMQ_NOBLOCK);
    if(!res)
      s->send(m); // re-send
#else
    ASSERT(sockets.size() > node_id) << "receive a wrong node id: " << node_id;
    auto s = sockets[node_id];
    auto l = locks[node_id];

    l->lock();
    s->send(m);
    l->unlock();
#endif
  }

  Qp::IOStatus broadcast_to(int *node_ids, int num_of_node, char *msg,int len) {

    for(uint i = 0;i < num_of_node;++i) {
      send_to(node_ids[i],msg,len);
    }
    return Qp::IO_SUCC;
  }

  Qp::IOStatus post_pending(int node_id,int tid,char *msg,int len) {
    return send_to(node_id,tid,msg,len);
  }

  Qp::IOStatus flush_pending() { } // not buffer message for TCP

  int  get_num_nodes() {
#if DEDICATED
    return sockets_.size();
#else
    return sockets.size();
#endif
  }

  int  get_thread_id() { assert(false); return 0; }

  void check() { }

  void  poll_comps() {
    zmq::message_t *msg;
    while(queue_->front((char *)(&msg))) {
      int tid = *((char *)(msg->data())); int nid = *( (char *)(msg->data()) + sizeof(char));
      callback_((char *)(msg->data()) + sizeof(char) + sizeof(char), nid,tid);
      free(msg);
      queue_->pop();
    }
  }
 private:
  /* a set of send sockets, if each adapter use dedicated sockets
   * which is used if DEDICATED == 1
   */
  std::vector<zmq::socket_t *> sockets_;

  int node_id_;   // machine id
  int thread_id_; // thread id
  SingleQueue *queue_;      // internal communication from the Adapterpoller
  msg_func_t callback_;     // msg callback after receiving a message
};
};
 