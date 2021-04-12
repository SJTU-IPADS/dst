#pragma once
/**
 * This is the tpcc workload we used.
 */

#include "tx_workload.hpp"
#include "db_helper.hpp"

extern NdbOperation::LockMode lock_mode;

namespace  db {

namespace tpcc {

// database name
extern const std::string db_name ;


// table names
extern const std::string STOCK_TABLE ;
extern const std::string WAREHOUSE_TABLE ;
extern const std::string DISTRICT_TABLE ;
extern const std::string CUSTOMER_TABLE ;
extern const std::string NEW_ORDER_TABLE ;
extern const std::string ORDERS_TABLE ;
extern const std::string ORDERLINE_TABLE ;
extern const std::string ITEM_TABLE ;
extern const std::string HISTORY_TABLE ;
extern const std::string ORDERS_INDEX ;
extern const std::string NEW_ORDERS_INDEX ;


// stock table columns
// primary key
extern const std::string S_I_ID ;
extern const std::string S_W_ID ;
// others
extern const std::string S_QUANTITY ;
extern const std::string S_DISTS[] ;
extern const std::string S_YTD ;
extern const std::string S_ORDER_CNT ;
extern const std::string S_REMOTE_CNT ;
extern const std::string S_DATA ;


// warehouse table columns
// primary key
extern const std::string W_ID ;
// others
extern const std::string W_NAME ;
extern const std::string W_STREET_1 ;
extern const std::string W_STREET_2 ;
extern const std::string W_CITY ;
extern const std::string W_STATE ;
extern const std::string W_ZIP ;
extern const std::string W_TAX ;
extern const std::string W_YTD ;


// district table columns
// primary key
extern const std::string D_ID ;
extern const std::string D_W_ID ;
// others
extern const std::string D_NAME ;
extern const std::string D_STREET_1 ;
extern const std::string D_STREET_2 ;
extern const std::string D_CITY ;
extern const std::string D_STATE ;
extern const std::string D_ZIP ;
extern const std::string D_TAX ;
extern const std::string D_YTD ;
extern const std::string D_NEXT_O_ID ;


// customer table columns
// primary key
extern const std::string C_ID ;
extern const std::string C_D_ID ;
extern const std::string C_W_ID ;
// others
extern const std::string C_FIRST ;
extern const std::string C_MIDDLE ;
extern const std::string C_LAST ;
extern const std::string C_STREET_1 ;
extern const std::string C_STREET_2 ;
extern const std::string C_CITY ;
extern const std::string C_STATE ;
extern const std::string C_ZIP ;
extern const std::string C_PHONE ;
extern const std::string C_SINCE ;
extern const std::string C_CREDIT ;
extern const std::string C_CREDIT_LIM ;
extern const std::string C_DISCOUNT ;
extern const std::string C_BALANCE ;
extern const std::string C_YTD_PAYMENT ;
extern const std::string C_PAYMENT_CNT ;
extern const std::string C_DELIVERY_CNT;


// new orders table columns
// primary key
extern const std::string NO_O_ID ;
extern const std::string NO_D_ID ;
extern const std::string NO_W_ID ;


// orders table columns
// primary key
extern const std::string O_ID ;
extern const std::string O_D_ID ;
extern const std::string O_W_ID ;
// others
extern const std::string O_C_ID ;
extern const std::string O_ENTRY_D ;
extern const std::string O_CARRIER_ID ;
extern const std::string O_OL_CNT ;
extern const std::string O_ALL_LOCAL ;


// orders_index table columns
extern const std::string C_D_W ;
extern const std::string RECENT_ORDER_ID ;

// new orders index talbe columns
extern const std::string W_D_O ;

// order_line table columns
// primary key
extern const std::string OL_O_ID ;
extern const std::string OL_D_ID ;
extern const std::string OL_W_ID ;
extern const std::string OL_NUMBER ;
// others
extern const std::string OL_I_ID ;
extern const std::string OL_SUPPLY_W_ID ;
extern const std::string OL_QUANTITY ;
extern const std::string OL_AMOUNT ;
extern const std::string OL_DELIVERY_D ;
extern const std::string OL_DIST_INFO ;

// item table columns
// primary key
extern const std::string I_ID ;
// others
extern const std::string I_NAME ;
extern const std::string I_PRICE ;
extern const std::string I_DATA ;

// history table columns
extern const std::string H_C_ID ;
extern const std::string H_C_D_ID ;
extern const std::string H_C_W_ID ;
extern const std::string H_D_ID ;
extern const std::string H_W_ID ;
extern const std::string H_AMOUNT ;
extern const std::string H_DATA ;


extern size_t NumWarehouse; 
extern size_t NumDistrictsPerWarehouse ;
extern size_t NumCustomersPerDistrict ;
extern size_t NumItems ;

extern const bool PrintInfo;
extern const int NumOldOrders;


txn_result_t txn_order_status(Ndb &myNdb,FastRandom &);
txn_result_t txn_new_ord(Ndb &myNdb,FastRandom &);
txn_result_t txn_stock_level(Ndb &myNdb,FastRandom &);
txn_result_t txn_payment(Ndb &myNdb,FastRandom &);
txn_result_t txn_delivery(Ndb &myNdb,FastRandom &);

unsigned PickWarehouseId(FastRandom &r, unsigned start, unsigned end);
int RandomNumber(FastRandom &r, int min, int max);
int NonUniformRandom(FastRandom &, int A, int C, int min, int max);
int GetCustomerId(FastRandom &r);
int GetItemId(FastRandom &r);
uint GetOtherWarehouse(uint w_id, FastRandom &r);


inline unsigned PickWarehouseId(FastRandom &r, unsigned start, unsigned end)
{
    const unsigned diff = end - start;
    return (r.next() % diff) + start;
}

inline int RandomNumber(FastRandom &r, int min, int max)
{
    return ((int) (r.next_uniform() * (max - min + 1) + min));
}

inline int NonUniformRandom(FastRandom &r, int A, int C, int min, int max)
{
  return (((RandomNumber(r, 0, A) | RandomNumber(r, min, max)) + C) % (max - min + 1)) + min;
}

inline int GetCustomerId(FastRandom &r)
{
  return NonUniformRandom(r, 1023, 554, 1, NumCustomersPerDistrict);
}

inline int GetItemId(FastRandom &r)
{
  return NonUniformRandom(r, 8191, 7911, 1, NumItems);
}

// TODO something wrong always remote stock
inline uint GetOtherWarehouse(uint warehouse_id, FastRandom &r) {
  int num_warehouses = NumWarehouse;
  uint other = -1;
  while ((other = RandomNumber(r, 1, num_warehouses)) == warehouse_id)
    ;
  return other;
}


void parse_config();

workload_desc_vec_t generate_test_funcs();

} // end namespace tpcc

} // end namespace db
 