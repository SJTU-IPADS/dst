#include "tpcc.hpp"
#include "util.hpp"

#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <set>

namespace db
{

namespace tpcc
{

// new order get warehouse
inline bool NOGetWarehouse(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *warehouseTable,
                           uint warehouse_id, double &warehouse_tax)
{
  auto op = get_op(tx, warehouseTable);
  // XD: replace it with a dirty OP, and not read it (to optimize performance)
  op->readTuple(NdbOperation::LM_CommittedRead);
  /* op->readTuple(NdbOperation::LM_Exclusive); */
    
  op->equal(W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  NdbRecAttr *w_tax = op->getValue(W_TAX.c_str(), NULL);
  RDMA_ASSERT(w_tax != nullptr);

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }

  warehouse_tax = w_tax->double_value();
  if (PrintInfo)
    RDMA_LOG(5) << "warehouse_tax : " << warehouse_tax << "~";

  return true;
}

// new order get & update district
inline bool NOGetUpdDistrict(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *districtTable,
                             uint district_id, uint district_w_id,
                             double &district_tax, uint &district_next_o_id)
{
  // read district tuple
  auto op = get_op(tx, districtTable);
  op->readTuple(NdbOperation::LM_Exclusive);

  op->equal(D_ID.c_str(), static_cast<Uint64>(district_id));
  op->equal(D_W_ID.c_str(), static_cast<Uint64>(district_w_id));

  NdbRecAttr *d_tax = op->getValue(D_TAX.c_str(), NULL);
  RDMA_ASSERT(d_tax != nullptr);
  NdbRecAttr *d_next_o_id = op->getValue(D_NEXT_O_ID.c_str(), NULL);
  RDMA_ASSERT(d_next_o_id != nullptr);

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }

  district_tax = d_tax->double_value();
  district_next_o_id = d_next_o_id->u_32_value();
  if (PrintInfo)
  {
    RDMA_LOG(5) << "district_tax : " << district_tax << "~";
    RDMA_LOG(3) << "district_next_o_id : " << district_next_o_id << "~";
  }

  // update district tuple
  auto op2 = get_op(tx, districtTable);
  op2->updateTuple();

  op2->equal(D_ID.c_str(), static_cast<Uint64>(district_id));
  op2->equal(D_W_ID.c_str(), static_cast<Uint64>(district_w_id));
  op2->setValue(D_NEXT_O_ID.c_str(), static_cast<Uint64>(district_next_o_id + 1));

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }

  if (PrintInfo)
    RDMA_LOG(5) << "update district next order id";

  return true;
}

// new order get customer
inline bool NOGetCust(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *customerTable,
                      uint customer_id, uint district_id, uint warehouse_id,
                      double &customer_discount, std::string &customer_last, std::string &customer_credit)
{
  auto op = get_op(tx, customerTable);
  op->readTuple(NdbOperation::LM_CommittedRead);
  /* op->readTuple(NdbOperation::LM_Exclusive); */

  op->equal(C_ID.c_str(), static_cast<Uint64>(customer_id));
  op->equal(C_D_ID.c_str(), static_cast<Uint64>(district_id));
  op->equal(C_W_ID.c_str(), static_cast<Uint64>(warehouse_id));

  NdbRecAttr *c_discount = op->getValue(C_DISCOUNT.c_str(), NULL);
  RDMA_ASSERT(c_discount != nullptr);
  NdbRecAttr *c_last = op->getValue(C_LAST.c_str(), NULL);
  RDMA_ASSERT(c_last != nullptr);
  NdbRecAttr *c_credit = op->getValue(C_CREDIT.c_str(), NULL);
  RDMA_ASSERT(c_credit != nullptr);

  /* if (!pre_execute(tx)) */
  /* { */
  /*   ndb.closeTransaction(tx); */
  /*   return false; */
  /* } */

  customer_discount = c_discount->double_value();
  get_string(c_last, customer_last);
  get_string(c_credit, customer_credit);
  if (PrintInfo)
  {
    RDMA_LOG(5) << "customer_discount : " << customer_discount << "~";
    RDMA_LOG(3) << "customer_last : " << customer_last << "~";
    RDMA_LOG(3) << "customer_credit : " << customer_credit << "~";
  }

  return true;
}

