#include "test_client.hpp"
#include "timer.h"
#include "macros.h"

#include <unistd.h>

int lock_mode = 0;

DB_DECLARE_int(id);

namespace db {

int client_id = 0;

static std::string get_logger_name(int id) {
  std::stringstream stream;
  stream << id << ".log";
  return stream.str();
}

TestClient::TestClient(int id,Ndb_cluster_connection *connect,
                       const std::string &sock,const std::string &api_host,
                       int epochs):
    id(id),
    ndb_mgm(connect),
    epochs(epochs),
    log_name(get_logger_name(id))
{
  RDMA_LOG(3) << "Client #" << id << " use " << api_host
              <<"; log to " << log_name << ".";
  client_id = id;
#if 0
  // init *mysql_handler*
  if ( !mysql_init(&mysql_handler) ) {
    RDMA_ASSERT(false) << "mysql_init failed\n";
  }
  if ( !mysql_real_connect(&mysql_handler, api_host.c_str(), "root", "password", "",
                           0, sock.c_str(), 0) )
    MYSQLERROR(mysql_handler);

  // switch to our database
  std::string db_stat = "USE "; db_stat.append(db_name);
  if (mysql_query(&mysql_handler, db_stat.c_str()) != 0)
    MYSQLERROR(mysql_handler);
#endif
  RDMA_LOG(3) << "Client #" << id << " init done.";
}

static void divide(std::vector<double> &v,int num) {
  for(uint i = 0;i < v.size();++i) {
    //v[i] =
  }
}

void TestClient::test_body(const workload_desc_vec_t &workloads, const std::string &database) {

  uint64_t done_reqs(0), abort_reqs(0);
  Timer t;

  RDMA_ASSERT(!workloads.empty())
      << "the workload is empty, nothing to execute";
  for (auto w : workloads) {
    RDMA_LOG(4) << w.to_str();
  }

  // connect ndb
  Ndb ndb(ndb_mgm, database.c_str());
  if (ndb.init()) APIERROR(ndb.getNdbError());

  // create a random variable
  FastRandom r(0xdeadbeaf + (id + 1) * 73);
  FastRandom rand(r.next());

  std::vector<double> res_per_tx(workloads.size(),0);

  Timer tt; uint64_t total_ops = 0;
  while(run_flag) {
    asm volatile ("" : : : "memory");

    // select a workload to execute
    double d = rand.next_uniform();
    uint idx = 0;
    for(size_t i = 0;i < workloads.size();++i) {
      if((i + 1) == workloads.size() || d < workloads[i].frequency) {
        idx = i;
        break;
      }
      d -= workloads[i].frequency;
    }
    auto res = workloads[idx].fn(ndb,rand);
    if(res.first == true) {
      done_reqs += 1;
      res_per_tx[idx] += 1;
    }
    else {
      FILE_LOG(log_name.c_str()) << "error abort TX!";
      RDMA_ASSERT(false);
      abort_reqs += 1;
    }

    // calculate the thpt
    auto passed_msec = t.passed_msec();
    if(passed_msec >= 1000000) {
      double thpt = ((double)done_reqs / passed_msec) * 1000000;
      RDMA_LOG(4) << "@" << results.size() <<  "Throughput " << thpt << " reqs / sec, "
                  << "total " << abort_reqs << " aborted" ;
      record_res(thpt);
      t.reset();
      total_ops += done_reqs;
      done_reqs = 0;
      abort_reqs = 0;
    }
  }
  RDMA_LOG(3) << "Client #" << id << " stop.";
  mysql_thread_end();
}

void TestClient::record_res(double res) {
  results.push_back(res);
  if(results.size() > epochs) {
    stop();
    sleep(id + 1);
    for(auto d : results) {
      FILE_LOG(log_name.c_str()) << d;
    }
    results.clear();
  }
}

void *MClient::run_body() {

  Ndb ndb(ndb_mgm, db_name.c_str());
  if (ndb.init()) APIERROR(ndb.getNdbError());

  FastRandom r(0xdeadbeaf + (::db::FLAGS_id + worker_id_) * 73);
  FastRandom rand(r.next());

  while(run_flag) {
    asm volatile ("" : : : "memory");

    // select a workload to execute
    double d = rand.next_uniform();
    uint idx = 0;
    for(size_t i = 0;i < workloads.size();++i) {
      if((i + 1) == workloads.size() || d < workloads[i].frequency) {
        idx = i;
        break;
      }
      d -= workloads[i].frequency;
    }
    auto res = workloads[idx].fn(ndb,rand);
    if(res.first == true) {
      ntxn_commits += 1;
    }
  }
}

};
 