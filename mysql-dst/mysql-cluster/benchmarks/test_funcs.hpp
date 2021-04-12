#pragma once

#include "tx_workload.hpp"

#include <map>

namespace  db {

typedef std::function<workload_desc_vec_t () > workload_generator_f;

extern std::map<std::string,workload_generator_f> supported_workloads;
// the used database name for the supported workload
extern std::map<std::string,std::string>          workload_database;

void init_workloads();

}
 