// new order insert new order table
inline bool NOInsertNewOrd(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *newOrdTable,
                           const NdbDictionary::Table *newOrdersIndexTable,
                           uint neword_o_id, uint neword_d_id, uint neword_w_id)
{
  auto op = get_op(tx, newOrdTable);
  op->insertTuple();
  op->setValue(NO_O_ID.c_str(), static_cast<Uint64>(neword_o_id));
  op->setValue(NO_D_ID.c_str(), static_cast<Uint64>(neword_d_id));
  op->setValue(NO_W_ID.c_str(), static_cast<Uint64>(neword_w_id));

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }

  if (PrintInfo)
    RDMA_LOG(5) << "insert into new order : " << neword_o_id << "," << neword_d_id << ","
                << neword_w_id << "~";

  // NOTE following is used to main order index 
  auto op2 = get_op(tx, newOrdersIndexTable);
  uint64_t up_id = (neword_w_id - 1) * 10 + neword_d_id-1;
  uint64_t low_id = neword_o_id - 1;
  uint64_t w_d_o = (uint64_t)(up_id << 32) | (uint64_t)low_id;
  op2->insertTuple();
  op2->setValue(W_D_O.c_str(), static_cast<Uint64>(w_d_o));

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }

  if (PrintInfo)
    RDMA_LOG(5) << "insert into new order index : " << w_d_o;
  return true;
}

// new order insert orders table
// TODO insert entry date
inline bool NOInsertOrd(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *ordersTable,
                        uint order_id, uint order_d_id, uint order_w_id, uint order_c_id,
                        uint order_ol_cnt, uint order_all_local)
{
  auto op = get_op(tx, ordersTable);
  op->insertTuple();
  op->setValue(O_ID.c_str(), static_cast<Uint64>(order_id));
  op->setValue(O_D_ID.c_str(), static_cast<Uint64>(order_d_id));
  op->setValue(O_W_ID.c_str(), static_cast<Uint64>(order_w_id));
  op->setValue(O_C_ID.c_str(), static_cast<Uint64>(order_c_id));
  op->setValue(O_OL_CNT.c_str(), static_cast<Uint64>(order_ol_cnt));
  op->setValue(O_ALL_LOCAL.c_str(), static_cast<Uint64>(order_all_local));

  /* if (!pre_execute(tx)) */
  /* { */
  /*   ndb.closeTransaction(tx); */
  /*   return false; */
  /* } */

  if (PrintInfo)
    RDMA_LOG(5) << "insert into orders : " << order_id << "," << order_d_id << ","
                << order_w_id << "," << order_c_id << "," << order_ol_cnt << "," << order_all_local << "~";
  return true;
}

// new order get item table
// TODO "not-found" confition in TX profile
inline bool NOGetItem(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *itemTable,
                      uint item_id,
                      std::string &item_name, double &item_price, std::string &item_data)
{
  auto op = get_op(tx, itemTable);
  op->readTuple(NdbOperation::LM_CommittedRead); // read dirty
  op->equal(I_ID.c_str(), static_cast<Uint64>(item_id));

  NdbRecAttr *i_name = op->getValue(I_NAME.c_str(), NULL);
  RDMA_ASSERT(i_name != nullptr);
  NdbRecAttr *i_price = op->getValue(I_PRICE.c_str(), NULL);
  RDMA_ASSERT(i_price != nullptr);
  NdbRecAttr *i_data = op->getValue(I_DATA.c_str(), NULL);
  RDMA_ASSERT(i_data != nullptr);

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }

  get_string(i_name, item_name);
  item_price = i_price->double_value();
  get_string(i_data, item_data);
  if (PrintInfo)
  {
    RDMA_LOG(5) << "item_name : " << item_name << "~";
    RDMA_LOG(3) << "item_price : " << item_price << "~";
    RDMA_LOG(3) << "item_data : " << item_data << "~";
  }

  return true;
}

