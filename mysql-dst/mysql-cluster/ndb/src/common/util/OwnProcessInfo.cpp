/*
   Copyright (c) 2016, 2017 Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "ndb_global.h"
#include "ProcessInfo.hpp"
#include "OwnProcessInfo.hpp"
#include <NdbMutex.h>

const char * ndb_basename(const char *path);

extern "C" {
  extern const char * my_progname;
}

/* Static storage; constructor called at process startup by C++ runtime. */
ProcessInfo singletonInfo;
NdbLockable theApiMutex;

/* Public API
 *
 */
void setOwnProcessInfoAngelPid(Uint32 pid)
{
  theApiMutex.lock();
  singletonInfo.setAngelPid(pid);
  theApiMutex.unlock();
}

void setOwnProcessInfoServerAddress(struct in_addr * addr)
{
  theApiMutex.lock();
  singletonInfo.setHostAddress(addr);
  theApiMutex.unlock();
}

void setOwnProcessInfoPort(Uint16 port)
{
  theApiMutex.lock();
  singletonInfo.setPort(port);
  theApiMutex.unlock();
}


/* Fill in missing parts of ProcessInfo before providing it to QMgr 
   or ClusterMgr
*/

#ifdef WIN32
#include "psapi.h"

void getNameFromEnvironment()
{
  HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                              FALSE, singletonInfo.getPid());
  GetModuleFileNameEx(handle, 0, singletonInfo.process_name,
                      singletonInfo.ProcessNameLength);
}
#else
void getNameFromEnvironment()
{
  const char * path = getenv("_");
  if(path)
  {
    singletonInfo.setProcessName(ndb_basename(path));
  }
}
#endif


/* On unix only, if we are not a daemon, and also not a process group leader,
   set parent pid as angel pid.
*/
static Uint32 getParentPidAsAngel()
{
#ifndef WIN32
  pid_t parent_process_id = getppid();
  if((parent_process_id != 1)  && (getpgrp() != singletonInfo.getPid()))
  {
    return parent_process_id;
  }
#endif
  return 0;
}


/* Public API for QMgr and ClusterMgr.
*/
ProcessInfo * getOwnProcessInfo(Uint16 nodeId) {
  Guard locked(theApiMutex);
  if(singletonInfo.process_id == 0)
  {
    /* Finalize */
    singletonInfo.setPid();
    singletonInfo.node_id = nodeId;
    if(singletonInfo.angel_process_id == 0)
      singletonInfo.angel_process_id = getParentPidAsAngel();
    if(singletonInfo.process_name[0] == 0)
    {
      if(my_progname)
        singletonInfo.setProcessName(ndb_basename(my_progname));
      else
        getNameFromEnvironment();
    }
  }

  return & singletonInfo;
}
 