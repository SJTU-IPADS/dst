/*
   Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef CONNECTION_HANDLER_IMPL_INCLUDED
#define CONNECTION_HANDLER_IMPL_INCLUDED

#include "my_global.h"
#include "mysql/psi/mysql_thread.h" // mysql_mutex_t
#include "connection_handler.h"     // Connection_handler

#include <list>

class Channel_info;
class THD;

/**
  This class represents the connection handling functionality
  that each connection is being handled in a single thread
*/
class Per_thread_connection_handler : public Connection_handler
{
  Per_thread_connection_handler(const Per_thread_connection_handler&);
  Per_thread_connection_handler&
    operator=(const Per_thread_connection_handler&);

  /**
    Check if idle threads to handle connection in
    thread cache. If so enqueue the new connection
    to be picked by the idle thread in thread cache.

    @retval false if idle pthread was found, else true.
  */
  bool check_idle_thread_and_enqueue_connection(Channel_info* channel_info);

  /**
    List of pending channel info objects to be picked by idle
    threads. Protected by LOCK_thread_cache.
  */
  static std::list<Channel_info*> *waiting_channel_info_list;

  static mysql_mutex_t LOCK_thread_cache;
  static mysql_cond_t COND_thread_cache;
  static mysql_cond_t COND_flush_thread_cache;

public:
  // Status variables related to Per_thread_connection_handler
  static ulong blocked_pthread_count;    // Protected by LOCK_thread_cache.
  static ulong slow_launch_threads;
  // System variable
  static ulong max_blocked_pthreads;

  static void init();
  static void destroy();

  /**
    Wake blocked pthreads and wait until they have terminated.
  */
  static void kill_blocked_pthreads();

  /**
    Block until a new connection arrives.
  */
  static Channel_info* block_until_new_connection();

  Per_thread_connection_handler() {}
  virtual ~Per_thread_connection_handler() { }

protected:
  virtual bool add_connection(Channel_info* channel_info);

  virtual uint get_max_threads() const;
};


/**
  This class represents the connection handling functionality
  of all connections being handled in a single worker thread.
*/
class One_thread_connection_handler : public Connection_handler
{
  One_thread_connection_handler(const One_thread_connection_handler&);
  One_thread_connection_handler&
    operator=(const One_thread_connection_handler&);

public:
  One_thread_connection_handler() {}
  virtual ~One_thread_connection_handler() {}

protected:
  virtual bool add_connection(Channel_info* channel_info);

  virtual uint get_max_threads() const { return 1; }
};

#endif // CONNECTION_HANDLER_IMPL_INCLUDED
 