#pragma once

#include "random.hpp"
#include "timer.h"
#include "tx_workload.hpp"
#include "thread.h"

namespace  db {

const int MAX_EPOCHS = 60;

class TestClient {
 public:
  /**
   * id:       identifier of the client. Should be *unique* in the global setting!
   * connect:  ndb connect handler. should be a global instance
   * sock:     the socket used at API node's mysql server
   * api_host: the IP of the API nodes.
   */
  TestClient(int id,Ndb_cluster_connection *connect,
             const std::string &sock,const std::string &api_host = "localhost",
             int epoch = MAX_EPOCHS); // default run for 60 seconds

  void stop() {
    run_flag = false;
  }

  void test_body(const workload_desc_vec_t &workloads,const std::string &db_name);

 private:
  const int id;
  std::string log_name;

  // handlers in Ndb and MySQL API
  Ndb_cluster_connection *ndb_mgm = nullptr;
  MYSQL mysql_handler;

  std::vector<double> results;
  const int epochs;

  void record_res(double res);

  volatile bool run_flag = true;
};

class MClient : public r2::Thread {
 public:
  MClient(const workload_desc_vec_t &workloads, int wid, Ndb_cluster_connection *ndb_mgm,const std::string &db):
      ndb_mgm(ndb_mgm),
      db_name(db),
      worker_id_(wid),
      workloads(workloads) {
  }
  void *run_body() override ;

  uint64_t ntxn_commits = 0;
  volatile bool run_flag = true;
  Ndb *ndb_;
  const int  worker_id_;
  workload_desc_vec_t workloads;
  Ndb_cluster_connection *ndb_mgm = nullptr;
  const std::string       db_name;
};

} // namespace db
 