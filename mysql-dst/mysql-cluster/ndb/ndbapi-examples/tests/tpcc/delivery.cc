#include "tpcc.hpp"
#include "util.hpp"

#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <set>

namespace db {

namespace tpcc {

// delivery read new order table
inline bool DGetOldOrd(NdbTransaction *tx,Ndb &ndb,const NdbDictionary::Table *newOrdersIndexTable,
                      uint neword_w_id, uint neword_d_id, uint &oldest_oid)
{
  // TODO what if no new order in current district
  int check = -1;
  NdbIndexScanOperation *op = (NdbIndexScanOperation *)get_scan_op(tx, newOrdersIndexTable);
  /* op->readTuples(NdbOperation::LM_CommittedRead); */
  int result = op->readTuples(NdbOperation::LM_CommittedRead, 1, 1, false, false, false, false, false);
  if (result != 0)
    RDMA_ASSERT(false);

  int NO_INDEX_WDO_Col_Num = newOrdersIndexTable->getColumn(W_D_O.c_str())->getColumnNo();

  // TODO keep record of scan
  uint64_t up_id = (neword_w_id - 1) * 10 + neword_d_id-1;
  uint64_t low_id1 = 0;
  uint64_t w_d_o1 = (int64_t)(up_id << 32) | (int64_t)low_id1;

  uint64_t low_id2 = 4294967295; // max of uint 32
  uint64_t w_d_o2 = (int64_t)(up_id << 32) | (int64_t)low_id2;

  uint64_t scanFrom = w_d_o1;
  uint64_t scanTo = w_d_o2;
  if (PrintInfo) {
    RDMA_LOG(1) << "scan From : " << scanFrom;
    RDMA_LOG(1) << "scan To : " << scanTo;
  }

  NdbScanFilter filter(op);
  if (filter.begin(NdbScanFilter::AND) < 0 ||
      filter.cmp(NdbScanFilter::COND_GT, NO_INDEX_WDO_Col_Num, &scanFrom, 20) < 0 ||
      filter.cmp(NdbScanFilter::COND_LT, NO_INDEX_WDO_Col_Num, &scanTo, 20) < 0 ||
      filter.end())
  {
    RDMA_ASSERT(false);
  }

  auto wdo_RecAttr = op->getValue(W_D_O.c_str());
  RDMA_ASSERT(wdo_RecAttr != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    RDMA_ASSERT(false);
  }

  if ((check = op->nextResult(true)) == 0){

    uint64_t w_d_o = wdo_RecAttr->u_64_value();
    oldest_oid = (uint)((w_d_o << 32) >> 32) + 1;
    if (PrintInfo) {
      RDMA_LOG(3) << "w_d_o: " << w_d_o;
      RDMA_LOG(5) << "oldest_oid : " << oldest_oid << " from district " << neword_d_id;
    }
    // TODO current impl random select one(not the oldest)
    op->close();
    return true;
  }

  return false;
}

// delivery read new order table
/* inline bool DGetOldOrd(NdbTransaction *tx,Ndb &ndb, const NdbDictionary::Table *newOrdTable, */
/*                       uint neword_w_id, uint neword_d_id, uint &oldest_oid) */
/* { */
/*   // TODO what if no new order in current district */
/*   int check = -1; */
/*   auto op = get_scan_op(tx, newOrdTable); */
/*   op->readTuples(NdbOperation::LM_CommittedRead); */
    
/*   int NO_W_ID_Col_Num = newOrdTable->getColumn(NO_W_ID.c_str())->getColumnNo(); */
/*   int NO_D_ID_Col_Num = newOrdTable->getColumn(NO_D_ID.c_str())->getColumnNo(); */
/*   /1* auto d_id_RecAttr = op->getValue(NO_D_ID.c_str()); *1/ */
/*   /1* auto w_id_RecAttr = op->getValue(NO_W_ID.c_str()); *1/ */

/*   NdbScanFilter filter(op); */
/*   if (filter.begin(NdbScanFilter::AND) < 0 || */
/*       filter.cmp(NdbScanFilter::COND_EQ, NO_W_ID_Col_Num, &neword_w_id, 11) < 0 || */
/*       filter.cmp(NdbScanFilter::COND_EQ, NO_D_ID_Col_Num, &neword_d_id, 11) < 0 || */
/*       filter.end()) */
/*   { */
/*     RDMA_ASSERT(false); */
/*   } */

/*   auto o_id_RecAttr = op->getValue(NO_O_ID.c_str()); */
/*   RDMA_ASSERT(o_id_RecAttr != nullptr); */

/*   if(!pre_execute(tx)) { */
/*     ndb.closeTransaction(tx); */
/*     RDMA_ASSERT(false); */
/*   } */

/*   while ((check = op->nextResult(true)) == 0){ */

/*     oldest_oid = o_id_RecAttr->u_32_value(); */
/*     if (PrintInfo) */
/*       RDMA_LOG(5) << "oldest_oid : " << oldest_oid << "~"; */
/*     // TODO current impl random select one(not the oldest) */
/*     break; */
/*   } */

/*   return true; */
/* } */

// delivery delete new order
inline bool DDelNewOrd(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *newOrdTable,
                      const NdbDictionary::Table *newOrdIndexTable, uint neword_w_id,
                      uint neword_d_id, uint neword_o_id)
{
  auto op = get_op(tx, newOrdIndexTable);

  uint64_t up_id = (neword_w_id - 1) * 10 + neword_d_id-1;
  uint64_t low_id = neword_o_id - 1;
  uint64_t w_d_o = (int64_t)(up_id << 32) | (int64_t)low_id;
  if(PrintInfo)
    RDMA_LOG(1) << "delete w_d_o " << w_d_o << " from district " << neword_d_id;
  op->deleteTuple();
  op->equal(W_D_O.c_str(), static_cast<Uint64>(w_d_o));

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    RDMA_ASSERT(false);
  }

