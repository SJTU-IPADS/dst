#include "tpcc.hpp"
#include "util.hpp"

#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <set>


namespace db {

namespace tpcc {


// stock level get district talbe
inline bool SLGetDistrict(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *districtTable,
                    uint warehouse_id, uint district_id, uint &district_next_o_id)
{
  auto op = get_op(tx, districtTable);
  op->readTuple(lock_mode);
    
  op->equal(D_ID.c_str(), static_cast<Uint64>(district_id));
  op->equal(D_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  NdbRecAttr *d_next_o_id = op->getValue(D_NEXT_O_ID.c_str(), NULL);
  RDMA_ASSERT(d_next_o_id != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  district_next_o_id = d_next_o_id->u_32_value();
  if (PrintInfo)
    RDMA_LOG(3) << "district_next_o_id : " << district_next_o_id << "~";

  return true;
}



// stock level get order line talbe
// TODO current impl will read Order talbe in order to avoid uisng ndb scan operation
inline bool SLGetOrderLine(NdbTransaction *tx,Ndb &ndb, const NdbDictionary::Table *orderlineTable,
                          const NdbDictionary::Table *ordersTable,
                          uint district_id, uint warehouse_id, uint district_next_o_id,
                          uint (&orderline_i_ids)[300]) // allocate big enougth
{
  uint order_ol_cnts[NumOldOrders];

  if (PrintInfo)
    RDMA_LOG(5) << "get order_ol_cnts[20] from orders talbe";
  // get order_ol_cnts from orders table
  // XD: it is better to move 20 to a constant
  uint order_id = district_next_o_id - NumOldOrders;
  for (int i = 0; i < NumOldOrders; i++)
  {
    auto op = get_op(tx, ordersTable);
    // XD: stock_level is a read-only transaction, so its read should use "lock_type"
    op->readTuple(lock_mode);

    op->equal(O_ID.c_str(), static_cast<Uint64>(order_id));
    op->equal(O_D_ID.c_str(), static_cast<Uint64>(district_id));
    op->equal(O_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

    NdbRecAttr *o_ol_cnt = op->getValue(O_OL_CNT.c_str(), NULL);
    RDMA_ASSERT(o_ol_cnt != nullptr);
    // XD: I think we can batch execute here
    if(!pre_execute(tx)) {
      ndb.closeTransaction(tx);
      return false;
    }

    order_ol_cnts[i] = o_ol_cnt->u_32_value();
    if (PrintInfo)
      RDMA_LOG(3) << "order_ol_cnts[ " << i << "] : " << order_ol_cnts[i] << "~";
    order_id++;
  }

  if (PrintInfo)
    RDMA_LOG(5) << "get item ids[] from orders table";
  // get item id from order line table
  order_id = district_next_o_id - NumOldOrders; // XD: again, move 20 to a constants
  int item_id_index = 0;
  for (int i = 0; i < NumOldOrders; i++) 
  {
    for (int j = 1; j <= order_ol_cnts[i]; j++) 
    {
      auto op = get_op(tx, orderlineTable);
      op->readTuple(lock_mode);

      op->equal(OL_O_ID.c_str(), static_cast<Uint64>(order_id));
      op->equal(OL_D_ID.c_str(), static_cast<Uint64>(district_id));
      op->equal(OL_W_ID.c_str(), static_cast<Uint64>(warehouse_id));
      op->equal(OL_NUMBER.c_str(), static_cast<Uint64>(j));

      NdbRecAttr *ol_i_id = op->getValue(OL_I_ID.c_str(), NULL);
      RDMA_ASSERT(ol_i_id != nullptr);

      if(!pre_execute(tx)) {
        ndb.closeTransaction(tx);
        return false;
      }

      orderline_i_ids[item_id_index] = ol_i_id->u_32_value();
      if (PrintInfo)
        RDMA_LOG(3) << "GetOrderLine(): orderline_i_ids[: " << orderline_i_ids[item_id_index] << "~";
      item_id_index++;
    }
    order_id++;
  }
  
  return true;
}


// stock level get stock talbe
inline bool SLGetStock(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *stockTable,
                    uint warehouse_id, uint item_id,
                    uint &stock_quantity)
{
  auto op = get_op(tx, stockTable);
  // XD: use "lock_type" here
  op->readTuple(lock_mode);
    
  op->equal(S_I_ID.c_str(), static_cast<Uint64>(item_id));
  op->equal(S_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  NdbRecAttr *s_quantity = op->getValue(S_QUANTITY.c_str(), NULL);
  RDMA_ASSERT(s_quantity != nullptr);

  if(!pre_execute(tx)) {
    ndb.closeTransaction(tx);
    return false;
  }

  stock_quantity = s_quantity->u_32_value();
  if (PrintInfo)
    RDMA_LOG(5) << "stock_quantity : " << stock_quantity << "~";

  return true;
}


txn_result_t txn_stock_level(Ndb &ndb, FastRandom &rand) {

  const NdbDictionary::Dictionary *myDict= ndb.getDictionary();
  const NdbDictionary::Table *stockTable = myDict->getTable(::db::tpcc::STOCK_TABLE.c_str());
  const NdbDictionary::Table *districtTable= myDict->getTable(::db::tpcc::DISTRICT_TABLE.c_str());
  const NdbDictionary::Table *ordersTable= myDict->getTable(::db::tpcc::ORDERS_TABLE.c_str());
  const NdbDictionary::Table *orderlineTable= myDict->getTable(::db::tpcc::ORDERLINE_TABLE.c_str());
  RDMA_ASSERT(myDict != nullptr);
  RDMA_ASSERT(stockTable != nullptr);
  RDMA_ASSERT(districtTable != nullptr);
  RDMA_ASSERT(ordersTable != nullptr);
  RDMA_ASSERT(orderlineTable != nullptr);

  uint warehouse_id, district_id, level_thres;
  warehouse_id = PickWarehouseId(rand, 1, NumWarehouse);
  district_id = RandomNumber(rand, 1, NumDistrictsPerWarehouse);
  level_thres = RandomNumber(rand, 10, 20);
  if (PrintInfo) {
    RDMA_LOG(2) << "warehouse_id : " << warehouse_id << ".";
    RDMA_LOG(2) << "district_id : " << district_id << ".";
    RDMA_LOG(2) << "level_thres : " << level_thres << ".";
  }

  auto tx = tx_start(ndb);
  {
    uint district_next_o_id;
    bool success1 = SLGetDistrict(tx, ndb, districtTable,
                                  warehouse_id, district_id,district_next_o_id);
    if (!success1)
      RDMA_ASSERT(false);

    uint orderline_i_ids[300]; // allocate big enougth
    for (int i = 0; i < 300; i++)
      orderline_i_ids[i] = -1; // 4294967295 because it is unsigned
    bool success2 = SLGetOrderLine(tx,ndb,orderlineTable,ordersTable,
                                    district_id, warehouse_id, district_next_o_id,
                                    orderline_i_ids);
    if (!success2)
      RDMA_ASSERT(false);

    // use set to store DISTINCT item ids
    std::set<uint> item_ids_set;
    for (int i = 0; i < 300; i++) {
      if (orderline_i_ids[i] != 4294967295) // -1
        item_ids_set.insert(orderline_i_ids[i]);
    }

    uint stock_quantity;
    uint stock_level_cnt = 0;

    // XD: sort the stocks
    // sort to avoid deadlock
    std::vector<int> stockKeys;
    for (auto i_id : item_ids_set) {
      int key = i_id + warehouse_id * NumItems;
      stockKeys.push_back(key);
    }

    std::sort(stockKeys.begin(), stockKeys.end());

    for (auto k : stockKeys)
    {
      int i_id = k % NumItems;
      int w_id = k / NumItems;
      if (PrintInfo)
        RDMA_LOG(5) << "sorted key: iid wid: " << i_id << ", " << w_id;
      bool success3 = SLGetStock(tx,ndb,stockTable,
                                warehouse_id,i_id,
                                stock_quantity);
      if (!success3)
        RDMA_ASSERT(false);
      if (stock_quantity < level_thres)
        stock_level_cnt++;
    }
    if (PrintInfo)
      RDMA_LOG(5) << "num of item below threshold : " << stock_level_cnt;


    /* std::set<uint>::iterator it = item_ids_set.begin(); */
    /* while(it != item_ids_set.end()) { */
    /*   int item_id_distinct = *it; */
    /*   bool success3 = SLGetStock(tx,ndb,stockTable, */
    /*                             warehouse_id,item_id_distinct, */
    /*                             stock_quantity); */
    /*   if (!success3) */
    /*     RDMA_ASSERT(false); */
    /*   if (stock_quantity < level_thres) */
    /*     stock_level_cnt++; */
    /*   it++; */
    /* } */
    /* if (PrintInfo) */
    /*   RDMA_LOG(5) << "num of item below threshold : " << stock_level_cnt; */

    commit(tx);
  }

  ndb.closeTransaction(tx);
  return txn_result_t(true, 0);
}

} // end of tpcc

} // end of db
 