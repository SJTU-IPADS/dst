#include "all.h"

namespace deptran {

TpccRealDistChopper::TpccRealDistChopper() {
}

void TpccRealDistChopper::new_order_shard(const char *tb, const std::vector<Value> &input, uint32_t &site, int cnt) {
    MultiValue mv;
    if (tb == TPCC_TB_DISTRICT
     || tb == TPCC_TB_CUSTOMER
     || tb == TPCC_TB_ORDER
     || tb == TPCC_TB_NEW_ORDER
     || tb == TPCC_TB_ORDER_LINE)
        // based on d_id && w_id
        mv = MultiValue(std::vector<Value>({input[1], input[0]}));
    else if (tb == TPCC_TB_WAREHOUSE) {
        // use the same server as district
        tb = TPCC_TB_DISTRICT;
        mv = MultiValue(std::vector<Value>({input[1], input[0]}));
    }
    else if (tb == TPCC_TB_ITEM
          || tb == TPCC_TB_STOCK)
        // based on i_id && w_id
        mv = MultiValue(std::vector<Value>({input[4 + 3 * cnt], input[5 + 3 * cnt]}));
    else
        verify(0);
    int ret = Sharding::get_site_id(tb, mv, site);
    verify(ret == 0);
}

void TpccRealDistChopper::payment_shard(const char *tb, const std::vector<Value> &input, uint32_t &site) {
    MultiValue mv;
    if (tb == TPCC_TB_WAREHOUSE) {
        // use the same server as district
        tb = TPCC_TB_DISTRICT;
        mv = MultiValue(std::vector<Value>({input[1], input[0]}));
    }
    else if (tb == TPCC_TB_DISTRICT)
        // based on d_id && w_id
        mv = MultiValue(std::vector<Value>({input[1], input[0]}));
    else if (tb == TPCC_TB_CUSTOMER)
        // based on c_d_id && c_w_id
        mv = MultiValue(std::vector<Value>({input[4], input[3]}));
    else if (tb == TPCC_TB_HISTORY)
        // based on h_key
        mv = MultiValue(input[6]);
    else
        verify(0);
    int ret = Sharding::get_site_id(tb, mv, site);
    verify(ret == 0);
}

void TpccRealDistChopper::order_status_shard(const char *tb, const std::vector<mdb::Value> &input, uint32_t &site) {
    MultiValue mv;
    if (tb == TPCC_TB_CUSTOMER
     || tb == TPCC_TB_ORDER
     || tb == TPCC_TB_ORDER_LINE)
        // based on d_id && w_id
        mv = MultiValue(std::vector<Value>({input[1], input[0]}));
    else
        verify(0);
    int ret = Sharding::get_site_id(tb, mv, site);
    verify(ret == 0);
}

void TpccRealDistChopper::delivery_shard(const char *tb, const std::vector<mdb::Value> &input, uint32_t &site, int cnt) {
    MultiValue mv;
    if (tb == TPCC_TB_NEW_ORDER
     || tb == TPCC_TB_ORDER
     || tb == TPCC_TB_ORDER_LINE
     || tb == TPCC_TB_CUSTOMER)
        // based on d_id && w_id
        mv = MultiValue(std::vector<Value>({Value((i32)cnt), input[0]}));
    else
        verify(0);
    int ret = Sharding::get_site_id(tb, mv, site);
    verify(ret == 0);
}

void TpccRealDistChopper::stock_level_shard(const char *tb, const std::vector<mdb::Value> &input, uint32_t &site) {
    MultiValue mv;
    if (tb == TPCC_TB_DISTRICT
     || tb == TPCC_TB_ORDER_LINE)
        // based on d_id & w_id
        mv = MultiValue(std::vector<Value>({input[1], input[0]}));
    else if (tb == TPCC_TB_STOCK)
        // based on s_i_id & s_w_id
        mv = MultiValue(std::vector<Value>({input[0], input[1]}));
    else
        verify(0);
    int ret = Sharding::get_site_id(tb, mv, site);
    verify(ret == 0);
}

TpccRealDistChopper::~TpccRealDistChopper() {
}

}
 