  if (PrintInfo)
    RDMA_LOG(5) << "delete from new order index : w_d_o " <<  w_d_o << ", " 
      << neword_d_id << ", " << neword_w_id;

  auto op2 = get_op(tx, newOrdTable);

  op2->deleteTuple();
  op2->equal(NO_O_ID.c_str(), neword_o_id);
  op2->equal(NO_D_ID.c_str(), neword_d_id);
  op2->equal(NO_W_ID.c_str(), neword_w_id);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    RDMA_ASSERT(false);
  }

  if (PrintInfo)
    RDMA_LOG(5) << "delete from new order : oid, did, wid " <<  neword_o_id << ", " 
      << neword_d_id << ", " << neword_w_id;
  return true;
}

// delivery read & update orders table
inline bool DGetUpdOrders(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *ordersTable,
                          uint orders_w_id, uint orders_d_id, uint orders_id,
                          uint &orders_c_id,
                          uint order_carrier_id)
{
  auto op = get_op(tx, ordersTable);
  op->readTuple(NdbOperation::LM_Exclusive);

  op->equal(O_ID.c_str(), orders_id);
  op->equal(O_D_ID.c_str(), orders_d_id);
  op->equal(O_W_ID.c_str(), orders_w_id);

  auto o_c_id = op->getValue(O_C_ID.c_str(), NULL);
  RDMA_ASSERT(o_c_id != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    RDMA_ASSERT(false);
  }

  orders_c_id = o_c_id->u_32_value();
  if (PrintInfo)
    RDMA_LOG(5) << "o_c_id : " << orders_c_id;


  // update orders table
  auto op2 = get_op(tx, ordersTable);
  op2->updateTuple();

  op2->equal(O_ID.c_str(), orders_id);
  op2->equal(O_D_ID.c_str(), orders_d_id);
  op2->equal(O_W_ID.c_str(), orders_w_id);
  
  op2->setValue(O_CARRIER_ID.c_str(), order_carrier_id);

  if (PrintInfo)
    RDMA_LOG(3) << "update orders table: carrier id " << order_carrier_id;

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }

  return true;
}

