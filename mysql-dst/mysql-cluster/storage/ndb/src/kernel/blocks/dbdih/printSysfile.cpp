/*
   Copyright (c) 2005, 2016, Oracle and/or its affiliates. All rights reserved.

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

#include <NdbOut.hpp>
#include "Sysfile.hpp"

#define JAM_FILE_ID 358


static int g_all = 0;

inline void ndb_end_and_exit(int exitcode)
{
  ndb_end(0);
  exit(exitcode);
}

void 
usage(const char * prg){
  ndbout << "Usage " << prg 
	 << " P[0-1].sysfile" << endl;  
}

struct NSString {
  Sysfile::ActiveStatus NodeStatus;
  const char * desc;
};

static const
NSString NodeStatusStrings[] = {
  { Sysfile::NS_Active,                 "Active         " },
  { Sysfile::NS_ActiveMissed_1,         "Active missed 1" },
  { Sysfile::NS_ActiveMissed_2,         "Active missed 2" },
  { Sysfile::NS_ActiveMissed_3,         "Active missed 3" },
  { Sysfile::NS_NotActive_NotTakenOver, "Not active     " },
  { Sysfile::NS_TakeOver,               "Take over      " },
  { Sysfile::NS_NotActive_TakenOver,    "Taken over     " },
  { Sysfile::NS_NotDefined,             "Not defined    " }
  ,{ Sysfile::NS_Configured,            "Configured     " }
};

const
char * getNSString(Uint32 ns){
  for(Uint32 i = 0; i<(sizeof(NodeStatusStrings)/sizeof(NSString)); i++)
    if((Uint32)NodeStatusStrings[i].NodeStatus == ns)
      return NodeStatusStrings[i].desc;
  return "<Unknown state>";
}

void
fill(const char * buf, int mod){
  int len = (int)(strlen(buf)+1);
  ndbout << buf << " ";
  while((len % mod) != 0){
    ndbout << " ";
    len++;
  }
}

void 
print(const char * filename, const Sysfile * sysfile){
  char buf[255];
  ndbout << "----- Sysfile: " << filename 
	 << " seq: " << hex << sysfile->m_restart_seq
	 << " -----" << endl;
  ndbout << "Initial start ongoing: " 
	 << Sysfile::getInitialStartOngoing(sysfile->systemRestartBits) 
	 << ", ";

  ndbout << "Restart Ongoing: "
	 << Sysfile::getRestartOngoing(sysfile->systemRestartBits) 
	 << ", ";

  ndbout << "LCP Ongoing: "
	 << Sysfile::getLCPOngoing(sysfile->systemRestartBits) 
	 << endl;


  ndbout << "-- Global Checkpoint Identities: --" << endl;
  sprintf(buf, "keepGCI = %u", sysfile->keepGCI);
  fill(buf, 40); 
  ndbout << " -- Tail of REDO log" << endl;
  
  sprintf(buf, "oldestRestorableGCI = %u", sysfile->oldestRestorableGCI);
  fill(buf, 40);
  ndbout << " -- " << endl;

  sprintf(buf, "newestRestorableGCI = %u", sysfile->newestRestorableGCI);
  fill(buf, 40);
  ndbout << " -- " << endl;

  sprintf(buf, "latestLCP = %u", sysfile->latestLCP_ID);
  fill(buf, 40);
  ndbout << " -- " << endl;

  ndbout << "-- Node status: --" << endl;
  for(int i = 1; i < MAX_NDB_NODES; i++){
    if(g_all || Sysfile::getNodeStatus(i, sysfile->nodeStatus) !=Sysfile::NS_NotDefined){
      sprintf(buf, 
	      "Node %.2d -- %s GCP: %d, NodeGroup: %d, TakeOverNode: %d, "
	      "LCP Ongoing: %s",
	      i, 
	      getNSString(Sysfile::getNodeStatus(i,sysfile->nodeStatus)),
	      sysfile->lastCompletedGCI[i],
	      Sysfile::getNodeGroup(i, sysfile->nodeGroups),
	      Sysfile::getTakeOverNode(i, sysfile->takeOver),
	      BitmaskImpl::get(NdbNodeBitmask::Size, 
			       sysfile->lcpActive, i) != 0 ? "yes" : "no");
      ndbout << buf << endl;
    }
  }
}


int main(int argc, char** argv)
{
  ndb_init();
  if(argc < 2){
    usage(argv[0]);
    ndb_end_and_exit(0);
  }

  for(int i = 1; i<argc; i++){
    const char * filename = argv[i];

    if (strcmp(filename, "--all") == 0)
    {
      g_all = 1;
      continue;
    }
    
    struct stat sbuf;

    if(stat(filename, &sbuf) != 0)
    {
      ndbout << "Could not find file: \"" << filename << "\"" << endl;
      continue;
    }
    const Uint32 bytes = sbuf.st_size;
    
    Uint32 * buf = new Uint32[bytes/4+1];
    
    FILE * f = fopen(filename, "rb");
    if(f == 0){
      ndbout << "Failed to open file" << endl;
      delete [] buf;
      continue;
    }
    Uint32 sz = (Uint32)fread(buf, 1, bytes, f);
    fclose(f);
    if(sz != bytes){
      ndbout << "Failure while reading file" << endl;
      delete [] buf;
      continue;
    }
    
    print(filename, (Sysfile *)&buf[0]);
    delete [] buf;
    continue;
  }
  ndb_end_and_exit(0);
}
 