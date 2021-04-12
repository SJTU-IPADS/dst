/* Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.

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

class PumpObjectParserTest : public ::testing::Test
{
protected:
  PumpObjectParserTest()
  {}

  std::vector<std::pair<std::string, std::string> > list;
  std::string err;
};

#define UNITTEST_OBJECT_FILTER_PARSER
#include "../client/dump/object_filter.cc"

TEST_F(PumpObjectParserTest, empty)
{
  err= parse_inclusion_string("", list, true, false);
  EXPECT_EQ(err.length(), 0U);
}

TEST_F(PumpObjectParserTest, singe)
{
  err= parse_inclusion_string(" t1 ", list, true, false);
  EXPECT_EQ(err.length(), 0U) << "returned '" << err << "'";
  EXPECT_EQ(list.size(), 1U);
  EXPECT_EQ(list[0].first, "%");
  EXPECT_EQ(list[0].second, "t1");
}

TEST_F(PumpObjectParserTest, two_tables)
{
  err= parse_inclusion_string(" t1 , foo . bar ", list, true, false);
  EXPECT_EQ(err.length(), 0U) << "returned '" << err << "'";
  EXPECT_EQ(list.size(), 2U);
  EXPECT_EQ(list[0].first, "%");
  EXPECT_EQ(list[0].second, "t1");
  EXPECT_EQ(list[1].first, "foo");
  EXPECT_EQ(list[1].second, "bar");
}

TEST_F(PumpObjectParserTest, table_with_a_dot)
{
  err= parse_inclusion_string(" t1\\\\.t2 ", list, true, false);
  EXPECT_EQ(err.length(), 0U) << "returned '" << err << "'";
  EXPECT_EQ(list.size(), 1U);
  EXPECT_EQ(list[0].first, "%");
  EXPECT_EQ(list[0].second, "t1.t2");
}

TEST_F(PumpObjectParserTest, bad_list_escape)
{
  err= parse_inclusion_string(" t1\\.t2 ", list, true, false);
  EXPECT_NE(err.length(), 0U) << "returned '" << err << "'";
  EXPECT_EQ(list.size(), 0U);
}

TEST_F(PumpObjectParserTest, bad_table_name_escape)
{
  err= parse_inclusion_string(" t1\\\\,t2 ", list, true, false);
  EXPECT_NE(err.length(), 0U) << "returned '" << err << "'";
  EXPECT_EQ(list.size(), 0U);
}

TEST_F(PumpObjectParserTest, table_with_a_comma)
{
  err= parse_inclusion_string(" t1\\,t2 ", list, true, false);
  EXPECT_EQ(err.length(), 0U) << "returned '" << err << "'";
  EXPECT_EQ(list.size(), 1U);
  EXPECT_EQ(list[0].first, "%");
  EXPECT_EQ(list[0].second, "t1,t2");
}

TEST_F(PumpObjectParserTest, user_with_an_at)
{
  err= parse_inclusion_string(" t1\\\\@t2 ", list, true, true);
  EXPECT_EQ(err.length(), 0U) << "returned '" << err << "'";
  EXPECT_EQ(list.size(), 1U);
  EXPECT_EQ(list[0].first, "'t1@t2'");
  EXPECT_EQ(list[0].second, "%");
}
 