#ifndef TPCC_CHOPPER_H_
#define TPCC_CHOPPER_H_

#include "memdb/value.h"
#include <vector>

namespace rococo {

class TxnChopper;
class TxnRequest;
class BatchStartArgsHelper;

class TpccChopper : public TxnChopper {
protected:
    typedef struct {
        size_t ol_cnt;
        bool piece_0_dist;
        bool *piece_items;
        bool *piece_stocks;
    } new_order_dep_t;

    typedef struct {
        bool piece_warehouse;
        bool piece_district;
        bool piece_last2id;
        bool piece_ori_last2id;
    } payment_dep_t;

    typedef struct {
        bool piece_last2id;
        bool piece_ori_last2id;
        bool piece_order;
    } order_status_dep_t;

    //typedef struct {
    //    size_t d_cnt;
    //    bool *piece_new_orders;
    //    bool *piece_orders;
    //    bool *piece_order_lines;
    //} delivery_dep_t;
    typedef struct {
        bool piece_new_orders;
        bool piece_orders;
        bool piece_order_lines;
    } delivery_dep_t;

    typedef struct {
        int threshold;
        int w_id;
    } stock_level_dep_t;

    union {
        new_order_dep_t new_order_dep_;
        payment_dep_t payment_dep_;
        order_status_dep_t order_status_dep_;
        delivery_dep_t delivery_dep_;
        stock_level_dep_t stock_level_dep_;
    };

    // new order
    virtual void new_order_shard(const char *tb, 
            const std::vector<mdb::Value> &input, 
            uint32_t &site, int cnt = 0);

    virtual void new_order_init(TxnRequest &req);

    virtual bool new_order_callback(int pi, int res, 
            const Value *output, uint32_t output_size);

    virtual void new_order_retry();

    // payment
    virtual void payment_shard(const char *tb, 
            const std::vector<mdb::Value> &input, uint32_t &site);
    
    virtual void payment_init(TxnRequest &req);

    virtual bool payment_callback(int pi, int res, 
            const Value *output, uint32_t output_size);

    virtual void payment_retry();

    // stock level
    virtual void stock_level_shard(const char *tb, 
            const std::vector<mdb::Value> &input, uint32_t &site);

    virtual void stock_level_init(TxnRequest &req);

    virtual bool stock_level_callback(int pi, int res, 
            const Value *output, uint32_t output_size);

    virtual void stock_level_retry();

    // delivery
    virtual void delivery_shard(const char *tb, 
            const std::vector<mdb::Value> &input, 
            uint32_t &site, int cnt);
    
    virtual void delivery_init(TxnRequest &req);

    virtual bool delivery_callback(int pi, int res, 
            const Value *output, uint32_t output_size);

    virtual void delivery_retry();

    // order status
    virtual void order_status_shard(const char *tb, 
            const std::vector<mdb::Value> &input, uint32_t &site);

    virtual void order_status_init(TxnRequest &req);

    virtual bool order_status_callback(int pi, int res, 
            const Value *output, uint32_t output_size);

    virtual void order_status_retry();

public:
    TpccChopper();

    virtual void init(TxnRequest &req);

    virtual bool start_callback(const std::vector<int> &pi, 
            int res, BatchStartArgsHelper &bsah);

    virtual bool start_callback(int pi, int res, 
            const std::vector<mdb::Value> &output);

    virtual bool is_read_only();

    virtual void retry();

    virtual ~TpccChopper();
};

}

#endif // TPCC_CHOPPER_H_
 