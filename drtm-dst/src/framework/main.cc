/*
 *  The code is part of our project called DrTM, which leverages HTM and RDMA for speedy distributed
 *  in-memory transactions.
 *
 *
 * Copyright (C) 2015 Institute of Parallel and Distributed Systems (IPADS), Shanghai Jiao Tong University
 *     All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  For more about this software, visit:  http://ipads.se.sjtu.edu.cn/drtm.html
 *
 */

#include <sys/resource.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <string>
#include <set>
#include <iterator>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "app/graph/graph.h"
#include "app/tpcc/tpcc_worker.h"
#include "app/tpce/tpce_worker.h"
#include "app/smallbank/bank_worker.h"
#include "app/micro_benches/bench_micro.h"

#include "util/spinlock.h"
#include "util/util.h"

#include "bench_runner.h"
#include "core/macros.h"

using namespace std;
using namespace nocc::util;

// benchmark parameters
extern size_t coroutine_num;
extern size_t nthreads;
extern size_t nclients;
extern size_t distributed_ratio;
extern size_t scale_factor;

// For distributed use
extern size_t total_partition;
extern size_t current_partition;
extern std::string config_file;
extern std::string host_file;

std::string exe_name;       // the executable's name
string bench_type = "tpcc"; // app name

int verbose = 0;
uint64_t txn_flags = 0;
uint64_t ops_per_worker = 0;
int enable_parallel_loading = false;
int pin_cpus = 0;
int slow_exit = 0;
int retry_aborted_transaction = 0;
int no_reset_counters = 0;
int backoff_aborted_transaction = 0;

DRTM_DEFINE_string(bench_type,"micro","Workload to run");
DRTM_DEFINE_string(host,"hosts.xml","Host file used");
DRTM_DEFINE_int(p,1,"Number of machines to run");
DRTM_DEFINE_int(scale,1,"The scale of memory store");
DRTM_DEFINE_int(client,1,"The number of client to use");
DRTM_DEFINE_int(id,0,"Mac ID to run");
DRTM_DEFINE_int(threads,1," Number of worker threads.");
DRTM_DEFINE_int(coro,1," Number of coroutines per threads");
DRTM_DEFINE_string(config,"config.xml","Config XML to run");
DRTM_DEFINE_int(tcp_port,6666,"Port of the binding");

extern int tcp_port;

static SpinLock exit_lock;

namespace nocc {

  extern volatile bool running; // running config variables

  // some helper functions
  static void printTraceExit(int sig) {
    nocc::util::print_stacktrace();
    running = false;
  }

  static void segfault_handler(int sig) {
    exit_lock.Lock();
    fprintf(stdout,"[NOCC] Meet a segmentation fault!\n");
    printTraceExit(sig);
    running = false;
    //exit_lock.Unlock();
    exit(-1);
  }

  static void segabort_handler(int sig) {
    exit_lock.Lock();
    fprintf(stdout,"[NOCC] Meet an assertion failure!\n");
    printTraceExit(sig);
    //exit_lock.Unlock();
    exit(-1);
  }

  static vector<string>
  split_ws(const string &s)
  {
    vector<string> r;
    istringstream iss(s);
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter<vector<string>>(r));
    return r;
  }

} // namespace nocc
int main(int argc, char **argv)
{

  gflags::SetUsageMessage("Usage : ./roccc* ");
  gflags::AllowCommandLineReparsing();
  gflags::ParseCommandLineFlags(&argc, &argv, false);

  exe_name = std::string(argv[0] + 2);
  void (*test_fn)( int argc, char **argv) = NULL;

  char time_buffer[80];
  {
    // calculate the run time for debug
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buffer,sizeof(time_buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
  }
  fprintf(stdout,"NOCC started with program [%s]. at %s\n",exe_name.c_str(),time_buffer);

  nocc::oltp::config_file_name = FLAGS_config;
  current_partition = FLAGS_id;
  total_partition   = FLAGS_p;
  scale_factor      = FLAGS_scale;
  bench_type        = FLAGS_bench_type;
  nthreads          = FLAGS_threads;
  coroutine_num     = FLAGS_coro;
  tcp_port          = FLAGS_tcp_port;
  host_file         = FLAGS_host;
  nclients          = FLAGS_client;

  LOG(3) << "use benchmark [" << bench_type << "], config file: " << nocc::oltp::config_file_name;

  if(bench_type == "tpcc") {
    test_fn = nocc::oltp::tpcc::TpccTest;
  } else if(bench_type == "tpce") {
    test_fn = nocc::oltp::tpce::TpceTest;
  } else if(bench_type == "micro") {
    test_fn = nocc::oltp::micro::MicroTest;
  } else if(bench_type == "bank") {
    test_fn = nocc::oltp::bank::BankTest;
  } else if(bench_type == "graph") {
    test_fn = nocc::oltp::link::GraphTest;
  } else{
    ASSERT(false) << "unknown benchmark name: " << bench_type;
  }

  /* install the event handler if necessary */
  signal(SIGSEGV, nocc::segfault_handler);
  signal(SIGABRT, nocc::segabort_handler);

  test_fn(argc, argv);
  return 0;
}
 