#include "tpcc.hpp"
#include "util.hpp"
#include "tx_workload.hpp"

#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <set>

namespace db {

namespace tpcc {


// payment get warehouse
inline bool PGetUpdWare(NdbTransaction *tx,Ndb &ndb,const NdbDictionary::Table *warehouseTable,
                          uint warehouse_id, uint history_amount, std::string &warehouse_name,
                          std::string &warehouse_street_1,std::string &warehouse_street_2,
                          std::string &warehouse_city,std::string &warehouse_state,
                          std::string &warehouse_zip)
{
  // get warehouse tuple
  double warehouse_ytd;
  auto op = get_op(tx, warehouseTable);
  op->readTuple(NdbOperation::LM_Exclusive);

  op->equal(W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  NdbRecAttr *w_name = op->getValue(W_NAME.c_str(), NULL);
  RDMA_ASSERT(w_name != nullptr);
  NdbRecAttr *w_street_1 = op->getValue(W_STREET_1.c_str(), NULL);
  RDMA_ASSERT(w_street_1 != nullptr);
  NdbRecAttr *w_street_2 = op->getValue(W_STREET_2.c_str(), NULL);
  RDMA_ASSERT(w_street_2 != nullptr);
  NdbRecAttr *w_city = op->getValue(W_CITY.c_str(), NULL);
  RDMA_ASSERT(w_city != nullptr);
  NdbRecAttr *w_state = op->getValue(W_STATE.c_str(), NULL);
  RDMA_ASSERT(w_state != nullptr);
  NdbRecAttr *w_zip = op->getValue(W_ZIP.c_str(), NULL);
  RDMA_ASSERT(w_zip != nullptr);
  NdbRecAttr *w_ytd = op->getValue(W_YTD.c_str(), NULL);
  RDMA_ASSERT(w_ytd != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  get_string(w_name, warehouse_name);
  get_string(w_street_1, warehouse_street_1);
  get_string(w_street_2, warehouse_street_2);
  get_string(w_city, warehouse_city);
  get_string(w_state, warehouse_state);
  get_string(w_zip, warehouse_zip);
  warehouse_ytd = w_ytd->double_value();
  if (PrintInfo) {
    RDMA_LOG(5) << "warehouse_name : " << warehouse_name << "~";
    RDMA_LOG(3) << "warehouse_street_1 : " << warehouse_street_1 << "~";
    RDMA_LOG(3) << "warehouse_street_2 : " << warehouse_street_2 << "~";
    RDMA_LOG(3) << "warehouse_city : " << warehouse_city << "~";
    RDMA_LOG(3) << "warehouse_state : " << warehouse_state << "~";
    RDMA_LOG(3) << "warehouse_zip : " << warehouse_zip << "~";
    RDMA_LOG(3) << "warehouse_ytd: " << warehouse_ytd << "~";
  }


  // update warehouse tuple
  auto op2 = get_op(tx, warehouseTable);
  op2->updateTuple();

  op2->equal(W_ID.c_str(), static_cast<Uint64>(warehouse_id));
  op2->setValue(W_YTD.c_str(), (warehouse_ytd + history_amount));
  if (PrintInfo) {
    RDMA_LOG(5) << "update warehouse_ytd to " << (warehouse_ytd + history_amount);
  }

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }
  return true;
}


// payment get district
inline bool PGetUpdDist(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *districtTable,
                        uint warehouse_id, uint district_id, double history_amount,
                        std::string &district_name, std::string &district_street_1,
                        std::string &district_street_2, std::string &district_city,
                        std::string &district_state, std::string &district_zip)
{
  // get district tuple
  double district_ytd;
  auto op = get_op(tx, districtTable);
  op->readTuple(NdbOperation::LM_Exclusive);

  op->equal(D_ID.c_str(), static_cast<Uint64>(district_id));
  op->equal(D_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  NdbRecAttr *d_name = op->getValue(D_NAME.c_str(), NULL);
  RDMA_ASSERT(d_name != nullptr);
  NdbRecAttr *d_street_1 = op->getValue(D_STREET_1.c_str(), NULL);
  RDMA_ASSERT(d_street_1 != nullptr);
  NdbRecAttr *d_street_2 = op->getValue(D_STREET_2.c_str(), NULL);
  RDMA_ASSERT(d_street_2 != nullptr);
  NdbRecAttr *d_city = op->getValue(D_CITY.c_str(), NULL);
  RDMA_ASSERT(d_city != nullptr);
  NdbRecAttr *d_state = op->getValue(D_STATE.c_str(), NULL);
  RDMA_ASSERT(d_state != nullptr);
  NdbRecAttr *d_zip = op->getValue(D_ZIP.c_str(), NULL);
  RDMA_ASSERT(d_zip != nullptr);
  NdbRecAttr *d_ytd = op->getValue(D_YTD.c_str(), NULL);
  RDMA_ASSERT(d_ytd != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  get_string(d_name, district_name);
  get_string(d_street_1, district_street_1);
  get_string(d_street_2, district_street_2);
  get_string(d_city, district_city);
  get_string(d_state, district_state);
  get_string(d_zip, district_zip);
  district_ytd = d_ytd->double_value();
  if (PrintInfo) {
    RDMA_LOG(5) << "district_name : " << district_name << "~";
    RDMA_LOG(3) << "district_street_1 : " << district_street_1 << "~";
    RDMA_LOG(3) << "district_street_2 : " << district_street_2 << "~";
    RDMA_LOG(3) << "district_city : " << district_city << "~";
    RDMA_LOG(3) << "district_state : " << district_state << "~";
    RDMA_LOG(3) << "district_zip : " << district_zip << "~";
    RDMA_LOG(3) << "district_ytd : " << district_ytd << "~";
  }

  // update district tuple
  auto op2 = get_op(tx, districtTable);
  op2->updateTuple();

  op2->equal(D_W_ID.c_str(), static_cast<Uint64>(warehouse_id));
  op2->equal(D_ID.c_str(), static_cast<Uint64>(district_id));
  op2->setValue(D_YTD.c_str(), (district_ytd + history_amount));
  if (PrintInfo) {
    RDMA_LOG(5) << "update district_ytd to " << (district_ytd + history_amount);
  }

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }
  return true;
}


// payment get customer
inline bool PGetUpdCust(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *customerTable,
                    uint customer_id, uint district_id, uint warehouse_id, uint history_amount,
                    std::string &customer_first, std::string &customer_middle,
                    std::string &customer_last, std::string & customer_street_1,
                    std::string &customer_street_2, std::string &customer_city,
                    std::string &customer_state, std::string &customer_zip,
                    std::string &customer_phone, std::string &customer_since,
                    std::string &customer_credit, std::string &customer_credit_lim,
                    double &customer_discount, double &customer_balance,
                    double &customer_ytd_payment, uint &customer_payment_cnt)
{
  auto op = get_op(tx, customerTable);
  op->readTuple(NdbOperation::LM_Exclusive);

  op->equal(C_ID.c_str(), static_cast<Uint64>(customer_id));
  op->equal(C_D_ID.c_str(), static_cast<Uint64>(district_id));
  op->equal(C_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  NdbRecAttr *c_first = op->getValue(C_FIRST.c_str(), NULL);
  RDMA_ASSERT(c_first != nullptr);
  NdbRecAttr *c_middle = op->getValue(C_MIDDLE.c_str(), NULL);
  RDMA_ASSERT(c_middle != nullptr);
  NdbRecAttr *c_last = op->getValue(C_LAST.c_str(), NULL);
  RDMA_ASSERT(c_last != nullptr);

  NdbRecAttr *c_street_1 = op->getValue(C_STREET_1.c_str(), NULL);
  RDMA_ASSERT(c_street_1 != nullptr);
  NdbRecAttr *c_street_2 = op->getValue(C_STREET_2.c_str(), NULL);
  RDMA_ASSERT(c_street_2 != nullptr);
  NdbRecAttr *c_city = op->getValue(C_CITY.c_str(), NULL);
  RDMA_ASSERT(c_city != nullptr);

  NdbRecAttr *c_state = op->getValue(C_STATE.c_str(), NULL);
  RDMA_ASSERT(c_state != nullptr);
  NdbRecAttr *c_zip = op->getValue(C_ZIP.c_str(), NULL);
  RDMA_ASSERT(c_zip != nullptr);
  NdbRecAttr *c_phone = op->getValue(C_PHONE.c_str(), NULL);
  RDMA_ASSERT(c_phone != nullptr);

  // TODO get since date & bigint credit limit
  /* NdbRecAttr *c_since = op->getValue(C_SINCE.c_str(), NULL); */
  /* RDMA_ASSERT(c_since != nullptr); */
  NdbRecAttr *c_credit = op->getValue(C_CREDIT.c_str(), NULL);
  RDMA_ASSERT(c_credit != nullptr);
  /* NdbRecAttr *c_credit_lim = op->getValue(C_CREDIT_LIM.c_str(), NULL); */
  /* RDMA_ASSERT(c_credit_lim != nullptr); */

  NdbRecAttr *c_discount = op->getValue(C_DISCOUNT.c_str(), NULL);
  RDMA_ASSERT(c_discount != nullptr);
  NdbRecAttr *c_balance = op->getValue(C_BALANCE.c_str(), NULL);
  RDMA_ASSERT(c_balance != nullptr);

  NdbRecAttr *c_ytd_payment = op->getValue(C_YTD_PAYMENT.c_str(), NULL);
  RDMA_ASSERT(c_ytd_payment != nullptr);
  NdbRecAttr *c_payment_cnt = op->getValue(C_PAYMENT_CNT.c_str(), NULL);
  RDMA_ASSERT(c_payment_cnt != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  get_string(c_first, customer_first);
  get_string(c_middle, customer_middle);
  get_string(c_last, customer_last);

  get_string(c_street_1, customer_street_1);
  get_string(c_street_2, customer_street_2);
  get_string(c_city, customer_city);

  get_string(c_state, customer_state);
  get_string(c_zip, customer_zip);
  get_string(c_phone, customer_phone);

  get_string(c_credit, customer_credit);

  customer_discount = c_discount->double_value();
  customer_balance = c_balance->double_value();

  customer_ytd_payment = c_ytd_payment->double_value();
  customer_payment_cnt = c_payment_cnt->double_value();
  if (PrintInfo) {
    RDMA_LOG(5) << "GetCust(): customer_first : " << customer_first << "~";
    RDMA_LOG(3) << "customer_middle : " << customer_middle << "~";
    RDMA_LOG(3) << "customer_last : " << customer_last << "~";
    RDMA_LOG(3) << "GetCust(): customer_street_1 : " << customer_street_1 << "~";
    RDMA_LOG(3) << "customer_street_2 : " << customer_street_2 << "~";
    RDMA_LOG(3) << "customer_city : " << customer_city << "~";
    RDMA_LOG(3) << "GetCust(): customer_state : " << customer_state << "~";
    RDMA_LOG(3) << "customer_zip : " << customer_zip << "~";
    RDMA_LOG(3) << "customer_phone : " << customer_phone << "~";
    RDMA_LOG(3) << "customer_credit : " << customer_credit << "~";
    RDMA_LOG(3) << "customer_discount : " << customer_discount << "~";
    RDMA_LOG(3) << "customer_balance : " << customer_balance << "~";
    RDMA_LOG(3) << "customer_ytd_payment : " << customer_ytd_payment << "~";
    RDMA_LOG(3) << "customer_payment_cnt : " << customer_payment_cnt << "~";
  }

  // update customer tuple
  auto op2 = get_op(tx, customerTable);
  op2->updateTuple();

  op2->equal(C_ID.c_str(), static_cast<Uint64>(customer_id));
  op2->equal(C_D_ID.c_str(), static_cast<Uint64>(district_id));
  op2->equal(C_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  op2->setValue(C_BALANCE.c_str(), (customer_balance - history_amount));
  op2->setValue(C_YTD_PAYMENT.c_str(), (customer_ytd_payment + history_amount));
  op2->setValue(C_PAYMENT_CNT.c_str(), (customer_payment_cnt + 1));
  if (PrintInfo) {
    RDMA_LOG(5) << "update balance to " << (customer_balance - history_amount);
    RDMA_LOG(5) << "update ytd payment to " << (customer_ytd_payment + history_amount);
    RDMA_LOG(5) << "update payment cnt to " << (customer_payment_cnt + 1);
  }

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  // TODO update c_data

  return true;
}

// payment insert history table
// TODO insert date
// TODO sometine failed to insert
inline bool PInsertHist(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *historyTable,
                        uint history_c_id, uint history_c_d_id, uint history_c_w_id,
                        uint history_d_id, uint history_w_id, double history_amount,
                        std::string history_data)
{
  auto op = get_op(tx, historyTable);
  op->insertTuple();

  op->setValue(H_C_ID.c_str(), static_cast<Uint64>(history_c_id));
  op->setValue(H_C_D_ID.c_str(), static_cast<Uint64>(history_c_d_id));
  op->setValue(H_C_W_ID.c_str(), static_cast<Uint64>(history_c_w_id));
  op->setValue(H_D_ID.c_str(), static_cast<Uint64>(history_d_id));
  op->setValue(H_W_ID.c_str(), static_cast<Uint64>(history_w_id));
  op->setValue(H_AMOUNT.c_str(), history_amount);
  // TODO insert date
  // TODO insert history data
  /* op->setValue(H_DATA.c_str(), history_data); */

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }
  if (PrintInfo)
    RDMA_LOG(5) << "insert into history : " << history_c_id << "," << history_c_d_id << ","
              << history_c_w_id << "," << history_d_id << "," << history_w_id <<
              "," << history_amount << "," << history_data << "~";
  return true;
}


// payment transaction
txn_result_t txn_payment(Ndb &ndb, FastRandom &rand) {

  const NdbDictionary::Dictionary *myDict= ndb.getDictionary();
  const NdbDictionary::Table *warehouseTable=myDict->getTable(::db::tpcc::WAREHOUSE_TABLE.c_str());
  const NdbDictionary::Table *districtTable= myDict->getTable(::db::tpcc::DISTRICT_TABLE.c_str());
  const NdbDictionary::Table *customerTable= myDict->getTable(::db::tpcc::CUSTOMER_TABLE.c_str());
  const NdbDictionary::Table *historyTable= myDict->getTable(::db::tpcc::HISTORY_TABLE.c_str());
  RDMA_ASSERT(myDict != nullptr);
  RDMA_ASSERT(warehouseTable != nullptr);
  RDMA_ASSERT(districtTable != nullptr);
  RDMA_ASSERT(customerTable != nullptr);
  RDMA_ASSERT(historyTable != nullptr);

  uint warehouse_id, district_id, customer_id;
  double history_amount;
  warehouse_id = PickWarehouseId(rand, 1, NumWarehouse);
  district_id = RandomNumber(rand, 1, NumDistrictsPerWarehouse);
  customer_id = GetCustomerId(rand);
  history_amount = RandomNumber(rand, 1, 5000);
  bool isHomeWarehouse = true;
  uint homeWarehouse, homeDistrict;

  /**
   * I think we should change this logic.
   * Use the setting according to the specification.
   */
  if (RandomNumber(rand, 1, 100) > 85) {
    isHomeWarehouse = false;
    homeWarehouse = warehouse_id;
    warehouse_id = GetOtherWarehouse(warehouse_id, rand);
    homeDistrict = district_id;
    district_id = RandomNumber(rand, 1, NumDistrictsPerWarehouse);
  }
  if (PrintInfo) {
    RDMA_LOG(2) << "warehouse_id : " << warehouse_id << ".";
    RDMA_LOG(2) << "district_id : " << district_id << ".";
    RDMA_LOG(2) << "customer_id : " << customer_id << ".";
  }

  auto tx = tx_start(ndb);
  {
    std::string warehouse_name, warehouse_street_1, warehouse_street_2, warehouse_city,
                warehouse_state, warehouse_zip;
    bool success1 = PGetUpdWare(tx, ndb, warehouseTable,
                                warehouse_id, history_amount, warehouse_name,
                                warehouse_street_1, warehouse_street_2, warehouse_city,
                                warehouse_state, warehouse_zip);
    if (!success1)
      RDMA_ASSERT(false);

    std::string district_name, district_street_1, district_street_2, district_city,
                district_state, district_zip;
    bool success2 = PGetUpdDist(tx, ndb, districtTable,
                                warehouse_id, district_id, history_amount,
                                district_name, district_street_1,
                                district_street_2, district_city,
                                district_state, district_zip);
    if (!success2)
      RDMA_ASSERT(false);

    // TODO select by name
    // cust by last name
    if (RandomNumber(rand, 1, 100) <= 60) {

    } else {

    }
    // cust by ID
    std::string customer_first, customer_middle, customer_last, customer_street_1,
                customer_street_2, customer_city, customer_state, customer_zip,
                customer_phone, customer_since, customer_credit, customer_credit_lim;
    double customer_discount, customer_balance, customer_ytd_payment;
    uint customer_payment_cnt;
    bool success3 = PGetUpdCust(tx, ndb, customerTable,
                                customer_id, district_id, warehouse_id, history_amount,
                                customer_first, customer_middle,
                                customer_last, customer_street_1,
                                customer_street_2, customer_city,
                                customer_state, customer_zip,
                                customer_phone, customer_since,
                                customer_credit, customer_credit_lim,
                                customer_discount, customer_balance,
                                customer_ytd_payment, customer_payment_cnt);
    if (!success3)
      RDMA_ASSERT(false);

    std::string history_data = warehouse_name + district_name;
    /**
     * XD: insert to histories donot need pre-execute
     */
    // TODO uncomment and add primary key
    if (isHomeWarehouse) {
      /* bool success4 = PInsertHist(tx, ndb, historyTable, */
                                  /* customer_id, district_id, warehouse_id, */
                                  /* district_id, warehouse_id, history_amount, */
                                  /* history_data); */
      /* if (!success4) */
        /* RDMA_ASSERT(false); */
    } else {
      /* bool success4 = PInsertHist(tx, ndb, historyTable, */
                                  /* customer_id, homeDistrict, homeWarehouse, */
                                  /* district_id, warehouse_id, history_amount, */
                                  /* history_data); */
      /* if (!success4) */
        /* RDMA_ASSERT(false); */
    }
    commit(tx);
  }

  ndb.closeTransaction(tx);
  return txn_result_t(true, 0);
}


} // end of namespace tpcc

} // end of namespace db
 