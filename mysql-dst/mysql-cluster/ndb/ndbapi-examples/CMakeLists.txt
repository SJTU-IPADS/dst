# Copyright (c) 2014, 2015, Oracle and/or its affiliates. All rights reserved.
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
cmake_minimum_required(VERSION 3.5)

include_directories(.)
include_directories(../include)
include_directories(../include/ndbapi)
include_directories(../include/mgmapi/)
include_directories(../../../include)
include_directories(../../../libbinlogevents/export/)

set(gflags_DIR "${CMAKE_SOURCE_DIR}/third_party/gflags")
add_subdirectory(${gflags_DIR})
include_directories(${gflags_DIR}/include)

## DB clients
include_directories(./tests)
FILE(GLOB SOURCES  tests/test_client.cc tests/test_funcs.cc)
ADD_EXECUTABLE(test_loader tests/dbloader.cpp)
ADD_EXECUTABLE(bank_loader "tests/smallbank/loader.cc" "tests/smallbank/bank.cc")
ADD_EXECUTABLE(tester tests/test_main.cc ${SOURCES} tests/micro/micro1.cc tests/smallbank/bank.cc tests/tpcc/tpcc.cc tests/tpcc/payment.cc tests/tpcc/order_status.cc tests/tpcc/stock_level.cc tests/tpcc/new_order.cc tests/tpcc/delivery.cc tests/flags.cc)

ADD_EXECUTABLE(micro_loader tests/micro/dbloader.cpp)

ADD_DEFINITIONS(-std=c++14)

SET(TESTS
    test_loader
    bank_loader
    tester
    micro_loader
    )

find_package(Threads REQUIRED)

FOREACH(TEST ${TESTS})
  add_dependencies(${TEST} gflags)
  TARGET_LINK_LIBRARIES(${TEST} mysqlclient)
  TARGET_LINK_LIBRARIES(${TEST} ndbclient)
  TARGET_LINK_LIBRARIES(${TEST} Threads::Threads  ${gflags_DIR}/libgflags_nothreads.a)
ENDFOREACH()


 