#include "tpcc.hpp"
#include "util.hpp"

#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <set>


namespace db {

namespace tpcc {

// order status get customer tuple
inline bool OSGetCust(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *customerTable,
                    uint customer_id, uint district_id, uint warehouse_id,
                    std::string &customer_first, std::string &customer_middle,
                    std::string &customer_last, double &customer_balance)
{
  auto op = get_op(tx, customerTable);
  op->readTuple(lock_mode);
    
  op->equal(C_ID.c_str(), static_cast<Uint64>(customer_id));
  op->equal(C_D_ID.c_str(), static_cast<Uint64>(district_id));
  op->equal(C_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  NdbRecAttr *c_first = op->getValue(C_FIRST.c_str(), NULL);
  RDMA_ASSERT(c_first != nullptr);
  NdbRecAttr *c_middle = op->getValue(C_MIDDLE.c_str(), NULL);
  RDMA_ASSERT(c_middle != nullptr);
  NdbRecAttr *c_last = op->getValue(C_LAST.c_str(), NULL);
  RDMA_ASSERT(c_last != nullptr);
  NdbRecAttr *c_balance = op->getValue(C_BALANCE.c_str(), NULL);
  RDMA_ASSERT(c_balance != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  get_string(c_first, customer_first);
  get_string(c_middle, customer_middle);
  get_string(c_last, customer_last);
  customer_balance = c_balance->double_value();

  if (PrintInfo) {
    RDMA_LOG(5) << "GetCust(): customer_first : " << customer_first << "~";
    RDMA_LOG(3) << "customer_middle : " << customer_middle << "~";
    RDMA_LOG(3) << "customer_last : " << customer_last << "~";
    RDMA_LOG(3) << "customer_balance : " << customer_balance << "~";
  }
  return true;
}


// order status get recent order
uint OSGetRecentOrd(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *ordersIndexTable,
                  uint customer_id, uint district_id, uint warehouse_id)
{
  auto op = get_op(tx, ordersIndexTable);
  op->readTuple(lock_mode); // XD: replace to "lock_type"
    
  int c_d_w = customer_id - 1 + (district_id - 1) * 3000 + (warehouse_id - 1) * 30000;
  op->equal(C_D_W.c_str(), static_cast<Uint64>(c_d_w));

  // XD: should replace this scan, and should achieve similar performance
  NdbRecAttr *rec_ord_id = op->getValue(RECENT_ORDER_ID.c_str(), NULL);
  RDMA_ASSERT(rec_ord_id != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  int recent_ord_id = rec_ord_id->u_32_value();
  if (PrintInfo)
    RDMA_LOG(5) << "recent_ord_id from index table : " << recent_ord_id << "~";
  return recent_ord_id;
}


// order status get order
inline bool OSGetOrder(NdbTransaction *tx, Ndb &ndb,
                    const NdbDictionary::Table *ordersTable,
                    const NdbDictionary::Table *ordersIndexTable, 
                    uint customer_id, uint district_id, uint warehouse_id,
                    uint &order_id,
                    std::string &order_entry_d, uint &order_carrier_id, uint &order_ol_cnt)
{
  // Get most recent order id
  order_id = OSGetRecentOrd(tx, ndb, ordersIndexTable, customer_id, district_id, warehouse_id); 

  auto op = get_op(tx, ordersTable);
  op->readTuple(lock_mode);

  op->equal(O_ID.c_str(), static_cast<Uint64>(order_id));
  /* op->equal(O_C_ID.c_str(), static_cast<Uint64>(customer_id)); */
  op->equal(O_D_ID.c_str(), static_cast<Uint64>(district_id));
  op->equal(O_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  NdbRecAttr *o_id = op->getValue(O_ID.c_str(), NULL);
  RDMA_ASSERT(o_id != nullptr);
  NdbRecAttr *o_entry_d = op->getValue(O_ENTRY_D.c_str(), NULL);
  RDMA_ASSERT(o_entry_d != nullptr);
  NdbRecAttr *o_carrier_id = op->getValue(O_CARRIER_ID.c_str(), NULL);
  RDMA_ASSERT(o_carrier_id != nullptr);
  NdbRecAttr *o_ol_cnt = op->getValue(O_OL_CNT.c_str(), NULL);
  RDMA_ASSERT(o_ol_cnt != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  order_id = o_id->u_32_value();
  // TODO print date
  /* get_string(o_entry_d, order_entry_d); */
  /* RDMA_LOG(3) << "order_entry_d: " << order_entry_d << "~"; */
  order_carrier_id = o_carrier_id->u_32_value();
  order_ol_cnt = o_ol_cnt->u_32_value();

  if (PrintInfo) {
    RDMA_LOG(5) << "GetOrder(): order_id: " << order_id << "~";
    RDMA_LOG(3) << "order_carrier_id: " << order_carrier_id << "~";
    RDMA_LOG(3) << "order_ol_cnt: " << order_ol_cnt << "~";
  }
  return true;
}


// order status get order line
inline bool OSGetOrderLine(NdbTransaction *tx,Ndb &ndb, const NdbDictionary::Table *orderlineTable,
                    uint order_id, uint district_id, uint warehouse_id, uint orderline_number,
                    uint &orderline_i_id, uint &orderline_supply_w_id, uint &orderline_quantity,
                    double &orderline_amount, std::string &orderline_delivery_d)
{
  auto op = get_op(tx, orderlineTable);
  op->readTuple(lock_mode);

  op->equal(OL_O_ID.c_str(), static_cast<Uint64>(order_id));
  op->equal(OL_D_ID.c_str(), static_cast<Uint64>(district_id));
  op->equal(OL_W_ID.c_str(), static_cast<Uint64>(warehouse_id));
  op->equal(OL_NUMBER.c_str(), static_cast<Uint64>(orderline_number));

  NdbRecAttr *ol_i_id = op->getValue(OL_I_ID.c_str(), NULL);
  RDMA_ASSERT(ol_i_id != nullptr);
  NdbRecAttr *ol_supply_w_id = op->getValue(OL_SUPPLY_W_ID.c_str(), NULL);
  RDMA_ASSERT(ol_supply_w_id != nullptr);
  NdbRecAttr *ol_quantity = op->getValue(OL_QUANTITY.c_str(), NULL);
  RDMA_ASSERT(ol_quantity != nullptr);
  NdbRecAttr *ol_amount = op->getValue(OL_AMOUNT.c_str(), NULL);
  RDMA_ASSERT(ol_amount != nullptr);
  NdbRecAttr *ol_delivery_d = op->getValue(OL_DELIVERY_D.c_str(), NULL);
  RDMA_ASSERT(ol_delivery_d != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  orderline_i_id = ol_i_id->u_32_value();
  orderline_supply_w_id = ol_supply_w_id->u_32_value();
  orderline_quantity = ol_quantity->u_32_value();
  orderline_amount = ol_amount->double_value();
  // TODO print date
  /* orderline_delivery_d = ol_delivery_d->aRef(); */
  /* get_string(ol_delivery_d, orderline_delivery_d); */
  /* RDMA_LOG(3) << "orderline_delivery_d: " << orderline_delivery_d << "~"; */
  
  if (PrintInfo) {
    RDMA_LOG(5) << "GetOrderLine(): orderline_i_id: " << orderline_i_id << "~";
    RDMA_LOG(3) << "orderline_supply_w_id: " << orderline_supply_w_id << "~";
    RDMA_LOG(3) << "orderline_quantity: " << orderline_quantity << "~";
    RDMA_LOG(3) << "orderline_amount: " << orderline_amount << "~";
  }
  return true;
}


// transaction order status
txn_result_t txn_order_status(Ndb &ndb, FastRandom &rand) {

  const NdbDictionary::Dictionary *myDict= ndb.getDictionary();
  const NdbDictionary::Table *customerTable= myDict->getTable(::db::tpcc::CUSTOMER_TABLE.c_str());
  const NdbDictionary::Table *ordersTable = myDict->getTable(::db::tpcc::ORDERS_TABLE.c_str());
  const NdbDictionary::Table *orderlineTable =
                    myDict->getTable(::db::tpcc::ORDERLINE_TABLE.c_str());
  const NdbDictionary::Table *ordersIndexTable = myDict->getTable("orders_index");
  RDMA_ASSERT(myDict != nullptr);
  RDMA_ASSERT(customerTable != nullptr);
  RDMA_ASSERT(ordersTable != nullptr);
  RDMA_ASSERT(orderlineTable != nullptr);

  uint warehouse_id, district_id, customer_id;
  // XD: add 2 (the number of warehouses) to flags
  warehouse_id = PickWarehouseId(rand, 1, NumWarehouse);
  district_id = RandomNumber(rand, 1, NumDistrictsPerWarehouse);
  customer_id = GetCustomerId(rand);
  if (PrintInfo) {
    RDMA_LOG(2) << "warehouse_id : " << warehouse_id << ".";
    RDMA_LOG(2) << "district_id : " << district_id << ".";
    RDMA_LOG(2) << "customer_id : " << customer_id << ".";
  }

  auto tx = tx_start(ndb);
  {
    // TODO select by name
    //1.1 cust by last name
    if (RandomNumber(rand, 1, 100) <= 60) {
      // XD: we can skip this
    } else {

    }
    //1.2 cust by ID
    std::string customer_first;
    std::string customer_middle;
    std::string customer_last;
    double customer_balance;
    
    bool success1 = OSGetCust(tx, ndb, customerTable,
                            customer_id, district_id, warehouse_id,
                            customer_first, customer_middle, customer_last, customer_balance);
    if (!success1)
      RDMA_ASSERT(false);

    // STEP 2. read record from ORDER
    uint order_id = -1;
    std::string order_entry_d;
    uint order_carrier_id;
    uint order_ol_cnt;
    
    bool success2 = OSGetOrder(tx, ndb, ordersTable, ordersIndexTable,
                            customer_id, district_id, warehouse_id,
                            order_id,
                            order_entry_d, order_carrier_id, order_ol_cnt);
    if (!success2)
      RDMA_ASSERT(false);

    //STEP 3. read record from ORDERLINE
    if (order_id != -1) {

      uint orderline_i_id;
      uint orderline_supply_w_id;
      uint orderline_quantity;
      double orderline_amount;
      std::string orderline_delivery_d;

      for (int32_t orderline_number = 1; orderline_number <= order_ol_cnt; ++orderline_number)
      {
        bool success3 = OSGetOrderLine(tx, ndb, orderlineTable,
                                    order_id, district_id, warehouse_id, orderline_number,
                                    orderline_i_id, orderline_supply_w_id, orderline_quantity,
                                    orderline_amount, orderline_delivery_d);
        if (!success3)
          RDMA_ASSERT(false);
        /* break; */
      }
    }
    commit(tx);
    /* return txn_result_t(true, order_ol_cnt); */
  }

  ndb.closeTransaction(tx);
  return txn_result_t(true, 0);
}

} // end of namespace tpcc

} // end of namespace db
 