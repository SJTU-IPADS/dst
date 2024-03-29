/* Copyright (c) 2010, 2015, Oracle and/or its affiliates. All rights reserved. 

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

#ifndef BOUNDED_QUEUE_INCLUDED
#define BOUNDED_QUEUE_INCLUDED

#include "my_global.h"
#include "my_base.h"
#include "my_sys.h"
#include "mysys_err.h"
#include "priority_queue.h"
#include "malloc_allocator.h"

/**
  A priority queue with a fixed, limited size.

  This is a wrapper on top of Priority_queue.
  It keeps the top-N elements which are inserted.

  Elements of type Element_type are pushed into the queue.
  For each element, we call a user-supplied Key_generator::make_sortkey(),
  to generate a key of type Key_type for the element.
  Instances of Key_type are compared with the user-supplied Key_compare.

  Pointers to the top-N elements are stored in the sort_keys array given
  to the init() function below. To access elements in sorted order,
  sort the array and access it sequentially.
 */
template<typename Element_type,
         typename Key_type,
         typename Key_generator,
         typename Key_compare = std::less<Key_type>
        >
class Bounded_queue
{
public:
  typedef Priority_queue<Key_type,
                         std::vector<Key_type, Malloc_allocator<Key_type> >,
                         Key_compare> Queue_type;

  typedef typename Queue_type::allocator_type allocator_type;

  explicit Bounded_queue(const allocator_type
                         &alloc = allocator_type(PSI_NOT_INSTRUMENTED))
    : m_queue(Key_compare(), alloc),
      m_sort_keys(NULL),
      m_sort_param(NULL)
  {}

  /**
    Initialize the queue.

    @param max_elements   The size of the queue.
    @param sort_param     Sort parameters. We call sort_param->make_sortkey()
                          to generate keys for elements.
    @param[in,out] sort_keys Array of keys to sort.
                             Must be initialized by caller.
                             Will be filled with pointers to the top-N elements.

    @retval false OK, true Could not allocate memory.

    We do *not* take ownership of any of the input pointer arguments.
   */
  bool init(ha_rows max_elements,
            Key_generator *sort_param,
            Key_type *sort_keys)
  {
    m_sort_keys= sort_keys;
    m_sort_param= sort_param;
    DBUG_EXECUTE_IF("bounded_queue_init_fail",
                    my_error(EE_OUTOFMEMORY, MYF(ME_FATALERROR), 42);
                    return true;);

    // We allocate space for one extra element, for replace when queue is full.
    if (m_queue.reserve(max_elements + 1))
      return true;
    m_queue.m_compare_length= sort_param->compare_length();
    return false;
  }

  /**
    Pushes an element on the queue.
    If the queue is already full, we discard one element.
    Calls m_sort_param::make_sortkey() to generate a key for the element.

    @param element        The element to be pushed.
   */
  void push(Element_type element)
  {
    if (m_queue.size() == m_queue.capacity())
    {
      const Key_type &pq_top= m_queue.top();
      m_sort_param->make_sortkey(pq_top, element);
      m_queue.update_top();
    } else {
      m_sort_param->make_sortkey(m_sort_keys[m_queue.size()], element);
      m_queue.push(m_sort_keys[m_queue.size()]);
    }
  }

  /**
    The number of elements in the queue.
   */
  size_t num_elements() const { return m_queue.size(); }

private:
  Queue_type         m_queue;
  Key_type          *m_sort_keys;
  Key_generator     *m_sort_param;
};

#endif  // BOUNDED_QUEUE_INCLUDED
 