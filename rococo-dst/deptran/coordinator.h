#ifndef COORDINATOR_H_
#define COORDINATOR_H_

#include "all.h"

namespace rococo {

class ClientControlServiceImpl;

/**
* TODO: Replace the old coordinator with the new DTxnCoordinator
*/
class DTxnCoo {
public:

};

class RO6DTxnCoo: public DTxnCoo {
public:
    std::set<i64> ro_txn_; // All the read only transactions I have observed.
};


class Coordinator: public RO6DTxnCoo {
public:

    struct deptran_batch_start_t {
        std::vector<RequestHeader> headers;
        std::vector<std::vector<Value>> inputs;
        std::vector<int> output_sizes;
        std::vector<int> pis;
        rrr::FutureAttr fuattr;
    };

    rrr::PollMgr *rpc_poll_;
    std::vector<rrr::Client*> vec_rpc_cli_;
    std::vector<RococoProxy *> vec_rpc_proxy_;

    uint32_t coo_id_;
    int benchmark_;
    int32_t mode_;
    ClientControlServiceImpl *ccsi_;
    uint32_t thread_id_;
    bool batch_optimal_;
    bool retry_wait_;

    std::atomic<uint64_t> next_pie_id_;
    std::atomic<uint64_t> next_txn_id_;

    Mutex mtx_;

    std::vector<int> site_prepare_;
    std::vector<int> site_commit_;
    std::vector<int> site_abort_;
    std::vector<int> site_piece_;
#ifdef TXN_STAT
    typedef struct txn_stat_t {
        uint64_t n_serv_tch;
        uint64_t n_txn;
        std::unordered_map<int32_t, uint64_t> piece_cnt;
        txn_stat_t() : n_serv_tch(0), n_txn(0) {}
        void one(uint64_t _n_serv_tch, const std::vector<int32_t> &pie) {
            n_serv_tch += _n_serv_tch;
            n_txn++;
            for (int i = 0; i < pie.size(); i++) {
                if (pie[i] != 0) {
                    auto it = piece_cnt.find(pie[i]);
                    if (it == piece_cnt.end()) {
                        piece_cnt[pie[i]] = 1;
                    } else {
                        piece_cnt[pie[i]]++;
                    }
                }
            }
        }
        void output() {
            Log::info("SERV_TCH: %lu, TXN_CNT: %lu, MEAN_SERV_TCH_PER_TXN: %lf",
                    n_serv_tch, n_txn, ((double)n_serv_tch) / n_txn);
            for (auto &it : piece_cnt) {
                Log::info("\tPIECE: %d, PIECE_CNT: %lu, MEAN_PIECE_PER_TXN: %lf",
                        it.first, it.second, ((double)it.second) / n_txn);
            }
        }
    } txn_stat_t;
    std::unordered_map<int32_t, txn_stat_t> txn_stats_;
#endif
    Recorder *recorder_;
    Coordinator(uint32_t coo_id,
                std::vector<std::string>& addrs,
                int benchmark,
                int32_t mode = MODE_OCC,
                ClientControlServiceImpl *ccsi = NULL,
                uint32_t thread_id = 0,
                bool batch_optimal = false);

    virtual ~Coordinator() {
        for (int i = 0; i < site_prepare_.size(); i++) {
            Log::info("Coo: %u, Site: %d, piece: %d, "
                      "prepare: %d, commit: %d, abort: %d",
                      coo_id_, i, site_piece_[i], site_prepare_[i],
                      site_commit_[i], site_abort_[i]);
        }
        if (recorder_)
            delete recorder_;
#ifdef TXN_STAT
        for (auto &it : txn_stats_) {
            Log::info("TXN: %d", it.first);
            it.second.output();
        }
#endif
        // TODO (shuai) destroy all the rpc clients and proxies.
    }

    /** thread safe */
    uint64_t next_pie_id() {
        return this->next_pie_id_++;
    }

    /** thread safe */
    uint64_t next_txn_id() {
        return this->next_txn_id_++;
    }

    /** return: SUCCESS or REJECT */
    //virtual int do_one() = 0;

    ///** return: SUCCESS or REJECT */
    //virtual int do_one(uint32_t &max_try) = 0; /** 0 TRY UNTIL SUCCESS; >=1 TRY TIMES */

    /** do it asynchronously, thread safe.
     */
    void do_one(TxnRequest&);

    void restart(TxnChopper *ch);

    void start(TxnChopper* ch);

    void rpc_null_start(TxnChopper *ch);

    void naive_batch_start(TxnChopper *ch);

    void batch_start(TxnChopper* ch);

    void prepare(TxnChopper* ch);

    void finish(TxnChopper* ch);

    void occ_start(TxnChopper* ch) {};

    void occ_prepare(TxnChopper* ch) {};

    void occ_finish(TxnChopper* ch) {};

    void deptran_start(TxnChopper* ch);

    void deptran_batch_start(TxnChopper *ch);

    void deptran_finish(TxnChopper* ch);

    void deptran_start_ro(TxnChopper* ch);

    void deptran_finish_ro(TxnChopper* ch);

    void ro6_start_ro(TxnChopper* ch);

    RequestHeader gen_header(TxnChopper* ch);

    BatchRequestHeader gen_batch_header(TxnChopper* ch);

    void report(TxnReply &txn_reply,
            double last_latency
#ifdef TXN_STAT
            , TxnChopper *ch
#endif
            );

    /* deprecated*/
    bool next_piece(std::vector<mdb::Value>** input,
                    RococoProxy **proxy,
                    int *pi,
                    int *p_type) {
        return false;
    }

    void start_callback(TxnRequest *req, int pi, int res,
                        std::vector<mdb::Value>& output) {}

    int exe_txn(){return 0;}
};

}

#endif // COORDINATOR_H_
 