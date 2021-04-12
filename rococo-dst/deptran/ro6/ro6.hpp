#pragma once

//namespace rococo {

class RO6DTxn : public RCCDTxn {
private:
    i64 txnId = tid_;
    std::set<i64> ro_;
    // for remembering row and col for this txn in start phase. row and col will be
    // used by this txn in commit phase.
    // for haonan, I think it should be like this?
    std::set<std::pair<mdb::Row *, int>> row_col_map;
public:
    RO6DTxn(i64 tid, DTxnMgr *mgr, bool ro) : RCCDTxn(tid, mgr, ro) {
    }

    // Implementing create method
    mdb::Row *create(const mdb::Schema *schema, const std::vector<mdb::Value> &values) {
        return RO6Row::create(schema, values);
    }


    virtual void start(
            const RequestHeader &header,
            const std::vector<mdb::Value> &input,
            bool *deferred,
            ChopStartResponse *res
    );

    virtual void start_ro(
            const RequestHeader &header,
            const std::vector<mdb::Value> &input,
            std::vector<mdb::Value> &output,
            rrr::DeferredReply *defer
    );

    // the start function above and this commit function only for general(write) transactions
    virtual void commit(
            const ChopFinishRequest &req,
            ChopFinishResponse *res,
            rrr::DeferredReply *defer
    );

    // This is not called by a read-only-transaction's start phase,
    virtual void kiss(
            mdb::Row *r,
            int col, bool immediate
    );

    virtual bool read_column(
            mdb::Row* row,
            mdb::column_id_t col_id,
            Value* value
    );

    // Called by ro_start. It contains the main logics for ROT's start phase
    // For instance, check txnid table to see if this txn's id is in the table.
    // If it is, then return old version accordingly; if not, add its id into the table
    // *but, before doing those, wait for all conflicting write txns commit.
    // It also does the read, and returns the value with correct version.
    Value do_ro(i64 txn_id, MultiVersionedRow *row, int col_id);
};

//} // namespace rococo 