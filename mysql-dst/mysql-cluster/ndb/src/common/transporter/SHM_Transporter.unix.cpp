/*
   Copyright (c) 2003, 2018, Oracle and/or its affiliates. All rights reserved.

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


#include <ndb_global.h>

#include "SHM_Transporter.hpp"
#include "TransporterInternalDefinitions.hpp"
#include <TransporterCallback.hpp>
#include <NdbSleep.h>
#include <NdbOut.hpp>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <EventLogger.hpp>
extern EventLogger * g_eventLogger;

#if 0
#define DEBUG_FPRINTF(arglist) do { fprintf arglist ; } while (0)
#else
#define DEBUG_FPRINTF(a)
#endif

void SHM_Transporter::make_error_info(char info[], int sz)
{
  snprintf(info,sz,"Shm key=%d sz=%d id=%d",
	   shmKey, shmSize, shmId);
}

bool
SHM_Transporter::ndb_shm_create()
{
  if (!isServer)
  {
    ndbout_c("Trying to create shared memory segment on the client side");
    return false;
  }
  shmId = shmget(shmKey, shmSize, IPC_CREAT | 960);
  if (shmId == -1)
  {
    DEBUG_FPRINTF((stderr, "(%u)shmget(IPC_CREAT)(%u) failed LINE:%d,shmId:%d,"
                           " errno %d(%s)\n",
                   localNodeId,
                   remoteNodeId,
                   __LINE__,
                   shmId,
                   errno,
                   strerror(errno)));
    return false;
  }
  return true;
}

bool
SHM_Transporter::ndb_shm_get()
{
  shmId = shmget(shmKey, shmSize, 0);
  if (shmId == -1)
  {
    DEBUG_FPRINTF((stderr, "(%u)shmget(0)(%u) failed LINE:%d, shmId:%d,"
                           " errno %d(%s)\n",
                   localNodeId,
                   remoteNodeId,
                   __LINE__,
                   shmId,
                   errno,
                   strerror(errno)));
    return false;
  }
  return true;
}

bool
SHM_Transporter::ndb_shm_attach()
{
  assert(shmBuf == 0);
  shmBuf = (char *)shmat(shmId, 0, 0);
  if (shmBuf == 0)
  {
    DEBUG_FPRINTF((stderr, "(%u)shmat(%u) failed LINE:%d, shmId:%d,"
                           " errno %d(%s)\n",
                   localNodeId,
                   remoteNodeId,
                   __LINE__,
                   shmId,
                   errno,
                   strerror(errno)));
    if (isServer)
      shmctl(shmId, IPC_RMID, 0);
    _shmSegCreated = false;
    return false;
  }
  return true;
}

void
SHM_Transporter::ndb_shm_destroy()
{
  /**
   * We have attached to the shared memory segment.
   * The shared memory won't be removed until all
   * attached processes have detached. To ensure
   * that we remove the shared memory segment even
   * after a crash we now remove it immediately.
   * Otherwise the shared memory segment will be
   * left after a crash.
   */
  const int res = shmctl(shmId, IPC_RMID, 0);
  if(res == -1)
  {
    DEBUG_FPRINTF((stderr, "(%u)shmctl(IPC_RMID)(%u) failed LINE:%d, shmId:%d,"
                           " errno %d(%s)\n",
                   localNodeId,
                   remoteNodeId,
                   __LINE__,
                   shmId,
                   errno,
                   strerror(errno)));
    return;
  }
  _shmSegCreated = false;
}

bool
SHM_Transporter::checkConnected()
{
  struct shmid_ds info;
  const int res = shmctl(shmId, IPC_STAT, &info);
  if (res == -1)
  {
    DEBUG_FPRINTF((stderr, "(%u)checkConnected(%u) failed LINE:%d, shmId:%d,"
                           " errno %d(%s)\n",
                   localNodeId,
                   remoteNodeId,
                   __LINE__,
                   shmId,
                   errno,
                   strerror(errno)));
    return false;
  }
 
  if (info.shm_nattch != 2)
  {
    DEBUG_FPRINTF((stderr, "(%u)checkConnected(%u) failed LINE:%d, nattch != 2",
                   localNodeId,
                   remoteNodeId,
                   __LINE__));
    DBUG_PRINT("error", ("Already connected to node %d",
                remoteNodeId));
    return false;
  }
  return true;
}

void
SHM_Transporter::detach_shm(bool rep_error)
{
  if (_attached)
  {
    struct shmid_ds info;
    const int ret_val = shmctl(shmId, IPC_STAT, &info);
    if (ret_val != -1)
    {
      if (info.shm_nattch > 0)
      {
        /**
         * Ensure that the last process to detach from the
         * shared memory is the one that removes the mutexes.
         * This ensures that we synchronize the removal of the
         * mutexes and ensures also that we will destroy the
         * mutexes before we detach the last shared memory user.
         */
        NdbMutex_Lock(serverMutex);
        if (isServer)
        {
          * serverUpFlag = 0;
        }
        else
        {
          * clientUpFlag = 0;
        }
        bool last = (*serverUpFlag == 0 && clientUpFlag == 0);
        NdbMutex_Unlock(serverMutex);
        if (last)
        {
          remove_mutexes();
        }
      }
    }
    const int res = shmdt(shmBuf);
    if(res == -1)
    {
      DEBUG_FPRINTF((stderr, "(%u)shmdt(%u) failed LINE:%d, shmId:%d,"
                             " errno %d(%s)\n",
                     localNodeId,
                     remoteNodeId,
                     __LINE__,
                     shmId,
                     errno,
                     strerror(errno)));
      if (rep_error)
        report_error(TE_SHM_UNABLE_TO_REMOVE_SEGMENT);
    }
    _attached = false;
    DEBUG_FPRINTF((stderr, "(%u)shmdt(%d)\n",
                   localNodeId, remoteNodeId));
  }
  
  if (isServer && _shmSegCreated)
  {
    /**
     * Normally should not happen.
     */
    assert(!rep_error);
    const int res = shmctl(shmId, IPC_RMID, 0);
    if(res == -1)
    {
      DEBUG_FPRINTF((stderr, "(%u)shmctl(IPC_RMID)(%u) failed LINE:%d,"
                             " shmId:%d, errno %d(%s)\n",
                     localNodeId,
                     remoteNodeId,
                     __LINE__,
                     shmId,
                     errno,
                     strerror(errno)));
      if (rep_error)
        report_error(TE_SHM_UNABLE_TO_REMOVE_SEGMENT);
    }
    DEBUG_FPRINTF((stderr, "(%u)shmctl(IPC_RMID)(%d)\n",
                   localNodeId, remoteNodeId));
  }
  _shmSegCreated = false;
  if (reader != 0)
  {
    DEBUG_FPRINTF((stderr, "(%u)detach_shm(%u) LINE:%d",
                   localNodeId, __LINE__, remoteNodeId));
    reader->~SHM_Reader();
    writer->~SHM_Writer();
    shmBuf = 0;
    reader = 0;
    writer = 0;
  }
  else
  {
    DEBUG_FPRINTF((stderr, "(%u)reader==0(%u) LINE:%d",
                   localNodeId, __LINE__, remoteNodeId));
  }
}

void
SHM_Transporter::disconnectImpl()
{
  DEBUG_FPRINTF((stderr, "(%u)disconnectImpl(%d)\n",
                 localNodeId, remoteNodeId));
  disconnect_socket();
  setupBuffersUndone();
}
 