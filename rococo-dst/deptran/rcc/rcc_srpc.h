#pragma once

#include "rrr.hpp"

#include <errno.h>


namespace rococo {

struct RequestHeader {
    rrr::i32 t_type;
    rrr::i32 p_type;
    rrr::i32 cid;
    rrr::i64 tid;
    rrr::i64 pid;
    rrr::i64 timestamp;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const RequestHeader& o) {
    m << o.t_type;
    m << o.p_type;
    m << o.cid;
    m << o.tid;
    m << o.pid;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, RequestHeader& o) {
    m >> o.t_type;
    m >> o.p_type;
    m >> o.cid;
    m >> o.tid;
    m >> o.pid;
    return m;
}

struct BatchRequestHeader {
    rrr::i32 t_type;
    rrr::i32 cid;
    rrr::i64 tid;
    rrr::i32 num_piece;
    rrr::i32 expected_output_size;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const BatchRequestHeader& o) {
    m << o.t_type;
    m << o.cid;
    m << o.tid;
    m << o.num_piece;
    m << o.expected_output_size;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, BatchRequestHeader& o) {
    m >> o.t_type;
    m >> o.cid;
    m >> o.tid;
    m >> o.num_piece;
    m >> o.expected_output_size;
    return m;
}

class RococoService: public rrr::Service {
public:
    enum {
        DISPATCH = 0x4237627d,
        FINISH = 0x3f6d0d47,
        INQUIRE = 0x2773788e,
        RCC_RO_START_PIE = 0x4d96b04a,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(DISPATCH, this, &RococoService::__Dispatch__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(FINISH, this, &RococoService::__Finish__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(INQUIRE, this, &RococoService::__Inquire__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCC_RO_START_PIE, this, &RococoService::__rcc_ro_start_pie__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(DISPATCH);
        svr->unreg(FINISH);
        svr->unreg(INQUIRE);
        svr->unreg(RCC_RO_START_PIE);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void Dispatch(const SimpleCommand& cmd, rrr::i32* res, std::map<rrr::i32, Value>* output, RccGraph* graph, rrr::DeferredReply* defer) = 0;
    virtual void Finish(const cmdid_t& id, const RccGraph&, std::map<uint32_t, std::map<int32_t, Value>>* outputs, rrr::DeferredReply* defer) = 0;
    virtual void Inquire(const cmdid_t& txn_id, RccGraph* res, rrr::DeferredReply* defer) = 0;
    virtual void rcc_ro_start_pie(const SimpleCommand& cmd, std::map<rrr::i32, Value>* output, rrr::DeferredReply* defer) = 0;
private:
    void __Dispatch__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        SimpleCommand* in_0 = new SimpleCommand;
        req->m >> *in_0;
        rrr::i32* out_0 = new rrr::i32;
        std::map<rrr::i32, Value>* out_1 = new std::map<rrr::i32, Value>;
        RccGraph* out_2 = new RccGraph;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
            *sconn << *out_2;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
            delete out_1;
            delete out_2;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Dispatch(*in_0, out_0, out_1, out_2, __defer__);
    }
    void __Finish__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        RccGraph* in_1 = new RccGraph;
        req->m >> *in_1;
        std::map<uint32_t, std::map<int32_t, Value>>* out_0 = new std::map<uint32_t, std::map<int32_t, Value>>;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Finish(*in_0, *in_1, out_0, __defer__);
    }
    void __Inquire__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        RccGraph* out_0 = new RccGraph;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Inquire(*in_0, out_0, __defer__);
    }
    void __rcc_ro_start_pie__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        SimpleCommand* in_0 = new SimpleCommand;
        req->m >> *in_0;
        std::map<rrr::i32, Value>* out_0 = new std::map<rrr::i32, Value>;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->rcc_ro_start_pie(*in_0, out_0, __defer__);
    }
};

class RococoProxy {
protected:
    rrr::Client* __cl__;
public:
    RococoProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_Dispatch(const SimpleCommand& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(RococoService::DISPATCH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Dispatch(const SimpleCommand& cmd, rrr::i32* res, std::map<rrr::i32, Value>* output, RccGraph* graph) {
        rrr::Future* __fu__ = this->async_Dispatch(cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *output;
            __fu__->get_reply() >> *graph;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Finish(const cmdid_t& id, const RccGraph& in_1, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(RococoService::FINISH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << id;
            *__cl__ << in_1;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Finish(const cmdid_t& id, const RccGraph& in_1, std::map<uint32_t, std::map<int32_t, Value>>* outputs) {
        rrr::Future* __fu__ = this->async_Finish(id, in_1);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *outputs;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Inquire(const cmdid_t& txn_id, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(RococoService::INQUIRE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << txn_id;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Inquire(const cmdid_t& txn_id, RccGraph* res) {
        rrr::Future* __fu__ = this->async_Inquire(txn_id);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_rcc_ro_start_pie(const SimpleCommand& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(RococoService::RCC_RO_START_PIE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 rcc_ro_start_pie(const SimpleCommand& cmd, std::map<rrr::i32, Value>* output) {
        rrr::Future* __fu__ = this->async_rcc_ro_start_pie(cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *output;
        }
        __fu__->release();
        return __ret__;
    }
};

} // namespace rococo



 