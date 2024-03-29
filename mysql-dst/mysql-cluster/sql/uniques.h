/* Copyright (c) 2001, 2015, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef UNIQUES_INCLUDED
#define UNIQUES_INCLUDED

#include "my_tree.h"            // TREE
#include "prealloced_array.h"   // Prealloced_array
#include "sql_alloc.h"          // Sql_alloc

class Cost_model_table;

/*
   Unique -- class for unique (removing of duplicates). 
   Puts all values to the TREE. If the tree becomes too big,
   it's dumped to the file. User can request sorted values, or
   just iterate through them. In the last case tree merging is performed in
   memory simultaneously with iteration, so it should be ~2-3x faster.
 */

class Unique :public Sql_alloc
{
  Prealloced_array<Merge_chunk, 16, true> file_ptrs;
  ulong max_elements;
  ulonglong max_in_memory_size;
  IO_CACHE file;
  TREE tree;
  uchar *record_pointers;
  bool flush();
  uint size;

public:
  ulong elements;
  Unique(qsort_cmp2 comp_func, void *comp_func_fixed_arg,
	 uint size_arg, ulonglong max_in_memory_size_arg);
  ~Unique();
  ulong elements_in_tree() { return tree.elements_in_tree; }
  inline bool unique_add(void *ptr)
  {
    DBUG_ENTER("unique_add");
    DBUG_PRINT("info", ("tree %u - %lu", tree.elements_in_tree, max_elements));
    if (tree.elements_in_tree > max_elements && flush())
      DBUG_RETURN(1);
    DBUG_RETURN(!tree_insert(&tree, ptr, 0, tree.custom_arg));
  }

  bool get(TABLE *table);

  typedef Bounds_checked_array<uint> Imerge_cost_buf_type;

  static double get_use_cost(Imerge_cost_buf_type buffer,
                             uint nkeys, uint key_size, 
                             ulonglong max_in_memory_size,
                             const Cost_model_table *cost_model);

  // Returns the number of elements needed in Imerge_cost_buf_type.
  inline static size_t get_cost_calc_buff_size(ulong nkeys, uint key_size, 
                                               ulonglong max_in_memory_size)
  {
    ulonglong max_elems_in_tree=
      (max_in_memory_size / ALIGN_SIZE(sizeof(TREE_ELEMENT)+key_size));
    return 1 + static_cast<size_t>(nkeys/max_elems_in_tree);
  }

  void reset();
  bool walk(tree_walk_action action, void *walk_action_arg);

  uint get_size() const { return size; }
  ulonglong get_max_in_memory_size() const { return max_in_memory_size; }

  friend int unique_write_to_file(uchar* key, element_count count, Unique *unique);
  friend int unique_write_to_ptrs(uchar* key, element_count count, Unique *unique);
};




#endif  // UNIQUES_INCLUDED
 