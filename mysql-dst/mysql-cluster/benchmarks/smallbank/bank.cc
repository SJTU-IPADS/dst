#include "bank.hpp"
#include "db_helper.hpp"

#include <algorithm>

extern NdbOperation::LockMode lock_mode;
extern bool sanity_checks;

namespace db {

namespace bank {

static inline void generate_id(FastRandom &r,uint64_t &id) {
  uint64_t num_global(0);
  if(r.next() % 100 < hot_ratio) {
    num_global = hot_nums;
  } else {
    num_global = num_accounts;
  }
  id = r.next() % num_global;
}

static inline void generate_two_id(FastRandom &rand,uint64_t &id,uint64_t &id1) {
  do {
    generate_id(rand, id);
    generate_id(rand, id1);
  } while(id == id1);
  if(id > id1)
    std::swap(id,id1);
  RDMA_ASSERT(id < id1);
}

inline bool double_equal(double a,double b) {
  return abs(a-b) <= 0.0001;
}

static bool sanity_check_dc(Ndb &ndb,uint64_t id,double expected_res);
static bool sanity_check_ts(Ndb &ndb,uint64_t id,double expected_res);
static bool sanity_check_sd(Ndb &ndb,uint64_t id,uint64_t id1,double expected_res);


/***
 * The TX lock order:
 * - check id0, id1, ...
 * - save  id0, id1, ...
 */
static txn_result_t send_payment(Ndb &ndb,FastRandom &rand) {

  /**
   * @input: id0, id1
   * id0.check -= amount
   * id1.check += amount
   */
  uint64_t id0,id1;
  generate_two_id(rand, id0, id1);

  float amount = 5.0;

  const NdbDictionary::Dictionary* myDict= ndb.getDictionary();
  const NdbDictionary::Table *check = myDict->getTable(::db::bank::check_table.c_str());
  const NdbDictionary::Table *save = myDict->getTable(::db::bank::save_table.c_str());

  double res = 0.0;
  // TX start
  auto tx = tx_start(ndb);
  {
    auto op = read_tuple(tx, check, id0, NdbOperation::LM_Exclusive);
    auto col = read_col(op,"BALANCE");

    if(!pre_execute(tx))
      goto ABORT_SP;

    auto op1 = read_tuple(tx, check, id1, NdbOperation::LM_Exclusive);
    auto col1 = read_col(op1,"BALANCE");

    if(!pre_execute(tx))
      goto ABORT_SP;

    auto balance0 = col->double_value();
    auto balance1 = col1->double_value();
    res = balance0 + balance1;

    // now we write these values back
    write_tuple<double>(tx, check, id0, "BALANCE", balance0 - amount);
    write_tuple<double>(tx, check, id1, "BALANCE", balance1 + amount);

    if (sanity_checks) {
      RDMA_LOG(3) << "change " << id0 << "\'s balance from " << balance0 << " to " << balance0 - amount;
      RDMA_LOG(3) << "change " << id1 << "\'s balance from " << balance1 << " to " << balance1 + amount;
    }

    if(! commit(tx))
      goto ABORT_SP;
  }
  if(sanity_checks) {
    return txn_result_t(sanity_check_sd(ndb, id0, id1, res),0);
  }
  ndb.closeTransaction(tx);
  return txn_result_t(true,0);

ABORT_SP:
  ndb.closeTransaction(tx);
  return txn_result_t(false,0);
}

static txn_result_t deposit_checking(Ndb &ndb,FastRandom &rand) {
  /**
   * @input: id
   * id.check.balance += amount;
   */

  float amount = 1.3;
  uint64_t id; generate_id(rand, id);

  const NdbDictionary::Dictionary* myDict= ndb.getDictionary();
  const NdbDictionary::Table *check = myDict->getTable(::db::bank::check_table.c_str());

  auto tx = tx_start(ndb);

  auto op = read_tuple(tx, check, id, NdbOperation::LM_Exclusive);
  auto col = read_col(op,"BALANCE");

  if(!pre_execute(tx))
    RDMA_ASSERT(false); // this TX cannot involved in deadlock

  auto val = col->double_value();
  write_tuple<double>(tx, check, id, "BALANCE", val + amount);

  //if(!commit(tx))
  //    RDMA_ASSERT(false);

  ndb.closeTransaction(tx);

  if(sanity_checks) {
    RDMA_LOG(4) << "sanity check deposit checking...";
    bool res = sanity_check_dc(ndb, id, val + amount);
    return txn_result_t(res,0);
  }
  return txn_result_t(true,0);
}

static txn_result_t balance(Ndb &ndb,FastRandom &rand) {

  /**
   * return id.save.balance + id.check.balance;
   */
  const NdbDictionary::Dictionary* myDict= ndb.getDictionary();
  const NdbDictionary::Table *check = myDict->getTable(::db::bank::check_table.c_str());
  const NdbDictionary::Table *save = myDict->getTable(::db::bank::save_table.c_str());

  double res = 0;

  uint64_t id; generate_id(rand, id);
  auto tx = tx_start(ndb);
  //tx->timestamp = 0;
  {
    auto op = read_tuple(tx, check, id, lock_mode);
    auto col = read_col(op,"BALANCE");

    if(!pre_execute(tx))
      goto ABORT_BALANCE;

    auto op1 = read_tuple(tx, save, id, lock_mode);
    auto col1 = read_col(op1,"BALANCE");

    if(!commit(tx))
      goto ABORT_BALANCE;
    res = col->double_value() + col1->double_value();
  }
  ndb.closeTransaction(tx);
  return txn_result_t(true,res);

ABORT_BALANCE:
  ndb.closeTransaction(tx);
  return txn_result_t(false,0);
}

static txn_result_t transact_saving(Ndb &ndb,FastRandom &rand) {
  /**
   * @input: id
   * id.save.balance += amount;
   */
  const NdbDictionary::Dictionary* myDict= ndb.getDictionary();
  const NdbDictionary::Table *save = myDict->getTable(::db::bank::save_table.c_str());

  // inputs
  float amount   = 20.20; //from original code
  uint64_t id; generate_id(rand, id);

  auto tx = tx_start(ndb);
  auto op = read_tuple(tx, save, id, NdbOperation::LM_Exclusive);
  auto col = read_col(op, "BALANCE");

  if(!pre_execute(tx))
    RDMA_ASSERT(false);

  auto val = col->double_value();
  write_tuple<double>(tx, save, id, "BALANCE", val + amount);

  if(!commit(tx))
    RDMA_ASSERT(false);

  ndb.closeTransaction(tx);

  if(sanity_checks) {
    auto res = sanity_check_ts(ndb,id,val + amount);
    return txn_result_t(res,0);
  }

  return txn_result_t(true,0);
}

static txn_result_t write_check(Ndb &ndb,FastRandom &rand) {
  /**
   * @input: id
   * if(id.check.balance + id.save.balance > amount)
   *   id.check.balance -= amount;
   * else
   *   id.check.balance -= (amount - 1)
   */
  float amount = 5.0;

  uint64_t id;   generate_id(rand, id);

  const NdbDictionary::Dictionary* myDict= ndb.getDictionary();
  const NdbDictionary::Table *check = myDict->getTable(::db::bank::check_table.c_str());
  const NdbDictionary::Table *save = myDict->getTable(::db::bank::save_table.c_str());
  auto tx = tx_start(ndb);
  {
    auto op = read_tuple(tx, check, id, NdbOperation::LM_Exclusive);
    auto col = read_col(op,"BALANCE");

    if(!pre_execute(tx))
      goto ABORT_WC;

    auto op1 = read_tuple(tx, save, id, NdbOperation::LM_Exclusive);
    auto col1 = read_col(op1,"BALANCE");

    if(!pre_execute(tx))
      goto ABORT_WC;

    auto total = col->double_value() + col1->double_value();
    if(total < amount) {
      amount -= 1;
    }
    write_tuple<double>(tx, check, id, "BALANCE", col->double_value() - amount);

    if(!commit(tx))
      goto ABORT_WC;
  }
  ndb.closeTransaction(tx);
  return txn_result_t(true,0);
ABORT_WC:
  ndb.closeTransaction(tx);
  return txn_result_t(false,0);
}

static txn_result_t amal(Ndb &ndb,FastRandom &rand) {

  const NdbDictionary::Dictionary* myDict= ndb.getDictionary();
  const NdbDictionary::Table *check = myDict->getTable(::db::bank::check_table.c_str());
  const NdbDictionary::Table *save = myDict->getTable(::db::bank::save_table.c_str());
  /**
   * @input id0,id1
   * c0 = id0.check.balance
   * s0 = id0.save.balance
   *
   * total = c0 + s0
   *
   * id0.save.balance  = 0
   * id0.check.balance = 0
   *
   * id1.check.balance += total
   *
   */

  uint64_t id0,id1; generate_two_id(rand, id0, id1);
  auto tx = tx_start(ndb);
  {
    auto opc = read_tuple(tx, check, id0, NdbOperation::LM_Exclusive);
    auto colc = read_col(opc, "BALANCE");

    if(!pre_execute(tx))
      goto AMAL_ABORT;

    auto ops = read_tuple(tx, save, id0, NdbOperation::LM_Exclusive);
    auto cols = read_col(ops,"BALANCE");

    if(!pre_execute(tx))
      goto AMAL_ABORT;

    auto opc1 = read_tuple(tx, check, id1, NdbOperation::LM_Exclusive);
    auto colc1 = read_col(opc1,"BALANCE");

    if(!pre_execute(tx))
      goto AMAL_ABORT;

    auto total = colc->double_value() + cols->double_value();

    write_tuple<double>(tx, check, id0, "BALANCE", 0);
    write_tuple<double>(tx, save, id0, "BALANCE", 0);
    write_tuple<double>(tx, check, id1, "BALANCE", total + colc1->double_value());

    if(!commit(tx))
      goto AMAL_ABORT;
  }
  ndb.closeTransaction(tx);
  return txn_result_t(true,0);
AMAL_ABORT:
  ndb.closeTransaction(tx);
  return txn_result_t(false,0);
}

workload_desc_vec_t generate_test_funcs() {

  RDMA_LOG(4) << "Bank use setting: high-contention=" << HIGH_CONTENTION;
  auto ret = workload_desc_vec_t();
  RDMA_ASSERT(!sanity_checks);
#if 1
  ret.emplace_back("send_payment",SEND_PAYMENT / 100.0,send_payment);
  ret.emplace_back("deposit_checking",DEPOSIT_CHECKING / 100.0,deposit_checking);
  ret.emplace_back("balance",BALANCE / 100.0,balance);
  ret.emplace_back("write_check",WRITE_CHECK / 100.0,write_check);
  ret.emplace_back("transact_saving",TRANSACT_SAVING / 100.0,transact_saving);
  ret.emplace_back("amal",AMAL / 100.0,amal);
#else
  ret.emplace_back("deposit_checking",0.5,amal);
  ret.emplace_back("deposit_checking",0.5,balance);
#endif
  return ret;
}

// below are test functions
static bool sanity_check_dc(Ndb &ndb,uint64_t id,double expected_value) {

  const NdbDictionary::Dictionary* myDict= ndb.getDictionary();
  const NdbDictionary::Table *check = myDict->getTable(::db::bank::check_table.c_str());

  auto tx = tx_start(ndb);
  auto op  = read_tuple(tx, check, id, NdbOperation::LM_Exclusive);
  auto col = read_col(op,"BALANCE");
  RDMA_ASSERT(commit(tx));
  bool ret = double_equal(col->double_value(),expected_value);

  RDMA_LOG(3) << "sanity check ID: " << id << " value " << expected_value;
  ndb.closeTransaction(tx);
  return ret;
}

static bool sanity_check_ts(Ndb &ndb,uint64_t id,double expected_value) {

  const NdbDictionary::Dictionary* myDict= ndb.getDictionary();
  const NdbDictionary::Table *save = myDict->getTable(::db::bank::save_table.c_str());

  auto tx = tx_start(ndb);
  auto op  = read_tuple(tx, save, id, NdbOperation::LM_Exclusive);
  auto col = read_col(op,"BALANCE");
  RDMA_ASSERT(commit(tx));

  bool ret = double_equal(col->double_value(),expected_value);

  ndb.closeTransaction(tx);

  RDMA_LOG(3) << "sanity check ID: " << id << " expected value " << expected_value << " ; original value: " << col->double_value();
  return ret;
}

static bool sanity_check_sd(Ndb &ndb,uint64_t id0,uint64_t id1,double expected_value) {

  const NdbDictionary::Dictionary* myDict= ndb.getDictionary();
  const NdbDictionary::Table *check = myDict->getTable(::db::bank::check_table.c_str());

  auto tx = tx_start(ndb);
  auto op  = read_tuple(tx, check, id0, NdbOperation::LM_Exclusive);
  auto col = read_col(op,"BALANCE");
  auto op1  = read_tuple(tx, check, id1, NdbOperation::LM_Exclusive);
  auto col1 = read_col(op1,"BALANCE");

  RDMA_ASSERT(commit(tx));

  auto res = col->double_value() + col1->double_value();

  ndb.closeTransaction(tx);
  return double_equal(res,expected_value);
}

void check_consistency(Ndb &ndb,FastRandom &rand) {
  RDMA_LOG(4) << "start deposit checking test...";
  auto ret = deposit_checking(ndb,rand);
  RDMA_LOG_IF(4,ret.first) << "deposit checking test passes";

  ret = send_payment(ndb,rand);
  RDMA_LOG_IF(4,ret.first) << "send payment test passes";

  ret = transact_saving(ndb,rand);
  RDMA_LOG_IF(4,ret.first) << "TS test passes";
}

} // end namespace bank

} // end namespace db
 