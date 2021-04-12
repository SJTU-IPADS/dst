#pragma once
/**
 * This is the first micro-benchmark we used.
 * It aims at
 */

#include "tx_workload.hpp"

namespace  db {

namespace micro1 {

// some DB contants for this benchmark
const std::string db_name = "micro";
const std::string table_name = "micro1";

const int total_records = 30000;
//const int total_records = 1; // for debug only
/**
 * This micro database has 3000 records in total, partition in the following manner
 * [ 0 - 1000        |     1000 - 2000         |     2000 - 3000 )
 * contention area     exclusive usage of TX1    exclusive usage of TX2
 * Contention area means that the 2 TX (defined below) may have conflict accesses for records in (0-1000).
 */

// TX execution ratio. We benchmark 2 TXs in total.
const double TX1_update_ratio = 0.5f;
const double TX2_read_ratio = 0.5f;

/**
 * The following is the main descriptions of this benchmark, which has 2 class of TX.
 * Each TX performance 6 group of operations, each group has 1 + 5 operation.
 * The first 1 operations aims at measuing contention, which access records in the contention area.
 * The rest 5 operations access non conflicting records, which resides in each TX's exclusive area.
 */

/**
 * Why design such a micro-benchmark? The workload actually is inspired from Callas[SOSP'15]'s microbenchmark.
 * The difference is that both TXs in Callas are update TX.
 */

txn_result_t TX1_UPDATE(Ndb &myNdb,FastRandom &);

txn_result_t TX2_READ(Ndb &myNdb,FastRandom &);

// parse the config from micro1.config
void parse_config();

workload_desc_vec_t generate_test_funcs();

} // end namespace micro1

} // end namespace db
 