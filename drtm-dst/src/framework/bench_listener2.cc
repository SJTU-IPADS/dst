#include "config.h"
#include "rocc_config.h"
#include "bench_listener.h"

#include "core/logging.h"
#include "client.h"

#include <fstream>
#include <signal.h>

/* global config constants */
extern size_t nthreads;
extern size_t nclients;
extern size_t current_partition;
extern size_t total_partition;
extern size_t coroutine_num;
extern size_t distributed_ratio;

extern std::string exe_name;
extern std::string bench_type;

extern int tcp_port;

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

namespace nocc {

extern RdmaCtrl *cm;
extern std::vector<SingleQueue *>   local_comm_queues; // used for TCP connection
extern zmq::context_t send_context;

namespace oltp {

static std::ofstream log_file;
static bool global_inited = false;
static struct timespec start_t;

void wait_worker_barrier(std::vector<BenchWorker *> &workers);

std::function<void(int)> sigint_callback2;
void sigint_callback_wrapper2(int value)
{
  sigint_callback2(value);
}

#define INIT_RPC_ID 0
#define START_RPC_ID 1
#define RES_RPC_ID   2
#define EXIT_RPC_ID  3

// bench listener's main implementation
BenchLocalListener::BenchLocalListener(unsigned worker_id,const std::vector<RWorker *> &workers,BenchReporter *reporter)
    :reporter_(reporter),
     RWorker(worker_id,cm)
{
  workers_.clear();
  for(auto i = 0;i < workers.size();++i) {
    workers_.push_back(static_cast<BenchWorker *>(workers[i]));
  }

  // 0 is the master id in the cluster
  if(current_partition == 0) {
    // catch the signal
    sigint_callback2 = std::bind1st(std::mem_fun(&BenchLocalListener::sigint_handler),this);
    signal(SIGINT,sigint_callback_wrapper2);

    // open the log file
    char log_file_name[64];
    snprintf(log_file_name,64,"./results/%d.log",
             //exe_name.c_str(),bench_type.c_str(),total_partition,nthreads,coroutine_num,distributed_ratio);
             current_partition
             );
    //LOG(4) << "try log results to " << log_file_name;
    log_file.open(log_file_name,std::ofstream::out);
  } // end master extra stuff

  reporter_->init(&workers_);
}

void BenchLocalListener::run() {

  LOG(2) << "New monitor running!";
  RThreadLocalInit();
  init_routines(1);

#if USE_RDMA
  init_rdma();
  create_qps();
#endif

  // first ensures all worker has initilized
  wait_worker_barrier(workers_);
  inited = true;

  // currently, listener use UD for communication
#if USE_RDMA
  create_rdma_ud_connections(1);
#else
  ASSERT(worker_id_ < local_comm_queues.size()) << "worker: " << worker_id_
                                                << ", total local queues " << local_comm_queues.size();
  create_tcp_connections(local_comm_queues[worker_id_],tcp_port,send_context);
#endif

  // register RPC handlers, then
  ROCC_BIND_STUB(rpc_,&BenchLocalListener::init_rpc_handler,this,INIT_RPC_ID);
  ROCC_BIND_STUB(rpc_,&BenchLocalListener::start_rpc_handler,this,START_RPC_ID);
  ROCC_BIND_STUB(rpc_,&BenchLocalListener::get_result_rpc_handler,this,RES_RPC_ID);
  ROCC_BIND_STUB(rpc_,&BenchLocalListener::exit_rpc_handler,this,EXIT_RPC_ID);

  routine_v1();    // routines are ready to start
  start_routine(); // start worker routines
}

void wait_worker_barrier(std::vector<BenchWorker *> &workers) {

  LOG(3) << "wait worker size:" << workers.size();
  int done = 0;
WAIT_RETRY:
  for(uint i = 0;i < workers.size();++i) {
    if(workers[i]->init_status()) {
      //ASSERT(false) << "worker: " << i << " is inited";
      done += 1;
    }
  }
  if(done != workers.size()) {
    //ASSERT(false);
    const char *fmt = "%d worker finish initilization: ";
    char char_buf[64];
    snprintf(char_buf,64,fmt,done);
    PrintProgress((double)done / workers.size(),char_buf,stderr);
    usleep(3000);
    done = 0;
    goto WAIT_RETRY;
  }

  LOG(2) << "All workers has initilized.";
}

void BenchLocalListener::worker_routine(yield_func_t &yield) {
  if(current_partition == 0)
    worker_routine_master(yield);
  else
    worker_routine_slave(yield);
}

void BenchLocalListener::worker_routine_master(yield_func_t &yield) {
#if 1
  // a global barrier to wait for worker's connection
  while(nresult_returned_ != (total_partition - 1)) {
    yield_next(yield);
  }

  nresult_returned_ = 0;
#endif
  // start all servers
  char *dummy = rpc_->get_static_buf(64);
  assert(total_partition > 0);
  for(int i = 1;i < total_partition;++i) {
    rpc_->append_req(dummy,START_RPC_ID,sizeof(char),1,RRpc::REQ,i);
  }
  // then start at local
  start_workers(); // start the first server

  // routine's main loop
  while(true) {

    if(unlikely(this->running == false)) {

      // master send end RPCs to remotes
      for(uint i = 0;i < total_partition;++i) {
        rpc_->append_req(dummy,EXIT_RPC_ID,sizeof(char),1,RRpc::REQ,i);
      }
      exit_rpc_handler(0,0,NULL,NULL);
      yield_next(yield);
    }

    if(total_partition == 1) {
      sleep(1);
      get_result_rpc_handler(0,0,NULL,NULL);
    }
    else {
      auto t = Clock::now();
      while((nresult_returned_ != total_partition - 1) // received all the replies
            && running) // if the listener is not running, just not wait
      {
        if(epoch_ > MASTER_EPOCH)
          break;
        yield_next(yield);
#if 1
        auto t1 = Clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t).count();
        if(elapsed_ms > 3000) {
          this->running = false;
          ASSERT(epoch_ >= 10);
        }
#endif
      }
      nresult_returned_ = 0;
    }
    // epoch will be added in get_result_rpc_handler
    if(epoch_ > MASTER_EPOCH) {
      //exit_rpc_handler(0,0,NULL,NULL);
      this->running = false;
      //      exit_handler();
    }
  }
  // end master_worker_routine
}

void BenchLocalListener::worker_routine_slave(yield_func_t &yield) {
#if 1
  char *dummy = rpc_->get_static_buf(64);
  *((int *)dummy) = current_partition;
  rpc_->append_req(dummy,INIT_RPC_ID,sizeof(int),1 /* cor_id */,RRpc::REQ,0 /* master id */);

  // wait for master's start RPC
  while(!global_inited)
    yield_next(yield);
#endif
  // slave's main loop
  char msg[64];
  while(running) {

    sleep(1); // report per one second
    char *msg_buf = msg + rpc_->rpc_padding();
    //char *msg_buf = rpc_->get_fly_buf(cor_id_);
    reporter_->collect_data(msg_buf,start_t);
    rpc_->append_req(msg_buf,RES_RPC_ID,reporter_->data_len(),1,RRpc::REQ,0);

    epoch_ += 1;
    if(epoch_ > MASTER_EPOCH + 20) {
      exit_rpc_handler(0,0,NULL,NULL);
    }

    yield_next(yield); // wait to receive RPC requests
  }
}

void BenchLocalListener::sigint_handler(int) {
  running = false;
}

void BenchLocalListener::init_rpc_handler(int id,int cid,char *msg,void *arg) {
  ASSERT(*((int *)msg) == id) << "get wrong id " << *((int *)msg);
  nresult_returned_ += 1;
}

void BenchLocalListener::start_workers() {

  clock_gettime(CLOCK_REALTIME,&start_t);
  LOG(2) << "[LISTENER] receive start RPC.";

  try {
#if 0
    for(auto it = workers_.begin();it != workers_.end();++it) {
      (*it)->routine_v1();
    }
#endif
    for(uint i = 0;i < nthreads;++i) {
      workers_[i]->routine_v1();
    }
    sleep(1);
#if CS
    for(uint i = nthreads;i < nthreads + nclients;++i) {
      workers_[i]->routine_v1();
    }
#endif
  }
  catch(...) {
    assert(false);
  }
  global_inited = true;
  epoch_ = 0; // reset-the epoch
}

void BenchLocalListener::start_rpc_handler(int id,int cid,char *msg,void *arg) {
  // only master should start the routine
  assert(id == 0);
  start_workers();
}

void BenchLocalListener::get_result_rpc_handler(int id, int cid,char *msg, void *arg) {

  if(id != current_partition) {
    reporter_->merge_data(msg,id);
    nresult_returned_ += 1;
  }

  if(nresult_returned_ == total_partition - 1) {
    epoch_ += 1;
    char *buffer = new char[reporter_->data_len() + CACHE_LINE_SZ];
    reporter_->collect_data(buffer,start_t);
    reporter_->merge_data(buffer,0);
    free(buffer);

#ifdef LOG_RESULTS
    reporter_->report_data(epoch_,log_file);
#endif
  }
}

void BenchLocalListener::exit_rpc_handler(int id,int cid, char *msg, void *arg) {

  LOG(2) << "start to exit.";
  running = false;
  for(auto it = workers_.begin();it != workers_.end();++it) {
    (*it)->end_routine();
    //(*it)->join();
  }
}


// calcualte the latency in the exit handler
void BenchLocalListener::exit_handler() {
  auto second_cycle = util::BreakdownTimer::get_one_second_cycle();
  sleep(2);
#if 1

#if CS == 1
  ASSERT(workers_.size() > nthreads);
  int client_idx = nthreads;
#if SI_TX
  //client_idx += 1;
  //ASSERT(workers_.size() > (nthreads + 1));
#endif
  auto c = (BenchClient *)(workers_[client_idx]);
  auto &timer = c->timer_;
  LOG(3) << "previous " << timer.buffer.size() << " latency counted";
  timer.calculate_detailed();

  for(uint i = client_idx + 1;i < client_idx + nclients ;++i){
    ASSERT(i < workers_.size());
    auto cc = (BenchClient *)(workers_[i]);
    LOG(3) << "merge client: #" << i;
    cc->timer_.calculate_detailed();
    timer.merge(cc->timer_.buffer);
    LOG(3) << "merge client: #" << i << " done";
  }
#else
  auto &timer = workers_[0]->latency_timer_;
#endif

  //timer.dump();
  //timer.calculate_detailed(0.3);
  timer.sort();

  //LOG(3) << "total " << timer.buffer.size() << " latency counted";

  //#if 1
#if CS == 0 || RDTSC
  auto m_l = timer.report_medium() / second_cycle * 1000;
  auto m_9 = timer.report_90() / second_cycle * 1000;
  auto m_95 = timer.report_95() / second_cycle * 1000;
  auto m_99 = timer.report_99() / second_cycle * 1000;
  auto m_av = timer.report_avg() / second_cycle * 1000;
#else
  auto m_l = timer.report_medium() / 1000;
  auto m_9 = timer.report_90() / 1000;
  auto m_95 = timer.report_95() / 1000;
  auto m_99 = timer.report_99() / 1000;
  auto m_av = timer.report_avg() / 1000;
#endif

  LOG(4) << "Medium latency " << m_l << "ms, 90th latency " << m_9 << "ms, 99th latency "
         << m_99 << "ms; " << "95th latency: " << m_95 << "ms; average latency: " << m_av << "ms.";
#endif

#ifdef LOG_RESULTS
  if(log_file.is_open()) {
    log_file << reporter_->end() << " ";
    log_file << m_l << " " << m_9<<" " << m_99 <<" "<<m_av<<std::endl;
    log_file.close();
  }
#endif

  // really end the benchmark
  sleep(2);
  LOG(2) << "benchmark ends";
  reporter_->end();
}


} // namespace oltp

} // namespace nocc
 