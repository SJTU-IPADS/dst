#include "test_funcs.hpp"

#include "micro/micro1.hpp"
#include "smallbank/bank.hpp"
#include "tpcc/tpcc.hpp"

namespace db {

std::map<std::string,workload_generator_f> supported_workloads;
std::map<std::string,std::string>          workload_database;

txn_result_t simple_test(Ndb &myndb,FastRandom &);

workload_desc_vec_t generate_test_funcs() {
  auto ret = workload_desc_vec_t();
  ret.emplace_back("simple",1.0 /* 100% */,simple_test);
  return ret;
}

static bool sanity_checkworkload(workload_generator_f f){
  auto workloads = f();
  double sum = 0.0;

  for (auto w : workloads) {
    sum += w.frequency;
  }
  return abs(sum - 1.0) < 0.001;
}

static void _insert_workload(const std::string  &name,const std::string &db_name,workload_generator_f func) {
  RDMA_ASSERT(sanity_checkworkload(func)) << "workload sanity check [" << name << "] error: "
                                          << "the frequencies should sum up to 1.0!";
  supported_workloads.insert(std::make_pair(name,func));
  workload_database.insert(std::make_pair(name, db_name));
}

void init_workloads() {
  // the default workoad
  _insert_workload("test",::db::db_name,generate_test_funcs);

  // the workload for micro
  _insert_workload("micro1",micro1::db_name,micro1::generate_test_funcs);

  // the workload for Smallbank
  _insert_workload("bank", bank::db_name,bank::generate_test_funcs);

  // the workload for tpcc new order
  _insert_workload("tpcc", tpcc::db_name, tpcc::generate_test_funcs);
}

txn_result_t simple_test(Ndb &myNdb,FastRandom &rand) {
#if 1
  static NdbDictionary::Table *myTable = nullptr;
  if(myTable == nullptr) {
    const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();
    RDMA_ASSERT(myDict != nullptr);
    myTable= (NdbDictionary::Table *)(myDict->getTable(table_one.c_str()));
    if (myTable == nullptr)
      APIERROR(myDict->getNdbError());
  }
#endif
  auto mode = NdbOperation::LM_Exclusive;

  NdbTransaction *myTransaction = myNdb.startTransaction();

  NdbOperation *pop = myTransaction->getNdbOperation(myTable);
  pop->readTuple(mode);
  pop->equal("ID",0);

  NdbRecAttr *myRecAttr= pop->getValue("ATTR", nullptr);

  if( myTransaction->execute( NdbTransaction::NoCommit ) == -1 )
    APIERROR(myTransaction->getNdbError());

  //printf("then: %2d    %2d\n", 0, myRecAttr->u_32_value());

  NdbOperation *update_op = myTransaction->getNdbOperation(myTable);
  update_op->updateTuple();
  update_op->equal("ID",0);
  update_op->setValue("ATTR",myRecAttr->u_32_value() + 1);

  //myTransaction->refresh(); // avoids timeout on lock?
  if( myTransaction->execute( NdbTransaction::Commit ) == -1 )
    APIERROR(myTransaction->getNdbError());

  myNdb.closeTransaction(myTransaction);

  return txn_result_t(true,1);
}

};
 