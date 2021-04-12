#include <stdlib.h>
// Used for cout
#include <stdio.h>
#include <iostream>

// for bind ctrl+c
#include <signal.h>

#include <unistd.h>

#include "micro_read_client.hpp"
#include "test_client.hpp"
#include "test_funcs.hpp"

#include "macros.h"

DB_DECLARE_int(name)

DB_DECLARE_int(id);
DB_DECLARE_string(manager);
DB_DECLARE_int(epoch);
DB_DECLARE_int(lock_mode);
DB_DECLARE_string(type);

DB_DEFINE_int(threads,1,"number of threads per client");

//using namespace db;

db::TestClient *client = nullptr;

void bind_sigint_handler();

extern NdbOperation::LockMode lock_mode;
bool sanity_checks = false;

int main(int argc, char** argv)
{
#if 0
  if (argc < 4)
  {
    std::cout << "Arguments are <id> <socket mysqld> <connect_string cluster> <type> <lock_mode> <run_epoch>.\n";
    exit(-1);
  }
#endif
  // ndb_init must be called first
  ndb_init();

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  int    tester_id = FLAGS_id;
  //char * mysqld_sock  = argv[2]
  const char *mysqld_sock = "";
  const char *connectstring = FLAGS_manager.c_str();
  int epoches = FLAGS_epoch;

  // benchmark type
  lock_mode = (NdbOperation::LockMode)(FLAGS_lock_mode);

  std::string type = FLAGS_type;

  RDMA_LOG(2) << "benchmark type [" << type << "], using " << epoches << " epoches";

  db::init_workloads();

  {
    /**
     * XD: For some unknown reason, the cluster connection should be first deallocated,
     * then the main thread can exit without reporting an error (although seems not so important).
     */

    // create the cluster connection
    Ndb_cluster_connection cluster_connection(connectstring);
    // make some connection to the cluster
    if (cluster_connection.connect(8 /* retries               */,
                                   1 /* delay between retries */,
                                   1 /* verbose               */))
    {
      RDMA_LOG(4) << "Cluster management server was not ready within 30 secs.\n";
      RDMA_ASSERT(false) << "connect cluster error";
    }

    // Optionally connect and wait for the storage nodes (ndbd's)
    if (cluster_connection.wait_until_ready(30,0) < 0)
    {
      RDMA_LOG(4) << "Cluster was not ready within 30 secs.\n";
      RDMA_ASSERT(false);
    }

#if 0
    db::TestClient c(tester_id,&cluster_connection,mysqld_sock,"192.168.15.104",epoches);
    client = &c;

    bind_sigint_handler();

    // bind the ctrl+c for close
    RDMA_ASSERT(db::supported_workloads.find(type) != db::supported_workloads.end())
        << " workload [" << type << "] not supported.";
    auto workloads = db::supported_workloads[type]();
    c.test_body(workloads,db::workload_database[type]);
#else
    auto workloads = db::supported_workloads[type]();

    //Ndb ndb(&cluster_connection, db::workload_database[type].c_str());
    //if (ndb.init()) APIERROR(ndb.getNdbError());

    std::vector<db::MClient *> workers;
    std::vector<uint64_t>     prev_results;
    for (uint i = 0;i < FLAGS_threads;++i) {
      auto w = new db::MClient(workloads,i,&cluster_connection,db::workload_database[type].c_str());
      w->start();
      workers.push_back(w);
      prev_results.push_back(0);
    }

    int local_epoch = 0;
    db::Timer t;

    std::vector<double> results;

    while(local_epoch < epoches + 10) {
      sleep(1);
      local_epoch += 1;

      // count results
      uint64_t sum = 0;
      for(auto w : workers) {
        auto temp = w->ntxn_commits;
        sum += (temp - prev_results[w->worker_id_]);
        prev_results[w->worker_id_] = temp;
      }
      auto passed_msec = t.passed_msec();
      double res = ((double)sum / passed_msec) * 1000000;
      RDMA_LOG(3) << "get res: " << res << " reqs/sec";
      results.push_back(res);
      t.reset();
    }

    // end
    for(auto w : workers) {
      w->run_flag = false;
    }

    for(auto w : workers) {
      w->join();
    }

    std::string s = std::to_string(tester_id);
    s.append(".log");
    for (auto r : results) {
      FILE_LOG(s.c_str()) << r;
    }
    RDMA_LOG(3) << "all workers stopped";
#endif
  }

  ndb_end(0);

  return 0;
}

void my_handler(int s){
  if(client != nullptr)
    client->stop();
}

void bind_sigint_handler() {

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, nullptr);

}
 