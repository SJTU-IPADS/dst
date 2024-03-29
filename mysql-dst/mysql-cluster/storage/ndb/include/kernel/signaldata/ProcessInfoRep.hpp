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

#ifndef NDB_PROCESSINFO_REP_H
#define NDB_PROCESSINFO_REP_H

#include "SignalData.hpp"

class ProcessInfoRep {
  friend class ClusterMgr;     // Sender
  friend class Qmgr;           // Receiver
  friend class ProcessInfo;    // Stored format
  friend bool printPROCESSINFO_REP(FILE *, const Uint32 *, Uint32, Uint16);

public:
  STATIC_CONST( SignalLength = 20);
  STATIC_CONST( PathSectionNum = 0);
  STATIC_CONST( HostSectionNum = 1);

private:
  Uint8 process_name[48];
  Uint8 uri_scheme[16];
  Uint32 node_id;
  Uint32 process_id;
  Uint32 angel_process_id;
  Uint32 application_port;
};

// path and host sections of service URI are sent as separate sections

#endif
 