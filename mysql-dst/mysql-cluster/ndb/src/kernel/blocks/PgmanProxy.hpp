/* Copyright (c) 2008, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA */

#ifndef NDB_PGMAN_PROXY_HPP
#define NDB_PGMAN_PROXY_HPP

#include <LocalProxy.hpp>
#include <signaldata/LCP.hpp>
#include <signaldata/ReleasePages.hpp>
#include "pgman.hpp"

#define JAM_FILE_ID 434


class PgmanProxy : public LocalProxy {
public:
  PgmanProxy(Block_context& ctx);
  virtual ~PgmanProxy();
  BLOCK_DEFINES(PgmanProxy);

protected:
  virtual SimulatedBlock* newWorker(Uint32 instanceNo);

  // GSN_END_LCPREQ
  struct Ss_END_LCPREQ : SsParallel {
    /*
     * Sent once from LQH proxy (at LCP) and LGMAN (at SR).
     * Each pgman instance runs LCP before we send a CONF.
     */
    static const char* name() { return "END_LCPREQ"; }
    EndLcpReq m_req;
    bool m_extraLast;
    Ss_END_LCPREQ() {
      m_sendREQ = (SsFUNCREQ)&PgmanProxy::sendEND_LCPREQ;
      m_sendCONF = (SsFUNCREP)&PgmanProxy::sendEND_LCPCONF;
      // extra worker (for extent pages) must run after others
      m_extraLast = false;
    }
    enum { poolSize = 1 };
    static SsPool<Ss_END_LCPREQ>& pool(LocalProxy* proxy) {
      return ((PgmanProxy*)proxy)->c_ss_END_LCPREQ;
    }
  };
  SsPool<Ss_END_LCPREQ> c_ss_END_LCPREQ;
  static Uint32 getSsId(const EndLcpReq* req) {
    return SsIdBase | (req->backupId & 0xFFFF);
  }
  static Uint32 getSsId(const EndLcpConf* conf) {
    return conf->senderData;
  }
  static Uint32 getSsId(const ReleasePagesConf* conf) {
    return conf->senderData;
  }
  void execSYNC_EXTENT_PAGES_REQ(Signal*);
  void execEND_LCPREQ(Signal*);
  void sendEND_LCPREQ(Signal*, Uint32 ssId, SectionHandle*);
  void execEND_LCPCONF(Signal*);
  void sendEND_LCPCONF(Signal*, Uint32 ssId);
  void execRELEASE_PAGES_CONF(Signal*);

  // client methods
  friend class Page_cache_client;

  int get_page(Page_cache_client& caller,
               Signal*, Page_cache_client::Request& req, Uint32 flags);

  void update_lsn(Signal *signal,
                  Page_cache_client& caller,
                  Local_key key, Uint64 lsn);

  int drop_page(Page_cache_client& caller,
                Local_key key, Uint32 page_id);

  Uint32 create_data_file(Signal*, Uint32 version);

  Uint32 alloc_data_file(Signal*, Uint32 file_no, Uint32 version);

  void map_file_no(Signal*, Uint32 file_no, Uint32 fd);

  void free_data_file(Signal*, Uint32 file_no, Uint32 fd);

  void send_data_file_ord(Signal*, Uint32 i, Uint32 ret, Uint32 version,
                          Uint32 cmd, Uint32 file_no = RNIL, Uint32 fd = RNIL);
};


#undef JAM_FILE_ID

#endif
 