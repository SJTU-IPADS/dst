#pragma once

#include "rrr.hpp"

#include <errno.h>


namespace rococo {

class BrqService: public rrr::Service {
public:
    enum {
        DISPATCH = 0x41bd14e7,
        COMMIT = 0x62baa6c9,
        INQUIRE = 0x1e142baa,
        PREACCEPT = 0x42396e6e,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(DISPATCH, this, &BrqService::__Dispatch__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(COMMIT, this, &BrqService::__Commit__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(INQUIRE, this, &BrqService::__Inquire__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(PREACCEPT, this, &BrqService::__PreAccept__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(DISPATCH);
        svr->unreg(COMMIT);
        svr->unreg(INQUIRE);
        svr->unreg(PREACCEPT);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void Dispatch(const SimpleCommand& cmd, rrr::i32* res, std::map<rrr::i32, Value>* output, BrqGraph* ret_graph, rrr::DeferredReply* defer) = 0;
    virtual void Commit(const cmdid_t& id, const RccGraph&, int32_t* res, TxnOutput* output, rrr::DeferredReply* defer) = 0;
    virtual void Inquire(const cmdid_t& txn_id, BrqGraph* ret_graph, rrr::DeferredReply* defer) = 0;
    virtual void PreAccept(const cmdid_t& txn_id, const RccGraph& graph, rrr::i32* res, RccGraph* ret_graph, rrr::DeferredReply* defer) = 0;
private:
    void __Dispatch__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        SimpleCommand* in_0 = new SimpleCommand;
        req->m >> *in_0;
        rrr::i32* out_0 = new rrr::i32;
        std::map<rrr::i32, Value>* out_1 = new std::map<rrr::i32, Value>;
        BrqGraph* out_2 = new BrqGraph;
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
    void __Commit__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        RccGraph* in_1 = new RccGraph;
        req->m >> *in_1;
        int32_t* out_0 = new int32_t;
        TxnOutput* out_1 = new TxnOutput;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Commit(*in_0, *in_1, out_0, out_1, __defer__);
    }
    void __Inquire__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        BrqGraph* out_0 = new BrqGraph;
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
    void __PreAccept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        RccGraph* in_1 = new RccGraph;
        req->m >> *in_1;
        rrr::i32* out_0 = new rrr::i32;
        RccGraph* out_1 = new RccGraph;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->PreAccept(*in_0, *in_1, out_0, out_1, __defer__);
    }
};

class BrqProxy {
protected:
    rrr::Client* __cl__;
public:
    BrqProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_Dispatch(const SimpleCommand& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(BrqService::DISPATCH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Dispatch(const SimpleCommand& cmd, rrr::i32* res, std::map<rrr::i32, Value>* output, BrqGraph* ret_graph) {
        rrr::Future* __fu__ = this->async_Dispatch(cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *output;
            __fu__->get_reply() >> *ret_graph;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Commit(const cmdid_t& id, const RccGraph& in_1, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(BrqService::COMMIT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << id;
            *__cl__ << in_1;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Commit(const cmdid_t& id, const RccGraph& in_1, int32_t* res, TxnOutput* output) {
        rrr::Future* __fu__ = this->async_Commit(id, in_1);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *output;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Inquire(const cmdid_t& txn_id, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(BrqService::INQUIRE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << txn_id;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Inquire(const cmdid_t& txn_id, BrqGraph* ret_graph) {
        rrr::Future* __fu__ = this->async_Inquire(txn_id);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *ret_graph;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_PreAccept(const cmdid_t& txn_id, const RccGraph& graph, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(BrqService::PREACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << txn_id;
            *__cl__ << graph;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 PreAccept(const cmdid_t& txn_id, const RccGraph& graph, rrr::i32* res, RccGraph* ret_graph) {
        rrr::Future* __fu__ = this->async_PreAccept(txn_id, graph);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *ret_graph;
        }
        __fu__->release();
        return __ret__;
    }
};

} // namespace rococo



 