/* Copyright (c) 2014, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA */

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"
#include <gtest/gtest.h>

#include <stdlib.h>
#include <string.h>

namespace calloc_unittest {

/*
  Set num_iterations to a reasonable value (e.g. 200000), build release
  and run with 'calloc-t --disable-tap-output' to see the time taken.
*/
#if !defined(DBUG_OFF)
// There is no point in benchmarking anything in debug mode.
static int num_iterations= 2;
#else
// Set this so that each test case takes a few seconds.
// And set it back to a small value before pushing!!
static int num_iterations= 2000;
#endif


class CallocTest : public ::testing::Test
{
protected:
  CallocTest() { };

  void malloc_test(int count);
  void calloc_test(int count);
};


void CallocTest::malloc_test(int count)
{
  for (int i= 1; i <= count; i++)
  {
    size_t size= i * 10;
    void *rawmem1= malloc(size);
    memset(rawmem1, 0, size);
    void *rawmem2= malloc(size);
    memset(rawmem2, 0, size);
    // We need to prevent the optimizer from removing the whole loop.
    EXPECT_FALSE(memcmp(rawmem1, rawmem2, 1));
    free(rawmem1);
    free(rawmem2);
  }
}


void CallocTest::calloc_test(int count)
{
  for (int i= 1; i <= count; i++)
  {
    size_t size= i * 10;
    void *rawmem1= calloc(size, 1);
    void *rawmem2= calloc(size, 1);
    // We need to prevent the optimizer from removing the whole loop.
    EXPECT_FALSE(memcmp(rawmem1, rawmem2, 1));
    free(rawmem1);
    free(rawmem2);
  }
}


TEST_F(CallocTest, WarmupTest)
{
  calloc_test(num_iterations);
}

TEST_F(CallocTest, MallocTest)
{
  malloc_test(num_iterations);
}

TEST_F(CallocTest, CallocTest)
{
  calloc_test(num_iterations);
}

}
 