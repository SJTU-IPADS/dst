#pragma once

#include "rrr.hpp"

#include <errno.h>


namespace rococo {

class TapirService: public rrr::Service {
public:
    enum {
        HANDOUT = 0x58e56a05,
        PREPARE = 0x58d4b659,
        ACCEPT = 0x6a5837db,
        FASTACCEPT = 0x42fc530f,
        DECIDE = 0x6598058f,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(HANDOUT, this, &TapirService::__Handout__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(PREPARE, this, &TapirService::__Prepare__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCEPT, this, &TapirService::__Accept__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(FASTACCEPT, this, &TapirService::__FastAccept__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(DECIDE, this, &TapirService::__Decide__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(HANDOUT);
        svr->unreg(PREPARE);
        svr->unreg(ACCEPT);
        svr->unreg(FASTACCEPT);
        svr->unreg(DECIDE);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void Handout(const SimpleCommand& cmd, rrr::i32* res, std::map<rrr::i32, Value>* output, rrr::DeferredReply* defer) = 0;
    virtual void Prepare(rrr::DeferredReply* defer) = 0;
    virtual void Accept(const uint64_t& cmd_id, const uint64_t& ballot, const int32_t& decision, rrr::DeferredReply* defer) = 0;
    virtual void FastAccept(const uint64_t& cmd_id, const std::vector<SimpleCommand>& txn_cmds, rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void Decide(const uint64_t& cmd_id, const rrr::i32& commit, rrr::DeferredReply* defer) = 0;
private:
    void __Handout__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        SimpleCommand* in_0 = new SimpleCommand;
        req->m >> *in_0;
        rrr::i32* out_0 = new rrr::i32;
        std::map<rrr::i32, Value>* out_1 = new std::map<rrr::i32, Value>;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Handout(*in_0, out_0, out_1, __defer__);
    }
    void __Prepare__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Prepare(__defer__);
    }
    void __Accept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint64_t* in_0 = new uint64_t;
        req->m >> *in_0;
        uint64_t* in_1 = new uint64_t;
        req->m >> *in_1;
        int32_t* in_2 = new int32_t;
        req->m >> *in_2;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Accept(*in_0, *in_1, *in_2, __defer__);
    }
    void __FastAccept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint64_t* in_0 = new uint64_t;
        req->m >> *in_0;
        std::vector<SimpleCommand>* in_1 = new std::vector<SimpleCommand>;
        req->m >> *in_1;
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->FastAccept(*in_0, *in_1, out_0, __defer__);
    }
    void __Decide__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint64_t* in_0 = new uint64_t;
        req->m >> *in_0;
        rrr::i32* in_1 = new rrr::i32;
        req->m >> *in_1;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Decide(*in_0, *in_1, __defer__);
    }
};

class TapirProxy {
protected:
    rrr::Client* __cl__;
public:
    TapirProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_Handout(const SimpleCommand& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(TapirService::HANDOUT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Handout(const SimpleCommand& cmd, rrr::i32* res, std::map<rrr::i32, Value>* output) {
        rrr::Future* __fu__ = this->async_Handout(cmd);
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
    rrr::Future* async_Prepare(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(TapirService::PREPARE, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Prepare() {
        rrr::Future* __fu__ = this->async_Prepare();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Accept(const uint64_t& cmd_id, const uint64_t& ballot, const int32_t& decision, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(TapirService::ACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd_id;
            *__cl__ << ballot;
            *__cl__ << decision;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Accept(const uint64_t& cmd_id, const uint64_t& ballot, const int32_t& decision) {
        rrr::Future* __fu__ = this->async_Accept(cmd_id, ballot, decision);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_FastAccept(const uint64_t& cmd_id, const std::vector<SimpleCommand>& txn_cmds, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(TapirService::FASTACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd_id;
            *__cl__ << txn_cmds;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 FastAccept(const uint64_t& cmd_id, const std::vector<SimpleCommand>& txn_cmds, rrr::i32* res) {
        rrr::Future* __fu__ = this->async_FastAccept(cmd_id, txn_cmds);
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
    rrr::Future* async_Decide(const uint64_t& cmd_id, const rrr::i32& commit, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(TapirService::DECIDE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd_id;
            *__cl__ << commit;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Decide(const uint64_t& cmd_id, const rrr::i32& commit) {
        rrr::Future* __fu__ = this->async_Decide(cmd_id, commit);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
};

} // namespace rococo



 