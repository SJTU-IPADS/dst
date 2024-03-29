/*
 Copyright (c) 2016, Oracle and/or its affiliates. All rights
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

#include <stdlib.h>
#include <assert.h>

#include <NdbApi.hpp>
#include <node_buffer.h>

#include "adapter_global.h"
#include "unified_debug.h"
#include "BlobHandler.h"
#include "JsWrapper.h"


// BlobHandler constructor
BlobHandler::BlobHandler(int colId, int fieldNo) :
  ndbBlob(0),
  next(0),
  content(0),
  length(0),
  columnId(colId), 
  fieldNumber(fieldNo)
{ 
}


// Helper functions for BlobReadHandler
int blobHandlerActiveHook(NdbBlob * ndbBlob, void * handler) {
  BlobReadHandler * blobHandler = static_cast<BlobReadHandler *>(handler);
  return blobHandler->runActiveHook(ndbBlob);
}

void freeBufferContentsFromJs(char *data, void *) {
  DEBUG_PRINT("Free %p", data);
  free(data);                                                // here is free
}


// BlobReadHandler methods 
void BlobReadHandler::prepare(const NdbOperation * ndbop) {
  ndbBlob = ndbop->getBlobHandle(columnId);
  assert(ndbBlob);
  ndbBlob->setActiveHook(blobHandlerActiveHook, this);

  if(next) next->prepare(ndbop);
}

int BlobReadHandler::runActiveHook(NdbBlob *b) {
  assert(b == ndbBlob);
  int isNull;
  ndbBlob->getNull(isNull);
  if(! isNull) {
    ndbBlob->getLength(length);
    uint32_t nBytes = static_cast<uint32_t>(length);
    content = (char *) malloc(length);                        // here is malloc
    if(content) {
      int rv = ndbBlob->readData(content, nBytes);
      DEBUG_PRINT("BLOB read: column %d, length %d, read %d/%d", 
                  columnId, length, rv, nBytes);
    } else {
      return -1;
    }
  }
  return 0;
}

v8::Local<v8::Object> BlobReadHandler::getResultBuffer(v8::Isolate * iso) {
  v8::Local<v8::Object> buffer;
  if(content) {
    buffer = LOCAL_BUFFER(node::Buffer::New(iso, content, length, freeBufferContentsFromJs, 0));
    /* Content belongs to someone else now; clear it for the next user */
    content = 0;
    length = 0;
  }
  return buffer;
}


// BlobWriteHandler methods

BlobWriteHandler::BlobWriteHandler(int colId, int fieldNo,
                                   v8::Handle<v8::Object> blobValue) :
  BlobHandler(colId, fieldNo)
{
  length = node::Buffer::Length(blobValue);
  content = node::Buffer::Data(blobValue);
}

void BlobWriteHandler::prepare(const NdbOperation * ndbop) {
  ndbBlob = ndbop->getBlobHandle(columnId);
  if(! ndbBlob) { 
    DEBUG_PRINT("getBlobHandle %d: [%d] %s", columnId, 
                ndbop->getNdbError().code, ndbop->getNdbError().message);
    assert(false);
  }

  DEBUG_PRINT("Prepare write for BLOB column %d, length %d", columnId, length);
  ndbBlob->setValue(content, static_cast<uint32_t>(length));
  if(next) next->prepare(ndbop);
}

 