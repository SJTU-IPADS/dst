#pragma once

#include "tx_workload.hpp"

namespace db {

namespace bank {

#define HIGH_CONTENTION 1

// some DB contants for this benchmark
const std::string db_name = "bank";
const std::string check_table = "checking";
const std::string save_table = "saving";

/**
 * settings for TX database scale
 */
// Original paper: The Cost of Serializability on Platforms That Use Snapshot Isolation
//const int num_accounts = 100000; // this number is from H-store
const int num_accounts = 18000;    // this number is from the original paper

/**
 * setting for TX workloads
 */
#if HIGH_CONTENTION == 0
const int hot_nums = 1000; // this number is from the original paper
#else
const int hot_nums = 10;  // evaluate the high contention setting, described in the original paper
#endif
//const int hot_nums     = 4000; // also, this number is from H-store
const int hot_ratio =  90; // 90\% of the TX access the hot records defined above

/**
 * Settings for specific accounts
 */
const int min_balance = 10000;
const int max_balance = 50000;

/**
 * settings for workloadsy
 */
#if HIGH_CONTENTION == 0
enum RATIO {
  SEND_PAYMENT = 25,
  DEPOSIT_CHECKING = 15,
  BALANCE = 15,
  TRANSACT_SAVING = 15,
  WRITE_CHECK = 15,
  AMAL = 15
};
#else
enum RATIO {
  SEND_PAYMENT = 8,
  DEPOSIT_CHECKING = 8,
  BALANCE = 60,
  TRANSACT_SAVING = 8,
  WRITE_CHECK = 8,
  AMAL = 8
};
#endif

workload_desc_vec_t generate_test_funcs();

void check_consistency(Ndb &ndb,FastRandom &rand);

} // namespace smallbank

} // end namespace db
 