// new order get update stock
inline bool NOGetUpdStock(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *stockTable,
                          uint stock_i_id, uint stock_w_id, uint orderline_quantity, bool isRemote,
                          uint &stock_quantity, std::string (&stock_dists)[10],
                          std::string &stock_data)
{
  double stock_ytd;
  uint stock_order_cnt, stock_remote_cnt;
  if (PrintInfo)
    RDMA_LOG(5) << "stock_i_id : " << stock_i_id << "~";

  // get from stock tuple
  auto op = get_op(tx, stockTable);
  op->readTuple(NdbOperation::LM_Exclusive);
  op->equal(S_I_ID.c_str(), static_cast<Uint64>(stock_i_id));
  op->equal(S_W_ID.c_str(), static_cast<Uint64>(stock_w_id));

  NdbRecAttr *s_dists_arr[10];
  for (int i = 0; i < 10; i++)
  {
    s_dists_arr[i] = op->getValue(S_DISTS[i].c_str(), NULL);
    RDMA_ASSERT(s_dists_arr[i] != nullptr);
  }
  NdbRecAttr *s_quantity = op->getValue(S_QUANTITY.c_str(), NULL);
  RDMA_ASSERT(s_quantity != nullptr);
  NdbRecAttr *s_data = op->getValue(S_DATA.c_str(), NULL);
  RDMA_ASSERT(s_data != nullptr);
  NdbRecAttr *s_order_cnt = op->getValue(S_ORDER_CNT.c_str(), NULL);
  RDMA_ASSERT(s_order_cnt != nullptr);
  NdbRecAttr *s_remote_cnt = op->getValue(S_REMOTE_CNT.c_str(), NULL);
  RDMA_ASSERT(s_remote_cnt != nullptr);
  NdbRecAttr *s_ytd = op->getValue(S_YTD.c_str(), NULL);
  RDMA_ASSERT(s_ytd != nullptr);

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }

  stock_quantity = s_quantity->u_32_value();
  if (PrintInfo)
    RDMA_LOG(5) << "stock_quantity : " << stock_quantity << "~";
  for (int i = 0; i < 10; i++)
  {
    get_string(s_dists_arr[i], stock_dists[i]);
    if (PrintInfo)
      RDMA_LOG(3) << "stock_dists[" << i << "] : " << stock_dists[i] << "~";
  }
  get_string(s_data, stock_data);
  stock_order_cnt = s_order_cnt->u_32_value();
  stock_remote_cnt = s_remote_cnt->u_32_value();
  stock_ytd = s_ytd->u_32_value();
  if (PrintInfo)
  {
    RDMA_LOG(3) << "stock_data : " << stock_data << "~";
    RDMA_LOG(3) << "stock_order_cnt : " << stock_order_cnt << "~";
    RDMA_LOG(3) << "stock_remote_cnt : " << stock_remote_cnt << "~";
    RDMA_LOG(3) << "stock_ytd : " << stock_ytd << "~";
  }

  // update stock tuple
  auto op2 = get_op(tx, stockTable);
  op2->updateTuple();
  op2->equal(S_I_ID.c_str(), static_cast<Uint64>(stock_i_id));
  op2->equal(S_W_ID.c_str(), static_cast<Uint64>(stock_w_id));

  if (PrintInfo)
    RDMA_LOG(5) << "update stock table s_quantity, s_ytd, s_remote_cnt | s_order_cnt";

  if (stock_quantity - orderline_quantity >= 10)
  {
    op2->setValue(S_QUANTITY.c_str(), static_cast<Uint64>(stock_quantity - orderline_quantity));
    if (PrintInfo)
      RDMA_LOG(5) << "stock_quantity : " << stock_quantity - orderline_quantity;
  }
  else
  {
    op2->setValue(S_QUANTITY.c_str(), static_cast<Uint64>(stock_quantity - orderline_quantity + 91));
    if (PrintInfo)
      RDMA_LOG(5) << "stock_quantity : " << stock_quantity - orderline_quantity + 91;
  }
  op2->setValue(S_YTD.c_str(), stock_ytd + orderline_quantity);
  if (PrintInfo)
    RDMA_LOG(5) << "stock_ytd: " << stock_ytd + orderline_quantity;
  if (isRemote)
  {
    op2->setValue(S_REMOTE_CNT.c_str(), static_cast<Uint64>(stock_remote_cnt + 1));
    if (PrintInfo)
      RDMA_LOG(5) << "stock_remote_cnt: " << stock_remote_cnt + 1;
  }
  else
  {
    op2->setValue(S_ORDER_CNT.c_str(), static_cast<Uint64>(stock_order_cnt + 1));
    if (PrintInfo)
      RDMA_LOG(5) << "stock_order_cnt: " << stock_order_cnt + 1;
  }

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }

  return true;
}

