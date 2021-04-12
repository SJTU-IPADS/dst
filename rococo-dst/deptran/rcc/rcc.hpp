#pragma once

//namespace rococo {
class RCCDTxn : public DTxn {
public:

    typedef struct {
        RequestHeader header;
        std::vector<mdb::Value> inputs;
        row_map_t row_map;
    } DeferredRequest;

    static DepGraph *dep_s;

    std::vector<DeferredRequest> dreqs_;
    Vertex <TxnInfo> *tv_;

    std::vector<TxnInfo *> conflict_txns_; // This is read-only transaction

    bool read_only_;

    RCCDTxn(i64 tid, DTxnMgr *mgr, bool ro);

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

    virtual void commit(
            const ChopFinishRequest &req,
            ChopFinishResponse *res,
            rrr::DeferredReply *defer
    );

    void to_decide(
            Vertex <TxnInfo> *v,
            rrr::DeferredReply *defer
    );

    void exe_deferred(
            std::vector<
                std::pair<RequestHeader, 
                          std::vector<mdb::Value> 
                         > 
            > &outputs
    );

    void send_ask_req(
            Vertex <TxnInfo> *av
    );

    virtual mdb::Row *create(
            const mdb::Schema *schema,
            const std::vector<mdb::Value> &values) {
        return RCCRow::create(schema, values);
    }

    virtual void kiss(
            mdb::Row *r,
            int col,
            bool immediate
    );

//    virtual bool read_column(
//            mdb::Row *row,
//            mdb::column_id_t col_id,
//            Value *value
//    );
//
//    virtual bool write_column(
//            mdb::Row* row,
//            mdb::column_id_t col_id,
//            const Value& value
//    );
//
//    virtual bool insert_row(
//            mdb::Table* tbl,
//            mdb::Row* row
//    );
//
//    virtual bool remove_row(
//            mdb::Table* tbl,
//            mdb::Row* row
//    );
};
//} // namespace rococo
 