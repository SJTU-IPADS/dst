#pragma once

#include "rrr.hpp"

#include <errno.h>


namespace mdcc {

struct StartRequest {
    rrr::i64 txn_id;
    rrr::i32 txn_type;
    std::map<rrr::i32, Value> inputs;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const StartRequest& o) {
    m << o.txn_id;
    m << o.txn_type;
    m << o.inputs;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, StartRequest& o) {
    m >> o.txn_id;
    m >> o.txn_type;
    m >> o.inputs;
    return m;
}

struct StartPieceRequest {
    rrr::i64 txn_id;
    rrr::i32 txn_type;
    rrr::i32 piece_id;
    rococo::SimpleCommand command;
    std::map<rrr::i32, Value> inputs;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const StartPieceRequest& o) {
    m << o.txn_id;
    m << o.txn_type;
    m << o.piece_id;
    m << o.command;
    m << o.inputs;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, StartPieceRequest& o) {
    m >> o.txn_id;
    m >> o.txn_type;
    m >> o.piece_id;
    m >> o.command;
    m >> o.inputs;
    return m;
}

struct StartResponse {
    rrr::i8 result;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const StartResponse& o) {
    m << o.result;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, StartResponse& o) {
    m >> o.result;
    return m;
}

struct StartPieceResponse {
    rrr::i32 result;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const StartPieceResponse& o) {
    m << o.result;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, StartPieceResponse& o) {
    m >> o.result;
    return m;
}

struct ProposeRequest {
    Ballot ballot;
    OptionSet updates;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const ProposeRequest& o) {
    m << o.ballot;
    m << o.updates;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, ProposeRequest& o) {
    m >> o.ballot;
    m >> o.updates;
    return m;
}

struct ProposeResponse {
    std::string table;
    rrr::i64 key;
    rrr::i8 accepted;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const ProposeResponse& o) {
    m << o.table;
    m << o.key;
    m << o.accepted;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, ProposeResponse& o) {
    m >> o.table;
    m >> o.key;
    m >> o.accepted;
    return m;
}

struct Phase2aRequest {
    rrr::i32 site_id;
    Ballot ballot;
    std::vector<OptionSet> values;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const Phase2aRequest& o) {
    m << o.site_id;
    m << o.ballot;
    m << o.values;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, Phase2aRequest& o) {
    m >> o.site_id;
    m >> o.ballot;
    m >> o.values;
    return m;
}

struct Phase2bRequest {
    rrr::i32 site_id;
    Ballot ballot;
    std::vector<OptionSet> values;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const Phase2bRequest& o) {
    m << o.site_id;
    m << o.ballot;
    m << o.values;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, Phase2bRequest& o) {
    m >> o.site_id;
    m >> o.ballot;
    m >> o.values;
    return m;
}

struct Phase2aResponse {
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const Phase2aResponse& o) {
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, Phase2aResponse& o) {
    return m;
}

class MdccLearnerService: public rrr::Service {
public:
    enum {
        PHASE2B = 0x44fd2b06,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(PHASE2B, this, &MdccLearnerService::__Phase2b__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(PHASE2B);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void Phase2b(const Phase2bRequest& req, rrr::DeferredReply* defer) = 0;
private:
    void __Phase2b__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        Phase2bRequest* in_0 = new Phase2bRequest;
        req->m >> *in_0;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Phase2b(*in_0, __defer__);
    }
};

class MdccLearnerProxy {
protected:
    rrr::Client* __cl__;
public:
    MdccLearnerProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_Phase2b(const Phase2bRequest& req, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MdccLearnerService::PHASE2B, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << req;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Phase2b(const Phase2bRequest& req) {
        rrr::Future* __fu__ = this->async_Phase2b(req);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
};

class MdccClientService: public rrr::Service {
public:
    enum {
        START = 0x13312b4d,
        STARTPIECE = 0x1276dadf,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(START, this, &MdccClientService::__Start__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(STARTPIECE, this, &MdccClientService::__StartPiece__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(START);
        svr->unreg(STARTPIECE);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void Start(const StartRequest& req, StartResponse* res, rrr::DeferredReply* defer) = 0;
    virtual void StartPiece(const rococo::SimpleCommand& cmd, StartPieceResponse* res, rrr::DeferredReply* defer) = 0;
private:
    void __Start__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        StartRequest* in_0 = new StartRequest;
        req->m >> *in_0;
        StartResponse* out_0 = new StartResponse;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Start(*in_0, out_0, __defer__);
    }
    void __StartPiece__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rococo::SimpleCommand* in_0 = new rococo::SimpleCommand;
        req->m >> *in_0;
        StartPieceResponse* out_0 = new StartPieceResponse;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->StartPiece(*in_0, out_0, __defer__);
    }
};

class MdccClientProxy {
protected:
    rrr::Client* __cl__;
public:
    MdccClientProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_Start(const StartRequest& req, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MdccClientService::START, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << req;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Start(const StartRequest& req, StartResponse* res) {
        rrr::Future* __fu__ = this->async_Start(req);
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
    rrr::Future* async_StartPiece(const rococo::SimpleCommand& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MdccClientService::STARTPIECE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 StartPiece(const rococo::SimpleCommand& cmd, StartPieceResponse* res) {
        rrr::Future* __fu__ = this->async_StartPiece(cmd);
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
};

class MdccLeaderService: public rrr::Service {
public:
    enum {
        PROPOSE = 0x6d5224df,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(PROPOSE, this, &MdccLeaderService::__Propose__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(PROPOSE);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void Propose(const ProposeRequest& req, rrr::DeferredReply* defer) = 0;
private:
    void __Propose__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        ProposeRequest* in_0 = new ProposeRequest;
        req->m >> *in_0;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Propose(*in_0, __defer__);
    }
};

class MdccLeaderProxy {
protected:
    rrr::Client* __cl__;
public:
    MdccLeaderProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_Propose(const ProposeRequest& req, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MdccLeaderService::PROPOSE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << req;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Propose(const ProposeRequest& req) {
        rrr::Future* __fu__ = this->async_Propose(req);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
};

class MdccAcceptorService: public rrr::Service {
public:
    enum {
        PROPOSE = 0x5e71ab96,
        PHASE2A = 0x684da841,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(PROPOSE, this, &MdccAcceptorService::__Propose__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(PHASE2A, this, &MdccAcceptorService::__Phase2a__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(PROPOSE);
        svr->unreg(PHASE2A);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void Propose(const ProposeRequest& req, rrr::DeferredReply* defer) = 0;
    virtual void Phase2a(const Phase2aRequest& req, rrr::DeferredReply* defer) = 0;
private:
    void __Propose__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        ProposeRequest* in_0 = new ProposeRequest;
        req->m >> *in_0;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Propose(*in_0, __defer__);
    }
    void __Phase2a__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        Phase2aRequest* in_0 = new Phase2aRequest;
        req->m >> *in_0;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Phase2a(*in_0, __defer__);
    }
};

class MdccAcceptorProxy {
protected:
    rrr::Client* __cl__;
public:
    MdccAcceptorProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_Propose(const ProposeRequest& req, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MdccAcceptorService::PROPOSE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << req;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Propose(const ProposeRequest& req) {
        rrr::Future* __fu__ = this->async_Propose(req);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Phase2a(const Phase2aRequest& req, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MdccAcceptorService::PHASE2A, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << req;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Phase2a(const Phase2aRequest& req) {
        rrr::Future* __fu__ = this->async_Phase2a(req);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
};

} // namespace mdcc



 