/* Copyright (C) 2003 MySQL AB

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

#include <signaldata/IndexStatSignal.hpp>

static void
get_req_rt_name(Uint32 rt, char* rt_name)
{
  strcpy(rt_name, "Unknown");
#define set_req_rt_name(x) if (rt == IndexStatReq::x) strcpy(rt_name, #x)
  set_req_rt_name(RT_UPDATE_STAT);
  set_req_rt_name(RT_CLEAN_NEW);
  set_req_rt_name(RT_SCAN_FRAG);
  set_req_rt_name(RT_CLEAN_OLD);
  set_req_rt_name(RT_START_MON);
  set_req_rt_name(RT_DELETE_STAT);
  set_req_rt_name(RT_STOP_MON);
  set_req_rt_name(RT_DROP_HEAD);
  set_req_rt_name(RT_CLEAN_ALL);
#undef set_req_rt_name
}

static void
get_rep_rt_name(Uint32 rt, char* rt_name)
{
  strcpy(rt_name, "Unknown");
#define set_rep_rt_name(x) if (rt == IndexStatRep::x) strcpy(rt_name, #x)
  set_rep_rt_name(RT_UPDATE_REQ);
  set_rep_rt_name(RT_UPDATE_CONF);
#undef set_rep_rt_name
}

bool
printINDEX_STAT_REQ(FILE* output, const Uint32* theData, Uint32 len, Uint16)
{
  const IndexStatReq* sig = (const IndexStatReq*)theData;
  fprintf(output, " clientRef: 0x%x", sig->clientRef);
  fprintf(output, " clientData: %u", sig->clientData);
  fprintf(output, "\n");
  Uint32 rt = sig->requestInfo & 0xFF;
  char rt_name[40];
  get_req_rt_name(rt, rt_name);
  fprintf(output, " requestType: %s[%u]", rt_name, rt);
  fprintf(output, " requestFlag: 0x%x", sig->requestFlag);
  fprintf(output, "\n");
  fprintf(output, " indexId: %u", sig->indexId);
  fprintf(output, " indexVersion: %u", sig->indexVersion);
  fprintf(output, " tableId: %u", sig->tableId);
  fprintf(output, "\n");
  return true;
}

bool
printINDEX_STAT_IMPL_REQ(FILE* output, const Uint32* theData, Uint32 len, Uint16)
{
  const IndexStatImplReq* sig = (const IndexStatImplReq*)theData;
  fprintf(output, " senderRef: 0x%x", sig->senderRef);
  fprintf(output, " senderData: %u", sig->senderData);
  fprintf(output, "\n");
  Uint32 rt = sig->requestType;
  char rt_name[40];
  get_req_rt_name(rt, rt_name);
  fprintf(output, " requestType: %s[%u]", rt_name, rt);
  fprintf(output, " requestFlag: 0x%x", sig->requestFlag);
  fprintf(output, "\n");
  fprintf(output, " indexId: %u", sig->indexId);
  fprintf(output, " indexVersion: %u", sig->indexVersion);
  fprintf(output, " tableId: %u", sig->tableId);
  fprintf(output, " fragId: %u", sig->fragId);
  fprintf(output, " fragCount: %u", sig->fragCount);
  fprintf(output, "\n");
  return true;
}

bool
printINDEX_STAT_CONF(FILE* output, const Uint32* theData, Uint32 len, Uint16)
{
  const IndexStatConf* sig = (const IndexStatConf*)theData;
  fprintf(output, " senderRef: 0x%x", sig->senderRef);
  fprintf(output, " senderData: %u", sig->senderData);
  fprintf(output, "\n");
  return true;
}

bool
printINDEX_STAT_IMPL_CONF(FILE* output, const Uint32* theData, Uint32 len, Uint16)
{
  const IndexStatImplConf* sig = (const IndexStatImplConf*)theData;
  fprintf(output, " senderRef: 0x%x", sig->senderRef);
  fprintf(output, " senderData: %u", sig->senderData);
  fprintf(output, "\n");
  return true;
}

bool
printINDEX_STAT_REF(FILE* output, const Uint32* theData, Uint32 len, Uint16)
{
  const IndexStatRef* sig = (const IndexStatRef*)theData;
  fprintf(output, " senderRef: 0x%x", sig->senderRef);
  fprintf(output, " senderData: %u", sig->senderData);
  fprintf(output, " errorCode: %u", sig->errorCode);
  fprintf(output, " errorLine: %u", sig->errorLine);
  fprintf(output, "\n");
  return true;
}

bool
printINDEX_STAT_IMPL_REF(FILE* output, const Uint32* theData, Uint32 len, Uint16)
{
  const IndexStatImplRef* sig = (const IndexStatImplRef*)theData;
  fprintf(output, " senderRef: 0x%x", sig->senderRef);
  fprintf(output, " senderData: %u", sig->senderData);
  fprintf(output, " errorCode: %u", sig->errorCode);
  fprintf(output, " errorLine: %u", sig->errorLine);
  fprintf(output, "\n");
  return true;
}

bool
printINDEX_STAT_REP(FILE* output, const Uint32* theData, Uint32 len, Uint16)
{
  const IndexStatRep* sig = (const IndexStatRep*)theData;
  fprintf(output, " senderRef: 0x%x", sig->senderRef);
  fprintf(output, " senderData: %u", sig->senderData);
  fprintf(output, "\n");
  Uint32 rt = sig->requestType;
  char rt_name[40];
  get_rep_rt_name(rt, rt_name);
  fprintf(output, " requestType: %s[%u]", rt_name, rt);
  fprintf(output, " requestFlag: 0x%x", sig->requestFlag);
  fprintf(output, "\n");
  fprintf(output, " indexId: %u", sig->indexId);
  fprintf(output, " indexVersion: %u", sig->indexVersion);
  fprintf(output, " tableId: %u", sig->tableId);
  fprintf(output, "\n");
  fprintf(output, " fragId: %u", sig->fragId);
  fprintf(output, " loadTime: %u", sig->loadTime);
  fprintf(output, "\n");
  return true;
}
 