// new order insert order line
// TODO insert date and district info
inline bool NOInsertOrdLn(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *orderlineTable,
                          uint orderline_o_id, uint orderline_d_id, uint orderline_w_id,
                          uint orderline_number, uint orderline_i_id, uint orderline_supply_w_id,
                          uint orderline_quantity, double orderline_amount)
{
  auto op = get_op(tx, orderlineTable);
  op->insertTuple();
  op->setValue(OL_O_ID.c_str(), static_cast<Uint64>(orderline_o_id));
  op->setValue(OL_D_ID.c_str(), static_cast<Uint64>(orderline_d_id));
  op->setValue(OL_W_ID.c_str(), static_cast<Uint64>(orderline_w_id));
  op->setValue(OL_NUMBER.c_str(), static_cast<Uint64>(orderline_number));
  op->setValue(OL_I_ID.c_str(), static_cast<Uint64>(orderline_i_id));
  op->setValue(OL_SUPPLY_W_ID.c_str(), static_cast<Uint64>(orderline_supply_w_id));
  op->setValue(OL_QUANTITY.c_str(), static_cast<Uint64>(orderline_quantity));
  op->setValue(OL_AMOUNT.c_str(), orderline_amount);

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }
  if (PrintInfo)
    RDMA_LOG(5) << "insert into order line: " << orderline_o_id << "," << orderline_d_id << ","
                << orderline_w_id << "," << orderline_number << "," << orderline_i_id
                << "," << orderline_supply_w_id << "," << orderline_quantity << ","
                << orderline_amount << "~";
  return true;
}

// new order update order index talbe
inline bool NOUpdOrdIndex(NdbTransaction *tx, Ndb &ndb, const NdbDictionary::Table *ordersIndexTable,
                          uint c_d_w, uint rec_ord_id)
{
  auto op = get_op(tx, ordersIndexTable);
  op->updateTuple();
  op->equal(C_D_W.c_str(), static_cast<Uint64>(c_d_w));
  op->setValue(RECENT_ORDER_ID.c_str(), rec_ord_id);

  if (PrintInfo)
    RDMA_LOG(5) << "update orders_index table: " << c_d_w << ", " << rec_ord_id;

  if (!pre_execute(tx))
  {
    ndb.closeTransaction(tx);
    return false;
  }
  return true;
}