// delivery read & update orders line table
// TODO this func will read orders table to get o_ol_cnt
inline bool DGetUpdOrdLn(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *orderlineTable,
                          const NdbDictionary::Table *ordersTable,
                          uint orders_w_id, uint orders_d_id, uint orders_id,
                          uint &total_ol_amount)
{
  auto op = get_op(tx, ordersTable);
  op->readTuple(NdbOperation::LM_Exclusive);

  op->equal(O_ID.c_str(), static_cast<Uint64>(orders_id));
  op->equal(O_D_ID.c_str(), static_cast<Uint64>(orders_d_id));
  op->equal(O_W_ID.c_str(), static_cast<Uint64>(orders_w_id));

  NdbRecAttr *o_ol_cnt = op->getValue(O_OL_CNT.c_str(), NULL);
  RDMA_ASSERT(o_ol_cnt != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  int order_ol_cnt = o_ol_cnt->u_32_value();
  if (PrintInfo)
    RDMA_LOG(5) << "order_ol_cnt : " << order_ol_cnt;

  // read order line table
  for (int i = 0; i < order_ol_cnt; i++) {
    auto op = get_op(tx, orderlineTable);
    op->readTuple(NdbOperation::LM_Exclusive);

    op->equal(OL_O_ID.c_str(), static_cast<Uint64>(orders_id));
    op->equal(OL_D_ID.c_str(), static_cast<Uint64>(orders_d_id));
    op->equal(OL_W_ID.c_str(), static_cast<Uint64>(orders_w_id));
    op->equal(OL_NUMBER.c_str(), static_cast<Uint64>(i+1));

    NdbRecAttr *ol_amount = op->getValue(OL_AMOUNT.c_str(), NULL);
    RDMA_ASSERT(ol_amount != nullptr);

    if(!pre_execute(tx)) {
      ndb.closeTransaction(tx);
      return false;
    }

    int orderline_amount = ol_amount->double_value();
    total_ol_amount += orderline_amount;

    if (PrintInfo)
      RDMA_LOG(3) << "orderline_amount: " << orderline_amount << "~";
  }

  // update order line table
  for (int i = 0; i < order_ol_cnt; i++) {
    auto op = get_op(tx, orderlineTable);
    op->updateTuple();

    op->equal(OL_O_ID.c_str(), static_cast<Uint64>(orders_id));
    op->equal(OL_D_ID.c_str(), static_cast<Uint64>(orders_d_id));
    op->equal(OL_W_ID.c_str(), static_cast<Uint64>(orders_w_id));
    op->equal(OL_NUMBER.c_str(), static_cast<Uint64>(i+1));

    op->setValue(OL_DELIVERY_D.c_str(), 666);

    if(!pre_execute(tx)) {
      ndb.closeTransaction(tx);
      return false;
    }

    if (PrintInfo)
      RDMA_LOG(3) << "update order line : delivery data to 666 ";
  }

  return true;
}


// delivey update customer table
inline bool DUpdCust(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *customerTable,
                    uint customer_id, uint district_id, uint warehouse_id,
                    uint total_ol_amount)
{
  auto op = get_op(tx, customerTable);
  op->readTuple(NdbOperation::LM_Exclusive);
    
  op->equal(C_ID.c_str(), static_cast<Uint64>(customer_id));
  op->equal(C_D_ID.c_str(), static_cast<Uint64>(district_id));
  op->equal(C_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  auto c_balance = op->getValue(C_BALANCE.c_str(), NULL);
  RDMA_ASSERT(c_balance != nullptr);
  auto c_delivery_cnt = op->getValue(C_DELIVERY_CNT.c_str(), NULL);
  RDMA_ASSERT(c_delivery_cnt != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  double customer_balance = c_balance->double_value();
  int customer_delivery_cnt = c_delivery_cnt->u_32_value();

  if (PrintInfo) {
    RDMA_LOG(5) << "customer delivery cnt : " << customer_delivery_cnt;
    RDMA_LOG(3) << "customer balance : " << customer_balance;
  }

  auto op2 = get_op(tx, customerTable);
  op2->updateTuple();
    
  op2->equal(C_ID.c_str(), static_cast<Uint64>(customer_id));
  op2->equal(C_D_ID.c_str(), static_cast<Uint64>(district_id));
  op2->equal(C_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  op2->setValue(C_BALANCE.c_str(), customer_balance + total_ol_amount);
  op2->setValue(C_DELIVERY_CNT.c_str(), customer_delivery_cnt + 1);

  if (PrintInfo) {
    RDMA_LOG(5) << "update customer delivery cnt : " << customer_delivery_cnt + 1;
    RDMA_LOG(3) << "update customer balance : " << customer_balance + total_ol_amount;
  }
  return true;
}



txn_result_t txn_delivery(Ndb &ndb, FastRandom &rand) {

  const NdbDictionary::Dictionary *myDict= ndb.getDictionary();
  const NdbDictionary::Table *customerTable= myDict->getTable(::db::tpcc::CUSTOMER_TABLE.c_str());
  const NdbDictionary::Table *newOrdTable= myDict->getTable(::db::tpcc::NEW_ORDER_TABLE.c_str());
  const NdbDictionary::Table *ordersTable = myDict->getTable(::db::tpcc::ORDERS_TABLE.c_str());
  const NdbDictionary::Table *orderlineTable = myDict->getTable(::db::tpcc::ORDERLINE_TABLE.c_str());
  const NdbDictionary::Table *newOrdersIndexTable = myDict->getTable(::db::tpcc::NEW_ORDERS_INDEX.c_str());
  RDMA_ASSERT(myDict != nullptr);

  RDMA_ASSERT(customerTable != nullptr);
  RDMA_ASSERT(newOrdTable != nullptr);
  RDMA_ASSERT(ordersTable != nullptr);
  RDMA_ASSERT(orderlineTable != nullptr);
  RDMA_ASSERT(newOrdersIndexTable != nullptr);

  uint warehouse_id, carrier_id;
  warehouse_id = PickWarehouseId(rand, 1, NumWarehouse);
  carrier_id = RandomNumber(rand, 1, NumWarehouse);
  if (PrintInfo) {
    RDMA_LOG(2) << "warehouse_id : " << warehouse_id << ".";
  }

  /* for (int i = 0; i < 10; i++) { */

  auto tx = tx_start(ndb);
  {
    // TODO for each district of that warehouse
    uint neword_o_ids[10];
    uint orders_c_ids[10];
    uint total_ol_amount = 0;

    /* bool success1 = DGetOldOrd(tx, ndb, newOrdersIndexTable, warehouse_id, 1, neword_o_ids[0]); */
    /* if (!success1) */
    /*   RDMA_ASSERT(false); */

    for (int i = 0; i < 10; i++) {

      bool success1 = DGetOldOrd(tx, ndb, newOrdersIndexTable, warehouse_id, i+1, neword_o_ids[i]);
      if (!success1)
        RDMA_ASSERT(false);

      bool success2 = DDelNewOrd(tx, ndb, newOrdTable, newOrdersIndexTable, warehouse_id, i+1,
                                 neword_o_ids[i]);
      if (!success2)
        RDMA_ASSERT(false);

      bool success3 = DGetUpdOrders(tx, ndb, ordersTable, warehouse_id, i+1, neword_o_ids[i],
                                    orders_c_ids[i], carrier_id);
      if (!success3)
        RDMA_ASSERT(false);

      bool success4 = DGetUpdOrdLn(tx, ndb, orderlineTable, ordersTable,
                                    warehouse_id, i+1, neword_o_ids[i], total_ol_amount);
      if (!success4)
        RDMA_ASSERT(false);

      bool success5 = DUpdCust(tx, ndb, customerTable,
                                    orders_c_ids[i], i+1, warehouse_id, total_ol_amount);
      if (!success5)
        RDMA_ASSERT(false);
    }
      
    commit(tx);
  }

  ndb.closeTransaction(tx);
  /* } */
  return txn_result_t(true, 0);
}

} // end of namespace tpcc

} // end of namespace db
 