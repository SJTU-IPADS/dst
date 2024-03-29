# Copyright (c) 2008, 2013, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

INCLUDE_DIRECTORIES(${NDB_SOURCE_DIR}/src/common/mgmcommon)

ADD_CONVENIENCE_LIBRARY(ndbmgmclient
            CommandInterpreter.cpp)

IF(WITHOUT_SERVER)
  RETURN()
ENDIF()

MYSQL_ADD_EXECUTABLE(ndb_mgm
  main.cpp
  COMPONENT ClusterManagementClient)

TARGET_LINK_LIBRARIES(ndb_mgm
               ndbmgmclient
               ndbmgmapi
               ndblogger
               ndbtrace
               ndbportlib
               ndbgeneral
               dbug
               mysys
               strings)
IF(UNIX)
  TARGET_LINK_LIBRARIES(ndb_mgm ${EDITLINE_LIBRARY})
ENDIF(UNIX)
 