txn_result_t txn_new_ord(Ndb &ndb, FastRandom &rand)
{

  const NdbDictionary::Dictionary *myDict = ndb.getDictionary();
  const NdbDictionary::Table *stockTable = myDict->getTable(::db::tpcc::STOCK_TABLE.c_str());
  const NdbDictionary::Table *warehouseTable = myDict->getTable(::db::tpcc::WAREHOUSE_TABLE.c_str());
  const NdbDictionary::Table *districtTable = myDict->getTable(::db::tpcc::DISTRICT_TABLE.c_str());
  const NdbDictionary::Table *customerTable = myDict->getTable(::db::tpcc::CUSTOMER_TABLE.c_str());
  const NdbDictionary::Table *newOrdTable = myDict->getTable(::db::tpcc::NEW_ORDER_TABLE.c_str());
  const NdbDictionary::Table *newOrdIndexTable = myDict->getTable(::db::tpcc::NEW_ORDERS_INDEX.c_str());
  const NdbDictionary::Table *ordersTable = myDict->getTable(::db::tpcc::ORDERS_TABLE.c_str());
  const NdbDictionary::Table *itemTable = myDict->getTable(::db::tpcc::ITEM_TABLE.c_str());
  const NdbDictionary::Table *orderlineTable = myDict->getTable(::db::tpcc::ORDERLINE_TABLE.c_str());
  const NdbDictionary::Table *ordersIndexTable = myDict->getTable(::db::tpcc::ORDERS_INDEX.c_str());
  RDMA_ASSERT(myDict != nullptr);
  RDMA_ASSERT(stockTable != nullptr);
  RDMA_ASSERT(warehouseTable != nullptr);
  RDMA_ASSERT(districtTable != nullptr);
  RDMA_ASSERT(customerTable != nullptr);
  RDMA_ASSERT(newOrdTable != nullptr);
  RDMA_ASSERT(ordersTable != nullptr);
  RDMA_ASSERT(itemTable != nullptr);
  RDMA_ASSERT(orderlineTable != nullptr);
  RDMA_ASSERT(ordersIndexTable != nullptr);
  RDMA_ASSERT(newOrdIndexTable != nullptr);

  uint warehouse_id, district_id, customer_id;
  warehouse_id = PickWarehouseId(rand, 1, NumWarehouse);
  district_id = RandomNumber(rand, 1, NumDistrictsPerWarehouse);
  customer_id = GetCustomerId(rand);
  if (PrintInfo)
  {
    RDMA_LOG(2) << "warehouse_id : " << warehouse_id << ".";
    RDMA_LOG(2) << "district_id : " << district_id << ".";
    RDMA_LOG(2) << "customer_id : " << customer_id << ".";
  }

  // order properties
  uint order_ol_cnt = RandomNumber(rand, 5, 15);
  uint order_all_local = 1;

  // order line properties (item properties)
  uint orderline_i_ids[order_ol_cnt];
  uint orderline_supply_w_ids[order_ol_cnt];
  uint orderline_quantitys[order_ol_cnt];
  /* uint orderline_dist_infos[order_ol_cnt]; */

  for (int i = 1; i <= order_ol_cnt; i++)
  {
    orderline_i_ids[i - 1] = GetItemId(rand);
    orderline_quantitys[i - 1] = RandomNumber(rand, 1, 10);
    if (RandomNumber(rand, 1, 100) == 1)
    {
      orderline_supply_w_ids[i - 1] = GetOtherWarehouse(warehouse_id, rand);
      order_all_local = 0;
    }
    else
    {
      orderline_supply_w_ids[i - 1] = warehouse_id;
    }
  }

  // TODO 1% rollback
  auto tx = tx_start(ndb);
  {
    double warehouse_tax;
    bool success1 = NOGetWarehouse(tx, ndb, warehouseTable, warehouse_id, warehouse_tax);
    if (!success1)
      RDMA_ASSERT(false);

    uint district_next_o_id;
    double district_tax;
    bool success2 = NOGetUpdDistrict(tx, ndb, districtTable,
                                     district_id, warehouse_id,
                                     district_tax, district_next_o_id);
    if (!success2)
      RDMA_ASSERT(false);

    double customer_discount;
    std::string customer_last, customer_credit;
    // XD: like warehouse, use dirty, and not pre-execute
    bool success3 = NOGetCust(tx, ndb, customerTable,
                              customer_id, district_id, warehouse_id,
                              customer_discount, customer_last, customer_credit);
    if (!success3)
      RDMA_ASSERT(false);

    bool success4 = NOInsertNewOrd(tx, ndb, newOrdTable, newOrdIndexTable,
                                   district_next_o_id, district_id, warehouse_id);

    // XD: remove the pre-execute in this insert
    if (!success4)
      RDMA_ASSERT(false);

    // XD: remove the pre-execute in this insert
    bool success5 = NOInsertOrd(tx, ndb, ordersTable,
                                district_next_o_id, district_id, warehouse_id, customer_id,
                                order_ol_cnt, order_all_local);
    if (!success5)
      RDMA_ASSERT(false);

    std::string item_names[order_ol_cnt];
    double item_prices[order_ol_cnt];
    std::string item_datas[order_ol_cnt];
    uint stock_quantity;
    std::string stock_data;
    std::string stock_dists[10];
    bool success6 = false, success7 = false, success8 = false, success9 = false;

    /** optimize this:
        XD: using two loops: one to update stocks, one to insert orderline
        1. sort the stock accroding to their keys
        2. for (auto k : stocks) {
               update_stock(k);
               pre_execute();
           }
        3. for (auto k : orderlines) {
               insert_orderline(k);
        }
        commit();
    */
    for (int i = 1; i <= order_ol_cnt; i++)
    {
      success6 = NOGetItem(tx, ndb, itemTable,
                           orderline_i_ids[i - 1], item_names[i - 1], item_prices[i - 1], item_datas[i - 1]);
      if (!success6)
        RDMA_ASSERT(false);
    }


    // sort to avoid deadlock
    std::vector<int> stockKeys;
    for (int i = 1; i <= order_ol_cnt; i++)
    {
      int key = orderline_i_ids[i-1] + warehouse_id * NumItems;
      stockKeys.push_back(key);
    }

    std::sort(stockKeys.begin(), stockKeys.end());

    int counter = 0;
    for (auto k : stockKeys)
    {
      int i_id = k % NumItems;
      int w_id = k / NumItems;
      if (PrintInfo)
        RDMA_LOG(5) << "sorted key: iid wid: " << i_id << ", " << w_id;
      bool isRemote = !(warehouse_id == orderline_supply_w_ids[counter]);
      success7 = NOGetUpdStock(tx, ndb, stockTable,
                               i_id, warehouse_id, orderline_quantitys[counter],
                               isRemote, stock_quantity, stock_dists, stock_data);
      if (!success7)
        RDMA_ASSERT(false);
      counter++;
    }

    /* for (int i = 1; i <= order_ol_cnt; i++) */
    /* { */
    /*   bool isRemote = !(warehouse_id == orderline_supply_w_ids[i - 1]); */
    /*   success7 = NOGetUpdStock(tx, ndb, stockTable, */
    /*                            orderline_i_ids[i - 1], warehouse_id, orderline_quantitys[i - 1], */
    /*                            isRemote, stock_quantity, stock_dists, stock_data); */
    /*   if (!success7) */
    /*     RDMA_ASSERT(false); */
    /*   // TODO "ORIGINAL" brand-generic blah blah */
    /* } */


    for (int i = 1; i <= order_ol_cnt; i++)
    {
      double orderline_amount = orderline_quantitys[i - 1] * item_prices[i - 1];
      success8 = NOInsertOrdLn(tx, ndb, orderlineTable,
                               district_next_o_id, district_id, warehouse_id,
                               i, orderline_i_ids[i - 1], orderline_supply_w_ids[i - 1],
                               orderline_quantitys[i - 1], orderline_amount);

      if (!success8)
        RDMA_ASSERT(false);

      // TODO why TX profile bother about total amout???
      //XD: skip this, no need to concern
    }

    // update order index table
    int c_d_w = customer_id - 1 + (district_id - 1) * 3000 + (warehouse_id - 1) * 30000;
    success9 = NOUpdOrdIndex(tx, ndb, ordersIndexTable,
                             c_d_w, district_next_o_id);
    if (!success9)
      RDMA_ASSERT(false);

    commit(tx);
  }

  ndb.closeTransaction(tx);
  return txn_result_t(true, 0);
}

} // end of namespace tpcc

} // end of namespace db
 