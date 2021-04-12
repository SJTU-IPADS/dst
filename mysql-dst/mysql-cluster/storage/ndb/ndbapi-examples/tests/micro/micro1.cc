#include "micro1.hpp"
#include "util.hpp"

#include <unistd.h>
#include <math.h>
#include <algorithm>

#include <ctime>
#include <iomanip>
#include <chrono>

#include "macros.h"

#define OPT 1

#if !OPT
#define LOCK_MODE (NdbOperation::LM_Read) // 0
#else
#define LOCK_MODE (NdbOperation::LM_SimpleRead) // 3
#endif

#define OPTIMAL 0
#if OPTIMAL
#undef LOCK_MODE
#define LOCK_MODE (NdbOperation::LM_CommittedRead) // 2
#endif

DB_DECLARE_int(id);
DB_DEFINE_int(ms_gap,0,"offset of the timestamp allocation");

static std::string get_logger_name() {
  std::stringstream stream;
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  stream << FLAGS_id << "_" << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << ".log";
  return stream.str();
}

extern NdbOperation::LockMode lock_mode;

DB_DEFINE_int(read_rounds,1,"number of rounds for read-only TX");
DB_DEFINE_int(contention_ratio,10,"contention ratio of the workloads, must be in [10,100]");

namespace db {

extern int client_id;

static const int deadlock_code = 266;

namespace micro1 {

int ratio = 10; // 10% contention ratio

void parse_config() {
  RDMA_LOG(4) << "use lock mode " << lock_mode;
}

static uint64_t get_timestamp(bool read = false) {
  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  auto epoch = now_ms.time_since_epoch();
  auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
  auto timestamp = value.count();
  assert(FLAGS_ms_gap < timestamp);
  if(read)
    timestamp -= FLAGS_ms_gap;
  //timestamp = timestamp << 12;
  //assert((timestamp >> 12) == value.count());

  //std::time_t t = std::time(0);  // t is an integer type
  return timestamp;
}

static txn_result_t tx_test1(Ndb &myNdb,FastRandom &rand) {

  NdbTransaction *myTransaction = myNdb.startTransaction();
  if (myTransaction == NULL) APIERROR(myNdb.getNdbError());

  if(lock_mode == 3) {
    myTransaction->timestamp = 0;
  }

  const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();
  const NdbDictionary::Table *myTable= myDict->getTable(::db::micro1::table_name.c_str());
  RDMA_ASSERT(myTable != nullptr);
  NdbOperation *op = myTransaction->getNdbOperation(myTable);
  op->readTuple(lock_mode);
  op->equal("ID",0);

  if(lock_mode == 3) {
    RDMA_LOG(3) << "original ts :" << myTransaction->timestamp;
  }

  if( myTransaction->execute( NdbTransaction::NoCommit, NdbOperation::AbortOnError ) == -1 ) {
    //APIERROR(myTransaction->getNdbError());
    if(myTransaction->getNdbError().code == deadlock_code) {
      myNdb.closeTransaction(myTransaction);
      return txn_result_t(false,0);
    } else
      APIERROR(myTransaction->getNdbError());
  }
  RDMA_LOG(3) << "pre execute return with ts " << myTransaction->timestamp;
  std::string dummy;
  std::cin >> dummy;

  if(1) {
    if( myTransaction->execute( NdbTransaction::Commit, NdbOperation::AbortOnError ) == -1 ) {
      //APIERROR(myTransaction->getNdbError());
      if(myTransaction->getNdbError().code == deadlock_code) {
        myNdb.closeTransaction(myTransaction);
        RDMA_ASSERT(false);
        return txn_result_t(false,0);
      } else
        APIERROR(myTransaction->getNdbError());
    }
    myNdb.closeTransaction(myTransaction);
  }
  RDMA_LOG(4) << "commit done";
  exit(-1);
}

static txn_result_t tx_test(Ndb &myNdb,FastRandom &rand) {

  NdbTransaction *myTransaction = myNdb.startTransaction();
  if (myTransaction == NULL) APIERROR(myNdb.getNdbError());

  const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();
  const NdbDictionary::Table *myTable= myDict->getTable(::db::micro1::table_name.c_str());
  RDMA_ASSERT(myTable != nullptr);

  static int space = floor(((double)ratio / 100.0) * 10000);

  // sort to avoid deadlock
  std::vector<int> keys;

  for(uint i = 0;i < 6;++i) {
    // do the first (conflicting) update
    int key = rand.next() % space;
    keys.push_back(key);

    // do the next 5 non conflicting update
    for (uint i = 0;i < 5;++i) {
      int key = rand.next() % 10000 + 10000;
      keys.push_back(key);
    }
  }

  // then sort the key
  std::sort(keys.begin(),keys.end());
  //RDMA_LOG(3) << util::vector_to_str(keys); sleep(1);

  for(auto k : keys) {
    NdbOperation *op = myTransaction->getNdbOperation(myTable);
    op->updateTuple();
    op->equal("ID",k);
    op->setValue("ATTR",73);

    //RDMA_LOG(3) << "write key " << k;
#if 0
    if( myTransaction->execute( NdbTransaction::NoCommit, NdbOperation::AbortOnError ) == -1 ) {
      //APIERROR(myTransaction->getNdbError());
      if(myTransaction->getNdbError().code == deadlock_code) {
        myNdb.closeTransaction(myTransaction);
        return txn_result_t(false,0);
      } else
        APIERROR(myTransaction->getNdbError());
    }
#endif
    //std::string dummy;
    //std::cin >> dummy;
  }

  if( myTransaction->execute( NdbTransaction::Commit, NdbOperation::AbortOnError ) == -1 ) {
    //APIERROR(myTransaction->getNdbError());
    if(myTransaction->getNdbError().code == deadlock_code) {
      myNdb.closeTransaction(myTransaction);
      return txn_result_t(false,0);
    } else
      APIERROR(myTransaction->getNdbError());
  }

  myNdb.closeTransaction(myTransaction);
  return txn_result_t(true,0);
}

workload_desc_vec_t generate_test_funcs() {
  parse_config();
  auto ret = workload_desc_vec_t();
  ret.emplace_back("update",TX1_update_ratio,TX1_UPDATE);
  ret.emplace_back("read",TX2_read_ratio,TX2_READ);
  //ret.emplace_back("read",1.0f,tx_test1);
  return ret;
}

// real implementations
txn_result_t TX1_UPDATE(Ndb &myNdb,FastRandom &rand) {

  const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();
  const NdbDictionary::Table *myTable= myDict->getTable(::db::micro1::table_name.c_str());
  RDMA_ASSERT(myTable != nullptr) << "get my table error!";

  static int space = floor(((double)FLAGS_contention_ratio / 100.0) * 10000);

  NdbTransaction *myTransaction = myNdb.startTransaction();
  myTransaction->timestamp = get_timestamp(false);
  //RDMA_LOG(3) << "get write ts: " << myTransaction->timestamp; sleep(1);

  // sort to avoid deadlock
  std::vector<int> keys;

  for(uint i = 0;i < 6;++i) {
    // do the first (conflicting) update
    int key = rand.next() % space;
    keys.push_back(key);

    // do the next 5 non conflicting update
    for (uint i = 0;i < 5;++i) {
      int key = rand.next() % 10000 + 20000;
      keys.push_back(key);
    }
  }

  // then sort the key
  std::sort(keys.begin(),keys.end());
  //RDMA_LOG(3) << util::vector_to_str(keys); sleep(1);

  for(auto k : keys) {
    NdbOperation *op = myTransaction->getNdbOperation(myTable);
    op->updateTuple();
    op->equal("ID",k);
    op->setValue("ATTR",73);
    RDMA_ASSERT(myTransaction->timestamp != 0);
#if 1 // XD: This pre-execute is the key to avoid dead lock
    if( myTransaction->execute( NdbTransaction::NoCommit, NdbOperation::AbortOnError ) == -1 ) {
      //APIERROR(myTransaction->getNdbError());
      if(myTransaction->getNdbError().code == deadlock_code) {
        myNdb.closeTransaction(myTransaction);
        return txn_result_t(false,0);
      } else {
        auto error = myTransaction->getNdbError();
        FILE_LOG(get_logger_name().c_str()) << error.message;
        APIERROR(myTransaction->getNdbError());
      }
    }
#endif
  }

  if( myTransaction->execute( NdbTransaction::Commit, NdbOperation::AbortOnError ) == -1 ) {
    //APIERROR(myTransaction->getNdbError());
    if(myTransaction->getNdbError().code == deadlock_code) {
      myNdb.closeTransaction(myTransaction);
      return txn_result_t(false,0);
    } else {
      auto error = myTransaction->getNdbError();
      FILE_LOG(get_logger_name().c_str()) << error.message;
      APIERROR(myTransaction->getNdbError());
    }
  }

  myNdb.closeTransaction(myTransaction);

  return txn_result_t(true,0);
}

txn_result_t TX2_READ(Ndb &myNdb,FastRandom &rand) {

  const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();
  const NdbDictionary::Table *myTable= myDict->getTable(::db::micro1::table_name.c_str());
  RDMA_ASSERT(myTable != nullptr) << "get my table error!";

  static int space = floor(((double)FLAGS_contention_ratio / 100.0) * 10000);

  NdbTransaction *myTransaction = myNdb.startTransaction();
  myTransaction->timestamp = get_timestamp(true);
  //myTransaction->timestamp = 0; // re-set the ts
  //RDMA_LOG(3) << "get read ts: " << myTransaction->timestamp; sleep(1);

  std::vector<int> keys;

  for(uint i = 0;i < FLAGS_read_rounds;++i) {
    // do the first (conflicting) update
    int key = rand.next() % space;
    keys.push_back(key);

    // do the next 5 non conflicting update
    for (uint i = 0;i < 5;++i) {
      int key = rand.next() % 10000 + 10000;
      keys.push_back(key);
    }
  }

  // then sort the key
  std::sort(keys.begin(),keys.end());
  //RDMA_LOG(3) << util::vector_to_str(keys); sleep(1);

  for(auto k : keys) {
    NdbOperation *op = myTransaction->getNdbOperation(myTable);
    op->readTuple(lock_mode);
    op->equal("ID",k);

#if 1 // XD: This pre-execute is the key to avoid
    if( myTransaction->execute( NdbTransaction::NoCommit, NdbOperation::AbortOnError ) == -1 ) {
      //APIERROR(myTransaction->getNdbError());
      if(myTransaction->getNdbError().code == deadlock_code) {
        myNdb.closeTransaction(myTransaction);
        return txn_result_t(false,0);
      } else {
        auto error = myTransaction->getNdbError();
        FILE_LOG(get_logger_name().c_str()) << error.message;
        APIERROR(myTransaction->getNdbError());
      }
    }
#endif
  }
  //if (lock_mode == 0 || lock_mode == 1) {
    // only in these two modes phase we need to release the lock
    if( myTransaction->execute( NdbTransaction::Commit ) == -1 ) {

      if(myTransaction->getNdbError().code == deadlock_code) {
        myNdb.closeTransaction(myTransaction);
        return txn_result_t(false,0);
      } else {
        auto error = myTransaction->getNdbError();
        FILE_LOG(get_logger_name().c_str()) << error.message;
        APIERROR(myTransaction->getNdbError());
      }
    }
    //}
  myNdb.closeTransaction(myTransaction);

  return txn_result_t(true,0);
}

} // end namespace micro1

} // end namespace db
 