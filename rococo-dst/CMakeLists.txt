cmake_minimum_required(VERSION 2.8.4)
project(rococo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Threads)
find_package(PkgConfig REQUIRED)
pkg_check_modules(APR REQUIRED apr-1)

IF(APPLE)
ELSE()
pkg_check_modules(PYTHON2 REQUIRED python2)
ENDIF()

link_directories(
    ${PYTHON2_LIBRARY_DIRS}
    ${APR_LIBRARY_DIRS}
    )

include_directories(
    ${PYTHON2_INCLUDE_DIRS}
    ${APR_INCLUDE_DIRS}
    .
    rrr
    memdb
    deptran
    bench)

add_library(ROCOCO
    rrr/base/all.hpp
    rrr/base/basetypes.cpp
    rrr/base/basetypes.hpp
    rrr/base/debugging.cpp
    rrr/base/debugging.hpp
    rrr/base/logging.cpp
    rrr/base/logging.hpp
    rrr/base/misc.cpp
    rrr/base/misc.hpp
    rrr/base/strop.cpp
    rrr/base/strop.hpp
    rrr/base/threading.cpp
    rrr/base/threading.hpp
    rrr/base/unittest.cpp
    rrr/base/unittest.hpp
    rrr/misc/alarm.hpp
    rrr/misc/alock.cpp
    rrr/misc/alock.hpp
    rrr/misc/cpuinfo.hpp
    rrr/misc/dball.hpp
    rrr/misc/marshal.cpp
    rrr/misc/marshal.hpp
    rrr/misc/rand.cpp
    rrr/misc/rand.hpp
    rrr/misc/recorder.cpp
    rrr/misc/recorder.hpp
    rrr/misc/stat.hpp
    rrr/mq/buf.cpp
    rrr/mq/buf.h
    rrr/mq/client.cpp
    rrr/mq/client.h
    rrr/mq/polling.cpp
    rrr/mq/polling.h
    rrr/mq/rpc.cpp
    rrr/mq/rpc.h
    rrr/mq/rpc_comm.h
    rrr/mq/server.cpp
    rrr/mq/server.h
    rrr/pylib/simplerpc/_pyrpc.cpp
    rrr/rpc/client.cpp
    rrr/rpc/client.hpp
    rrr/rpc/polling.cpp
    rrr/rpc/polling.hpp
    rrr/rpc/server.cpp
    rrr/rpc/server.hpp
    rrr/rpc/utils.cpp
    rrr/rpc/utils.hpp
    rrr/utils/hostname.h
    rrr/utils/logger.h
    rrr/utils/mht.h
    rrr/utils/mlock.h
    rrr/utils/mpr_dag.h
    rrr/utils/mpr_hash.h
    rrr/utils/mpr_queue.h
    rrr/utils/mpr_thread_pool.h
    rrr/utils/mtime.h
    rrr/utils/safe_assert.h
    rrr/rrr.h
    rrr/rrr.hpp
    memdb/blob.h
    memdb/locking.h
    memdb/MurmurHash3.cc
    memdb/MurmurHash3.h
    memdb/row.cc
    memdb/row.h
    memdb/schema.cc
    memdb/schema.h
    memdb/snapshot.h
    memdb/table.cc
    memdb/table.h
    memdb/txn.cc
    memdb/txn.h
    memdb/utils.cc
    memdb/utils.h
    memdb/value.cc
    memdb/value.h
    memdb/xxhash.cc
    memdb/xxhash.h
    bench/micro/chopper.cc
    bench/micro/chopper.h
    bench/micro/piece.cc
    bench/micro/piece.h
    bench/rw_benchmark/chopper.cc
    bench/rw_benchmark/chopper.h
    bench/rw_benchmark/piece.cc
    bench/rw_benchmark/piece.h
    bench/tpca/chopper.h
    bench/tpca/piece.cc
    bench/tpca/piece.h
    bench/tpcc/chopper.cc
    bench/tpcc/chopper.h
    bench/tpcc/piece.cc
    bench/tpcc/piece.h
    bench/tpcc/stock_level.cpp
    bench/tpcc/order_status.cpp
    bench/tpcc/new_order.cpp
    bench/tpcc/delivery.cpp
    bench/tpcc/payment.cpp
    bench/tpcc_dist/chopper.cc
    bench/tpcc_dist/chopper.h
    bench/tpcc_dist/piece.h
    bench/tpcc_real_dist/chopper.cc
    bench/tpcc_real_dist/chopper.h
    bench/tpcc_real_dist/piece.h
    deptran/__init__.py
    deptran/all.h
    deptran/batch_start_args_helper.cc
    deptran/batch_start_args_helper.h
    deptran/benchmark_control_rpc.cc
    deptran/benchmark_control_rpc.h
    deptran/c_main.cc
    deptran/config.cpp
    deptran/config.hpp
    deptran/constants.hpp
    deptran/coordinator.cc
    deptran/coordinator.h
    deptran/dep_graph.cc
    deptran/dep_graph.hpp
    deptran/rcc_row.cpp
    deptran/rcc_row.hpp
    deptran/ro6_row.hpp
    deptran/ro6_row.cpp
    deptran/dtxn.cpp
    deptran/dtxn.hpp
    deptran/tpl.hpp
    deptran/tpl.cpp
    deptran/rcc.cpp
    deptran/rcc.hpp
    deptran/ro6.hpp
    deptran/ro6.cpp
    deptran/graph.cc
    deptran/graph.hpp
    deptran/inactive_client.h
    deptran/inactive_input.h
    deptran/inactive_piecegraph.h
    deptran/marshal-value.cc
    deptran/marshal-value.h
    deptran/pie_info.h
    deptran/piece.cpp
    deptran/piece.hpp
    deptran/rcc_rpc.h
    deptran/rcc_rpc.py
    deptran/rcc_rpc.rpc
    deptran/rcc_service.cc
    deptran/rcc_service.h
    deptran/s_main.cc
    deptran/sharding.cpp
    deptran/sharding.h
    deptran/timer.cc
    deptran/timer.h
    deptran/txn-chopper-factory.cc
    deptran/txn-chopper-factory.h
    deptran/txn_chopper.cc
    deptran/txn_chopper.h
    deptran/txn_info.h
    deptran/txn_reg.hpp
    deptran/txn_req_factory.cc
    deptran/txn_req_factory.h deptran/compress.hpp)

add_executable(ROCOCO_SERVER deptran/s_main.cc)
add_executable(ROCOCO_CLIENT deptran/c_main.cc)

target_link_libraries(
    ROCOCO_SERVER
    ROCOCO
    ${CMAKE_THREAD_LIBS_INIT}
    )

target_link_libraries(
    ROCOCO_CLIENT
    ROCOCO
    ${CMAKE_THREAD_LIBS_INIT}
    )

#add_executable(ROCOCO_SERVER, deptran/s_main.cc)
#add_executable(ROCOCO_CLIENT ${SOURCE_FILES})
 