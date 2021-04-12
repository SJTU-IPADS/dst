#pragma once

#include <string>

#include <mysql.h>
#include <mysqld_error.h>
#include <NdbApi.hpp>
#include <string.h>

#ifndef RDMA_DEBUG
#define RDMA_DEBUG
#endif

#include <logging.hpp>

namespace  db {

// Table & DB names used in tests
const std::string db_name = "test";
const std::string table_one = "tb1";

const int global_seed = 0xdeadbeaf;

// helper macros used in NdbExample
#define PRINT_ERROR(code,msg)                                       \
  std::cout << "Error in " << __FILE__ << ", line: " << __LINE__    \
  << ", code: " << code                                             \
  << ", msg: " << msg << "." << std::endl
#define MYSQLERROR(mysql) {                                 \
    PRINT_ERROR(mysql_errno(&mysql),mysql_error(&mysql));   \
    exit(-1); }
#define APIERROR(error) {                       \
    PRINT_ERROR(error.code,error.message);      \
    exit(-1); }

};
 