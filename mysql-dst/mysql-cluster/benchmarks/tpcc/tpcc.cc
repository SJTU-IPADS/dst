#include "tpcc.hpp"
#include "util.hpp"

#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <set>

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

namespace db {

extern int client_id;

static const int deadlock_code = 266;

namespace tpcc {

// database name
const std::string db_name = "tpcc1000";


// table names
const std::string STOCK_TABLE = "stock";
const std::string WAREHOUSE_TABLE = "warehouse";
const std::string DISTRICT_TABLE = "district";
const std::string CUSTOMER_TABLE = "customer";
const std::string NEW_ORDER_TABLE = "new_orders";
const std::string ORDERS_TABLE = "orders";
const std::string ORDERLINE_TABLE = "order_line";
const std::string ITEM_TABLE = "item";
const std::string HISTORY_TABLE = "history";
const std::string ORDERS_INDEX = "orders_index";
const std::string NEW_ORDERS_INDEX = "new_orders_index";


// stock table columns
// primary key
const std::string S_I_ID = "s_i_id";
const std::string S_W_ID = "s_w_id";
// others
const std::string S_QUANTITY = "s_quantity";
const std::string S_DISTS[] = { "s_dist_01",
                                "s_dist_02",
                                "s_dist_03",
                                "s_dist_04",
                                "s_dist_05",
                                "s_dist_06",
                                "s_dist_06",
                                "s_dist_07",
                                "s_dist_08",
                                "s_dist_09",
                                "s_dist_10" };
const std::string S_YTD = "s_ytd";
const std::string S_ORDER_CNT = "s_order_cnt";
const std::string S_REMOTE_CNT = "s_remote_cnt";
const std::string S_DATA = "s_data";


// warehouse table columns
// primary key
const std::string W_ID = "w_id";
// others
const std::string W_NAME = "w_name";
const std::string W_STREET_1 = "w_street_1";
const std::string W_STREET_2 = "w_street_2";
const std::string W_CITY = "w_city";
const std::string W_STATE = "w_state";
const std::string W_ZIP = "w_zip";
const std::string W_TAX = "w_tax";
const std::string W_YTD = "w_ytd";


// district table columns
// primary key
const std::string D_ID = "d_id";
const std::string D_W_ID = "d_w_id";
// others
const std::string D_NAME = "d_name";
const std::string D_STREET_1 = "d_street_1";
const std::string D_STREET_2 = "d_street_2";
const std::string D_CITY = "d_city";
const std::string D_STATE = "d_state";
const std::string D_ZIP = "d_zip";
const std::string D_TAX = "d_tax";
const std::string D_YTD = "d_ytd";
const std::string D_NEXT_O_ID = "d_next_o_id";


// customer table columns
// primary key
const std::string C_ID = "c_id";
const std::string C_D_ID = "c_d_id";
const std::string C_W_ID = "c_w_id";
// others
const std::string C_FIRST = "c_first";
const std::string C_MIDDLE = "c_middle";
const std::string C_LAST = "c_last";
const std::string C_STREET_1 = "c_street_1";
const std::string C_STREET_2 = "c_street_2";
const std::string C_CITY = "c_city";
const std::string C_STATE = "c_state";
const std::string C_ZIP = "c_zip";
const std::string C_PHONE = "c_phone";
const std::string C_SINCE = "c_since";
const std::string C_CREDIT = "c_credit";
const std::string C_CREDIT_LIM = "c_credit_lim";
const std::string C_DISCOUNT = "c_discount";
const std::string C_BALANCE = "c_balance";
const std::string C_YTD_PAYMENT = "c_ytd_payment";
const std::string C_PAYMENT_CNT = "c_payment_cnt";
const std::string C_DELIVERY_CNT = "c_delivery_cnt";


// new orders table columns
// primary key
const std::string NO_O_ID = "no_o_id";
const std::string NO_D_ID = "no_d_id";
const std::string NO_W_ID = "no_w_id";


// orders table columns
// primary key
const std::string O_ID = "o_id";
const std::string O_D_ID = "o_d_id";
const std::string O_W_ID = "o_w_id";
// others
const std::string O_C_ID = "o_c_id";
const std::string O_ENTRY_D = "o_entry_d";
const std::string O_CARRIER_ID = "o_carrier_id";
const std::string O_OL_CNT = "o_ol_cnt";
const std::string O_ALL_LOCAL = "o_all_local";


// order_line table columns
// primary key
const std::string OL_O_ID = "ol_o_id";
const std::string OL_D_ID = "ol_d_id";
const std::string OL_W_ID = "ol_w_id";
const std::string OL_NUMBER = "ol_number";
// others
const std::string OL_I_ID = "ol_i_id";
const std::string OL_SUPPLY_W_ID = "ol_supply_w_id";
const std::string OL_QUANTITY = "ol_quantity";
const std::string OL_AMOUNT = "ol_amount";
const std::string OL_DELIVERY_D = "ol_delivery_d";
const std::string OL_DIST_INFO = "ol_dist_info";

// item table columns
// primary key
const std::string I_ID = "i_id";
// others
const std::string I_NAME = "i_name";
const std::string I_PRICE = "i_price";
const std::string I_DATA = "i_data";

// history table columns
const std::string H_C_ID = "h_c_id";
const std::string H_C_D_ID = "h_c_d_id";
const std::string H_C_W_ID = "h_c_w_id";
const std::string H_D_ID = "h_d_id";
const std::string H_W_ID = "h_w_id";
const std::string H_AMOUNT = "h_amount";
const std::string H_DATA = "h_data";


// orders_index table columns
const std::string C_D_W = "c_d_w";
const std::string RECENT_ORDER_ID = "recent_order_id";

// new_orders_index table columns
const std::string W_D_O = "w_d_o";

size_t NumWarehouse = 10;
size_t NumDistrictsPerWarehouse = 10;
size_t NumCustomersPerDistrict = 3000;
size_t NumItems = 100000;

const int NumOldOrders = 20;


const bool PrintInfo = false;

const double ORDER_STATUS_RATIO = 0.04f;
const double NEW_ORDER_RATIO = 0.45f;
const double STOCK_LEVEL_RATIO = 0.04f;
const double PAYMENT_RATIO = 0.43f;
const double DELIVERY_RATIO = 0.04f;

void parse_config() {
  RDMA_LOG(4) << "use lock mode " << lock_mode;
}

workload_desc_vec_t generate_test_funcs() {
  parse_config();
  auto ret = workload_desc_vec_t();
  ret.emplace_back("read", ORDER_STATUS_RATIO, txn_order_status);
  ret.emplace_back("update", NEW_ORDER_RATIO, txn_new_ord);
  ret.emplace_back("read", STOCK_LEVEL_RATIO, txn_stock_level);
  ret.emplace_back("update", PAYMENT_RATIO, txn_payment);
  ret.emplace_back("update", DELIVERY_RATIO, txn_delivery);
  return ret;
}


} // end namespace tpcc

} // end namespace db
 