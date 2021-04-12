#include "rocc_config.h"
#include "tx_config.h"

#include "config.h"

#include "bench_worker.h"
#include "client.h"

#include "req_buf_allocator.h"

#include "db/txs/dbrad.h"
#include "db/txs/dbsi.h"

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

extern size_t coroutine_num;
extern size_t current_partition;
extern size_t nclients;
extern size_t nthreads;
extern int tcp_port;

namespace nocc {

__thread oltp::BenchWorker *worker = NULL;
__thread TXHandler   **txs_ = NULL;
__thread rtx::OCC      **new_txs_ = NULL;

extern uint64_t total_ring_sz;
extern uint64_t ring_padding;

std::vector<CommQueue *> conns; // connections between clients and servers
extern std::vector<SingleQueue *>   local_comm_queues;
extern zmq::context_t send_context;

namespace oltp {

// used to calculate benchmark information ////////////////////////////////
__thread std::vector<size_t> *txn_counts = NULL;
__thread std::vector<size_t> *txn_aborts = NULL;
__thread std::vector<size_t> *txn_remote_counts = NULL;


// used to calculate the latency of each workloads
__thread workload_desc_vec_t *workloads = nullptr;

extern char *rdma_buffer;

extern __thread util::fast_random *random_generator;

// per-thread memory allocator
__thread RPCMemAllocator *msg_buf_alloctors = NULL;
extern MemDB *backup_stores_[MAX_BACKUP_NUM];

SpinLock exit_lock;

BenchWorker::BenchWorker(unsigned worker_id,bool set_core,unsigned seed,uint64_t total_ops,
                         spin_barrier *barrier_a,spin_barrier *barrier_b,BenchRunner *context,
                         DBLogger *db_logger):
    RWorker(worker_id,cm,seed),
    initilized_(false),
    set_core_id_(set_core),
    ntxn_commits_(0),
    ntxn_aborts_(0),
    ntxn_executed_(0),
    ntxn_abort_ratio_(0),
    ntxn_remote_counts_(0),
    ntxn_strict_counts_(0),
    total_ops_(total_ops),
    context_(context),
    db_logger_(db_logger),
    // r-set some local members
    new_logger_(NULL)
{
  assert(cm_ != NULL);
  INIT_LAT_VARS(yield);
#if CS == 0
  nclients = 0;
  server_routine = coroutine_num;
#else
  if(nclients >= nthreads) server_routine = coroutine_num;
  //else server_routine = MAX(coroutine_num,server_routine);
  //server_routine = MAX(40,server_routine);
  //server_routine = 4;
#endif
  //server_routine = 4;
  server_routine = 80;
  LOG(4) << "use server routines: " << server_routine;
}

void BenchWorker::init_tx_ctx() {

  worker = this;
  txs_              = new TXHandler*[1 + server_routine + 2];
  //new_txs_          = new rtx::OCC*[1 + server_routine + 2];
  //std::fill_n(new_txs_,1 + server_routine + 2,static_cast<rtx::OCC*>(NULL));

  msg_buf_alloctors = new RPCMemAllocator[1 + server_routine];

  txn_counts = new std::vector<size_t> ();
  txn_aborts = new std::vector<size_t> ();
  txn_remote_counts = new std::vector<size_t> ();

  for(uint i = 0;i < NOCC_BENCH_MAX_TX;++i) {
    txn_counts->push_back(0);
    txn_aborts->push_back(0);
    txn_remote_counts->push_back(0);
  }
  for(uint i = 0;i < 1 + server_routine + 2;++i)
    txs_[i] = NULL;

  // init workloads
  workloads = new workload_desc_vec_t[server_routine + 2];
}

void BenchWorker::run() {

  ASSERT(inited == false);
  // create connections
  exit_lock.Lock();
  if(conns.size() == 0) {
    // only create once
    for(uint i = 0;i < nthreads + nclients + 3;++i)
      conns.push_back(new CommQueue(nthreads + nclients + 3));
  }
  exit_lock.Unlock();

  //  BindToCore(worker_id_); // really specified to platforms
  //binding(worker_id_);
  init_tx_ctx();
  init_routines(server_routine);

  //create_logger();

#if USE_RDMA
  init_rdma();
  create_qps();
#endif
#if USE_TCP_MSG == 1
  ASSERT(local_comm_queues.size() > 0 && local_comm_queues[worker_id_] != NULL)
      << "local comm queue size " << local_comm_queues.size()
      << "; queue ptr " << local_comm_queues[worker_id_] << " @" << worker_id_;

  create_tcp_connections(local_comm_queues[worker_id_],tcp_port,send_context);
#else
  MSGER_TYPE type;

#if USE_UD_MSG == 1
  type = UD_MSG;
#if LOCAL_CLIENT == 1 || CS == 0
  int total_connections = 1;
#else
  int total_connections = nthreads + nclients;
#endif
  create_rdma_ud_connections(total_connections);
#else
  if (worker_id_ == 0) {
    LOG(3) << "create rdma rc message\n";
  }
  create_rdma_rc_connections(rdma_buffer + HUGE_PAGE_SZ,
                             total_ring_sz,ring_padding);
#endif

#endif

  //this->init_new_logger(backup_stores_);
  this->thread_local_init();   // application specific init

  register_callbacks();

#if CS == 1
#if LOCAL_CLIENT == 0
  create_client_connections(nthreads + nclients);
#endif
  assert(pending_reqs_.empty());
  rpc_->register_callback(std::bind(&BenchWorker::req_rpc_handler,this,
                                    std::placeholders::_1,
                                    std::placeholders::_2,
                                    std::placeholders::_3,
                                    std::placeholders::_4),RPC_REQ);
#endif
  ASSERT(inited == false);
  // waiting for master to start workers
  this->inited = true;
#if 1
  while(!this->running) {
    asm volatile("" ::: "memory");
  }
#else
  this->running = true;
#endif
  // starts the new_master_routine
  start_routine(); // uses parent worker->start
}

void
__attribute__((optimize("O1"))) // this flag is very tricky, it should be set this way
BenchWorker::worker_routine(yield_func_t &yield) {

  assert(conns.size() != 0);
  //if(current_partition != 0) indirect_must_yield(yield);

  using namespace db;
  /* worker routine that is used to run transactions */
  workloads[cor_id_] = get_workload();
  auto &workload = workloads[cor_id_];

  // Used for OCC retry
  unsigned int backoff_shifts = 0;
  unsigned long abort_seed = 73;

  while(abort_seed == random_generator[cor_id_].get_seed()) {
    abort_seed += 1;         // avoids seed collision
  }

#if SI_TX
  //if(current_partition == 0) indirect_must_yield(yield);
#endif
  if(current_partition == total_partition - 1)
    indirect_must_yield(yield);

  uint64_t retry_count(0);
  while(true) {
#if CS == 0
    /* select the workload */
    double d = random_generator[cor_id_].next_uniform();

    uint tx_idx = 0;
    assert(workload.size() > 0);
    for(size_t i = 0;i < workload.size();++i) {
      if((i + 1) == workload.size() || d < workload[i].frequency) {
        //LOG(3) << "check frequency: " << workload[i].frequency;
        tx_idx = i;
        break;
      }
      d -= workload[i].frequency;
    }

#else
#if LOCAL_CLIENT
    REQ req;
    if(!conns[worker_id_]->front((char *)(&req))){
      yield_next(yield);
      continue;
    }
    conns[worker_id_]->pop();
#if RDTSC
    //req.start_time = rdtsc();
#endif
#else
    if(pending_reqs_.empty()){
      yield_next(yield);
      continue;
    }
    REQ req = pending_reqs_.front();
    pending_reqs_.pop();
#endif
    int tx_idx = req.tx_id;
#endif

#if CALCULATE_LAT == 1
    if(cor_id_ == 1) {
      // only profile the latency for cor 1
      //#if LATENCY == 1
      latency_timer_.start();
      //#else
      (workload[tx_idx].latency_timer).start();
      //#endif
    }
#endif
    const unsigned long old_seed = random_generator[cor_id_].get_seed();
    (*txn_counts)[tx_idx] += 1;
 abort_retry:
    ntxn_executed_ += 1;
    auto ret = workload[tx_idx].fn(this,yield);
    if(likely(ret.first)) {
      // commit case
      retry_count = 0;
#if CALCULATE_LAT == 1
      if(cor_id_ == 1) {
        //#if LATENCY == 1
        latency_timer_.end();
        //#else
        workload[tx_idx].latency_timer.end();
        //#endif
      }
#endif

#if CS == 0 // self_generated requests
      ntxn_commits_ += 1;

#if PROFILE_RW_SET == 1 || PROFILE_SERVER_NUM == 1
      if(ret.second > 0)
        workload[tx_idx].p.process_rw(ret.second);
#endif
#else // send reply to clients
      ntxn_commits_ += 1;
      if(ret.second > 0)
        workload[tx_idx].p.process_rw(ret.second);

      // reply to client
#if LOCAL_CLIENT
      uint64_t dummy[2];
      dummy[0] = req.start_time;
      dummy[1] = tx_idx;
      ASSERT(req.c_tid == nthreads);
      conns[req.c_tid]->enqueue(worker_id_,(char *)(&dummy),sizeof(uint64_t) * 2);
#else
      char *reply = rpc_->get_reply_buf();
      rpc_->send_reply(reply,sizeof(uint8_t),req.c_id,req.c_tid,req.cor_id,client_handler_);
#endif
#endif
    } else {
      // abort case
      retry_count += 1;
#if 0
      if(retry_count > 10000000)
        ASSERT(false) << "tx " << tx_idx << " abort too many times!";
#endif
      // abort case
      if(old_seed != abort_seed) {
        /* avoid too much calculation */
        ntxn_abort_ratio_ += 1;
        abort_seed = old_seed;
      }
      (*txn_aborts)[tx_idx] += 1;
      (*txn_counts)[tx_idx] += 1;

      ntxn_aborts_ += 1;
      yield_next(yield);

      // reset the old seed
      random_generator[cor_id_].set_seed(old_seed);
      goto abort_retry;
    }
    yield_next(yield);
    // end worker main loop
  }
}

void BenchWorker::exit_handler() {

  if( worker_id_ == 0 ){

    // only sample a few worker information
    auto &workload = workloads[1];

    auto second_cycle = BreakdownTimer::get_one_second_cycle();
#if 1
    //exit_lock.Lock();
    fprintf(stdout,"aborts: ");
    workload[0].latency_timer.calculate_detailed();
    workload[0].latency_timer.sort();
    fprintf(stdout,"%s ratio: %f , aborts %lu, executed %lu, latency %f, rw_size %f, m %f, 90 %f, 99 %f\n",
            workload[0].name.c_str(),
            (double)((*txn_aborts)[0]) / ((*txn_counts)[0] + ((*txn_counts)[0] == 0)),
            (*txn_aborts)[0],
            (*txn_counts)[0],workload[0].latency_timer.report() / second_cycle * 1000,
            workload[0].p.report(),
            workload[0].latency_timer.report_medium() / second_cycle * 1000,
            workload[0].latency_timer.report_90() / second_cycle * 1000,
            workload[0].latency_timer.report_99() / second_cycle * 1000);

    for(uint i = 1;i < workload.size();++i) {
      //workload[i].latency_timer.calculate_detailed();
      workload[i].latency_timer.sort();
      fprintf(stdout,"        %s ratio: %f ,aborts: %lu,executed %lu, latency: %f, rw_size %f, m %f, 90 %f, 99 %f\n",
              workload[i].name.c_str(),
              (double)((*txn_aborts)[i]) / ((*txn_counts)[i] + ((*txn_counts)[i] == 0)),
              (*txn_aborts)[i],
              (*txn_counts)[i],
              workload[i].latency_timer.report() / second_cycle * 1000,
              workload[i].p.report(),
              workload[i].latency_timer.report_medium() / second_cycle * 1000,
              workload[i].latency_timer.report_90() / second_cycle * 1000,
              workload[i].latency_timer.report_99() / second_cycle * 1000);
    }
    fprintf(stdout,"\n");

    fprintf(stdout,"total: ");
    for(uint i = 0;i < workload.size();++i) {
      fprintf(stdout," %d %lu; ",i, (*txn_counts)[i]);
    }
    fprintf(stdout,"succs ratio %f\n",(double)(ntxn_executed_) /
            (double)(ntxn_commits_));

    exit_report();
#endif
#if RECORD_STALE
    util::RecordsBuffer<double> total_buffer;
    for(uint i = 0; i < server_routine + 1;++i) {

      // only calculate staleness for timestamp based method
#if RAD_TX
      auto tx = dynamic_cast<DBRad *>(txs_[i]);
#elif SI_TX
      auto tx = dynamic_cast<DBSI *>(txs_[i]);
#else
      DBRad *tx = NULL;
#endif
      if(tx != NULL)
        total_buffer.add(tx->stale_time_buffer);
    }
    fprintf(stdout,"total %d points recorded\n",total_buffer.size());
    total_buffer.sort_buffer(0);
    std::vector<int> cdf_idx({1,5,10,15,20,25,30,35,40,45,
            50,55,60,65,70,75,80,81,82,83,84,85,90,95,
            97,98,99,100});
    total_buffer.dump_as_cdf("stale.res",cdf_idx);
#endif
    check_consistency();

    //exit_lock.Unlock();

    fprintf(stdout,"master routine exit...\n");
  }
  return;
}

/* Abstract bench loader */
BenchLoader::BenchLoader(unsigned long seed)
    : random_generator_(seed) {
  worker_id_ = 0; /**/
}

void BenchLoader::run() {
  load();
}

BenchClient::BenchClient(unsigned worker_id,unsigned seed)
    :RWorker(worker_id,cm,seed) {
  this->inited = false;
}

void BenchClient::run() {

  fprintf(stdout,"[Client %d] started\n",worker_id_);
  // client only support ud msg for communication
  //BindToCore(worker_id_);
#if USE_RDMA
  init_rdma();
#endif
  init_routines(coroutine_num);

#if CS == 1 && (LOCAL_CLIENT == 0)
  create_client_connections();
#endif
  this->inited = true;

  while(!this->running) {
    asm volatile("" ::: "memory");
  }

  running = true;
#if LOCAL_CLIENT
  worker_routine_local2();
#else
  start_routine();
#endif
  return;
}

std::vector<uint64_t> send_count (24);
std::vector<uint64_t> tx_count(24);
std::vector<util::BreakdownTimer> tx_timers(24);

bool BenchClient::send_one(uint64_t start) {

  uint8_t tx_id;
  auto node = get_workload((char *)(&tx_id),random_generator[0]);
  // choose a node to send
  auto thread = random_generator[0].next() % nthreads;

  auto s_time = rdtsc();
  pending_reqs.emplace(tx_id,0,worker_id_,thread,s_time);
  //ASSERT(false) << sizeof(BenchWorker::REQ);
  if (1) {
    auto p_req = pending_reqs.front();
#if RDTSC
    p_req.start_time = rdtsc();
#else
    p_req.start_time = start;
#endif
    ASSERT(p_req.cor_id < conns.size());
    send_count[p_req.cor_id] += 1;
    if(conns[p_req.cor_id]->enqueue(worker_id_,(char *)(&p_req),sizeof(BenchWorker::REQ))) {
      pending_reqs.pop();
      return true;
    } else {
      ASSERT(false);
      return false;
    }
  }
}


void BenchClient::worker_routine_local2() {
  // used for TPC-E RDMA
retry:
  exit_lock.Lock();
  asm volatile("" ::: "memory");
 if(conns.size() == 0) {
   exit_lock.Unlock();
   sleep(1);
   goto retry;
  }
 exit_lock.Unlock();

 uint64_t request_count = 0;
 uint64_t ack_count = 0;

 LOG(2) << "client #" << worker_id_ << " real started";

#define CLOSE 0

#if !CLOSE
 // wait for the server to wake up
 char res[64];
 send_one();
 while(!conns[worker_id_]->front(res)) {
   asm volatile("" ::: "memory");
 }
 conns[worker_id_]->pop();
#endif
  auto t1 = Clock::now();
  double cur_speed = coroutine_num / 1000.0;
  double last_elapsed = 0;

  while(running) {

    auto t = Clock::now();
    double elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(t - t1).count();
    uint64_t expected_count = std::floor((elapsed_ms / 1000.0) * coroutine_num);

#if CLOSE
    if(request_count == 0)
      for(uint i = 0;i < coroutine_num;++i)
        send_one(elapsed_ms);
#endif


    char res[64];
    while(running && (conns[worker_id_]->front(res))) {
      uint64_t s_time = *((uint64_t *)(res));
      uint64_t tx_idx = *((uint64_t *)(res + sizeof(uint64_t)));

      tx_count[tx_idx] += 1;

      conns[worker_id_]->pop();
#if RDTSC
      auto latency = rdtsc() - s_time;
#else
      auto latency = elapsed_ms - s_time;
#endif
      //LOG(3) << "dequeue: " << s_time << "; lat: " << latency;sleep(1);
      timer_.emplace(latency);
      tx_timers[tx_idx].emplace(latency);
#if CLOSE
      send_one(elapsed_ms);
#else
      if(request_count < expected_count)
        if(send_one(elapsed_ms))
          request_count += 1;
      ack_count += 1;
#endif
    }

    // now send the request
    while((request_count < expected_count) && ((request_count - ack_count) <= coroutine_num)){
      if(send_one(elapsed_ms))
        request_count += 1;
      else
        break;
    }
    // end while
  }
  for(uint i = 0;i < tx_count.size();++i) {
    LOG(2) << "tx received [" << i <<"]: " << tx_count[i];
    auto &timer = tx_timers[i];
    timer.sort();
    LOG(2) << timer.report_medium() << " " << timer.report_90() << " "
           << timer.report_95() << " " << timer.report_99() << " " << timer.report_avg();
  }
}



void BenchClient::worker_routine_local() {

retry:
  exit_lock.Lock();
  asm volatile("" ::: "memory");
 if(conns.size() == 0) {
   exit_lock.Unlock();
   sleep(1);
   goto retry;
  }
 exit_lock.Unlock();

 uint64_t request_count = 0;
 uint64_t ack_count = coroutine_num * 2;

 LOG(2) << "client #" << worker_id_ << " real started";
#if 1
 // wait for the server to wake up
 char res[64];
 send_one();
 while(!conns[worker_id_]->front(res)) {
   asm volatile("" ::: "memory");
 }
 conns[worker_id_]->pop();
#endif
#if 0
  for(uint i = 0;i < coroutine_num * 2;++i) {
    send_one();
  }
  request_count += coroutine_num;
#endif

  auto t1 = Clock::now();

  for(uint i = 0;i < coroutine_num;++i)
    send_one();

  while(running) {

    auto t = Clock::now();
    double elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(t - t1).count();
    uint64_t expected_count = std::floor((elapsed_ms / 1000.0) * coroutine_num);

    char res[64];
    while(running && (conns[worker_id_]->front(res))) {
      uint64_t s_time = *((uint64_t *)(res));
      uint64_t tx_idx = *((uint64_t *)(res + sizeof(uint64_t)));

      tx_count[tx_idx] += 1;

      conns[worker_id_]->pop();
#if RDTSC
      auto latency = rdtsc() - s_time;
#else
      auto latency = elapsed_ms - s_time;
#endif
      //LOG(3) << "dequeue: " << s_time << "; lat: " << latency;sleep(1);
      timer_.emplace(latency);
      tx_timers[tx_idx].emplace(latency);

      if(request_count < expected_count)
        if(send_one(elapsed_ms))
          request_count += 1;
      ack_count += 1;
    }
#if 1

    double ack_speed = (double)ack_count / elapsed_ms;
    //bool whether_overflow = (ack_speed * 1.08) > coroutine_num || (coroutine_num > 10);
    bool whether_overflow = (ack_speed * 1.08) < coroutine_num; // 1.08 is the number for TPC-C
#if 0
    if(expected_count - request_count > coroutine_num) {
      if(whether_overflow) {
        request_count += (expected_count - request_count) / 2;
        whether_overflow = false;
      }
    }
#endif
    //whether_overflow = false;
    if(coroutine_num == 1)
      whether_overflow = false;

    // refresh the clock and timer
    while(running && (request_count < expected_count) && (!whether_overflow)) {
      //while(running && ((request_count * request_per_ms) < elapsed_ms)) {
      //    while(running && (request_count < elapsed_ms)) {
      if(!send_one(elapsed_ms))
        break;
      request_count += 1;
    }
#if 1
    if(!whether_overflow) {
      if(coroutine_num == 1)
        request_count = expected_count;
      else {
        //ASSERT(request_count >= ack_count);
        //request_count += ((expected_count - ack_count) / 2);
      }
    }
#endif
    //request_count = expected_count;
#endif
    // end while
  }
  for(uint i = 0;i < tx_count.size();++i) {
    LOG(2) << "tx received [" << i <<"]: " << tx_count[i];
    auto &timer = tx_timers[i];
    timer.sort();
    LOG(2) << timer.report_medium() << " " << timer.report_90() << " "
           << timer.report_95() << " " << timer.report_99() << " " << timer.report_avg();
  }
}

void BenchClient::worker_routine(yield_func_t &yield) {

  char *req_buf = rpc_->get_static_buf(64);
  char reply_buf[64];
  while(true) {
    auto start = rdtsc();

    // prepare arg
    char *arg = (char *)req_buf;
    auto node = get_workload(arg + sizeof(uint8_t),random_generator[cor_id_]);
    *((uint8_t *)(arg)) = worker_id_;
    uint thread = random_generator[cor_id_].next() % nthreads;

    // send to server
    rpc_->prepare_multi_req(reply_buf,1,cor_id_);
    rpc_->append_req(req_buf,RPC_REQ,sizeof(uint64_t),cor_id_,RRpc::REQ,node,thread);

    // yield
    indirect_yield(yield);
    // get results back
    auto latency = rdtsc() - start;
    timer_.emplace(latency);
  }
}

void BenchClient::exit_handler() {
#if LOCAL_CLIENT == 0
  auto second_cycle = util::BreakdownTimer::get_one_second_cycle();
  auto m_av = timer_.report_avg() / second_cycle * 1000;
  //exit_lock.Lock();
  fprintf(stdout,"avg latency for client %d, %3f ms\n",worker_id_,m_av);
  //exit_lock.Unlock();
#endif
}

void BenchWorker::req_rpc_handler(int id,int cid,char *msg,void *arg) {
  uint8_t *input = (uint8_t *)msg;
  pending_reqs_.emplace(input[1],id,input[0],cid);
}



}; // oltp

}; // nocc
 