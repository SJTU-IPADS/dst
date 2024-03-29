/*
 Copyright (c) 2013, 2016, Oracle and/or its affiliates. All rights
 reserved.
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; version 2 of
 the License.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 02110-1301  USA
 */

#include "adapter_global.h"
#include "ColumnHandler.h"
#include "BlobHandler.h"
#include "JsWrapper.h"
#include "js_wrapper_macros.h"

using namespace v8;


ColumnHandler::ColumnHandler() :
  column(0), offset(0), 
  isLob(false), isText(false)
{
}


ColumnHandler::~ColumnHandler() {
  // Persistent handles will be disposed by calling of their destructors
}

void ColumnHandler::init(v8::Isolate * isolate,
                         const NdbDictionary::Column *_column,
                         uint32_t _offset) {
  column = _column;
  encoder = getEncoderForColumn(column);
  offset = _offset;

  switch(column->getType()) {
    case NDB_TYPE_TEXT: 
      isText = true;   // fall through to also set isLob
    case NDB_TYPE_BLOB:
      isLob = true;
      break;
    default:
      break;
  }
}


Handle<Value> ColumnHandler::read(char * rowBuffer, Handle<Object> blobBuffer) const {
  Handle<Value> val;  // HandleScope is in ValueObject.cpp nroGetter

  if(isText) {
    DEBUG_PRINT("text read");
    val = getTextFromBuffer(column, blobBuffer);
  } else if(isLob) {
    DEBUG_PRINT("blob read");
    val = Handle<Value>(blobBuffer);
  } else {
    val = encoder->read(column, rowBuffer, offset);
  }
  return val;
}


// If column is a blob, val is the blob buffer
Handle<Value> ColumnHandler::write(Handle<Value> val, char *buffer) const {
  DEBUG_PRINT("write %s", column->getName());
  return encoder->write(column, val, buffer, offset);
}


BlobWriteHandler * ColumnHandler::createBlobWriteHandle(Local<Value> val,
                                                        int fieldNo) const {
  DEBUG_MARKER(UDEB_DETAIL);
  BlobWriteHandler * b = 0;
  Handle<Object> nodeBuffer;
  if(isLob) {
    nodeBuffer = (isText && val->IsString()) ?
       getBufferForText(column, val->ToString()) :  // TEXT
       val->ToObject();                             // BLOB
    b = new BlobWriteHandler(column->getColumnNo(), fieldNo, nodeBuffer);
  }
  return b;
}

 