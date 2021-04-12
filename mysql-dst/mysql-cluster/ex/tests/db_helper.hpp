#pragma once

/**
 * Notice: donot use this file in a .hpp or .h
 */

#include "common/error_handling.hpp"
#include "common/array_adapter.hpp"
#include "common/ndb_util.hpp"
#include "common/util.hpp"

#include <chrono>

static inline uint64_t localTS() {
#if 0
  std::chrono::time_point<std::chrono::system_clock>  t = std::chrono::system_clock::now();
  auto timestamp = std::chrono::system_clock::to_time_t(t);
  assert( (timestamp << 24 ) >> 24 == timestamp);
  timestamp = timestamp << 24; // avoids overflow of addition of timestamp
#endif
  std::time_t t = std::time(0);  // t is an integer type
  return (uint64_t)t;
}

static inline NdbTransaction *tx_start(Ndb &ndb,uint64_t timestamp = 0) {
  auto tx = ndb.startTransaction();
  RDMA_ASSERT(tx != nullptr);
  tx->timestamp = localTS();
  return tx;
}

static inline NdbOperation *get_op(NdbTransaction *tx,const NdbDictionary::Table *tab) {
  auto op = tx->getNdbOperation(tab);
  RDMA_ASSERT(op != nullptr);
  return op;
}

static inline NdbScanOperation *get_scan_op(NdbTransaction *tx,const NdbDictionary::Table *tab) {
  auto op = tx->getNdbScanOperation(tab);
  RDMA_ASSERT(op != nullptr);
  return op;
}

static inline bool execute_kernel(NdbTransaction *tx,NdbTransaction::ExecType type) {
  if(tx->execute(type, NdbOperation::AbortOnError) == -1) {
    if(tx->getNdbError().code == 266) {
      // there is deadlock
      return false;
    }
  }
  return true;
}

static inline bool pre_execute(NdbTransaction *tx) {
  return execute_kernel(tx, NdbTransaction::NoCommit);
}

static inline bool commit(NdbTransaction *tx) {
  RDMA_ASSERT(tx->timestamp != 0);
  return execute_kernel(tx, NdbTransaction::Commit);
}

static inline NdbOperation *read_tuple(NdbTransaction *tx,const NdbDictionary::Table *tab,
                                       uint64_t id,NdbOperation::LockMode mode,const std::string &primary_col = "ID") {
  auto op = get_op(tx,tab);
  op->readTuple(mode);
  op->equal(primary_col.c_str(),static_cast<Uint64>(id));
  return op;
}

static inline NdbRecAttr *read_col(NdbOperation *op,const std::string &col_name) {
  auto res = op->getValue(col_name.c_str(),nullptr);
  RDMA_ASSERT(res != nullptr);
  return res;
}

template <typename T>
static inline NdbOperation *write_tuple(NdbTransaction *tx,const NdbDictionary::Table *tab,uint64_t id,
                                        const std::string &col_name,const T &value,
                                        const std::string &primary_col = "ID"
                                        ) {
  auto op = get_op(tx,tab);
  op->updateTuple();
  op->equal(primary_col.c_str(),static_cast<Uint64>(id));
  op->setValue(col_name.c_str(),value);
  return op;
}

// some operations for strings
static inline bool write_string_attr(NdbOperation *op,const NdbDictionary::Table *tab,const std::string &attr,
                                     const std::string &value) {
  ReadWriteArrayAdapter attr_adapter;
  ReadWriteArrayAdapter::ErrorType error;

  auto col = tab->getColumn(attr.c_str());
  char *ref = attr_adapter.make_aRef(col,value.c_str(), error);
  PRINT_IF_NOT_EQUAL(error, ReadWriteArrayAdapter::Success,
                     "make_aRef failed for ATTR");
  op->setValue(attr.c_str(),ref);
}

static inline int get_byte_array(const NdbRecAttr* attr,
                          const char*& first_byte,
                          size_t& bytes)
{
  const NdbDictionary::Column::ArrayType array_type =
    attr->getColumn()->getArrayType();
  const size_t attr_bytes = attr->get_size_in_bytes();
  const char* aRef = attr->aRef();
  std::string result;

  switch (array_type) {
  case NdbDictionary::Column::ArrayTypeFixed:
    /*
     No prefix length is stored in aRef. Data starts from aRef's first byte
     data might be padded with blank or null bytes to fill the whole column
     */
    first_byte = aRef;
    bytes = attr_bytes;
    return 0;
  case NdbDictionary::Column::ArrayTypeShortVar:
    /*
     First byte of aRef has the length of data stored
     Data starts from second byte of aRef
     */
    first_byte = aRef + 1;
    bytes = (size_t)(aRef[0]);
    return 0;
  case NdbDictionary::Column::ArrayTypeMediumVar:
    /*
     First two bytes of aRef has the length of data stored
     Data starts from third byte of aRef
     */
    first_byte = aRef + 2;
    bytes = (size_t)(aRef[1]) * 256 + (size_t)(aRef[0]);
    return 0;
  default:
    first_byte = NULL;
    bytes = 0;
    return -1;
  }
}

/*
 Extracts the string from given NdbRecAttr
 Uses get_byte_array internally
 */
static inline int get_string(const NdbRecAttr* attr, std::string& str)
{
  size_t attr_bytes;
  const char* data_start_ptr = NULL;

  /* get stored length and data using get_byte_array */
  if(get_byte_array(attr, data_start_ptr, attr_bytes) == 0)
  {
    /* we have length of the string and start location */
    str= std::string(data_start_ptr, attr_bytes);
    if(attr->getType() == NdbDictionary::Column::Char)
    {
      /* Fixed Char : remove blank spaces at the end */
      size_t endpos = str.find_last_not_of(" ");
      if( std::string::npos != endpos )
      {
        str = str.substr(0, endpos+1);
      }
    }
  }
  return 0;
}
 