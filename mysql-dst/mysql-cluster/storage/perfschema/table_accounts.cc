/* Copyright (c) 2011, 2015, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include "my_global.h"
#include "my_thread.h"
#include "table_accounts.h"
#include "pfs_instr_class.h"
#include "pfs_instr.h"
#include "pfs_account.h"
#include "pfs_visitor.h"
#include "pfs_memory.h"
#include "pfs_status.h"
#include "field.h"

THR_LOCK table_accounts::m_table_lock;

static const TABLE_FIELD_TYPE field_types[]=
{
  {
    { C_STRING_WITH_LEN("USER") },
    { C_STRING_WITH_LEN("char(" USERNAME_CHAR_LENGTH_STR ")") },
    { NULL, 0}
  },
  {
    { C_STRING_WITH_LEN("HOST") },
    { C_STRING_WITH_LEN("char(60)") },
    { NULL, 0}
  },
  {
    { C_STRING_WITH_LEN("CURRENT_CONNECTIONS") },
    { C_STRING_WITH_LEN("bigint(20)") },
    { NULL, 0}
  },
  {
    { C_STRING_WITH_LEN("TOTAL_CONNECTIONS") },
    { C_STRING_WITH_LEN("bigint(20)") },
    { NULL, 0}
  }
};

TABLE_FIELD_DEF
table_accounts::m_field_def=
{ 4, field_types };

PFS_engine_table_share
table_accounts::m_share=
{
  { C_STRING_WITH_LEN("accounts") },
  &pfs_truncatable_acl,
  table_accounts::create,
  NULL, /* write_row */
  table_accounts::delete_all_rows,
  cursor_by_account::get_row_count,
  sizeof(PFS_simple_index), /* ref length */
  &m_table_lock,
  &m_field_def,
  false, /* checked */
  false  /* perpetual */
};

PFS_engine_table* table_accounts::create()
{
  return new table_accounts();
}

int
table_accounts::delete_all_rows(void)
{
  reset_events_waits_by_thread();
  reset_events_waits_by_account();
  reset_events_stages_by_thread();
  reset_events_stages_by_account();
  reset_events_statements_by_thread();
  reset_events_statements_by_account();
  reset_events_transactions_by_thread();
  reset_events_transactions_by_account();
  reset_memory_by_thread();
  reset_memory_by_account();
  reset_status_by_thread();
  reset_status_by_account();
  purge_all_account();
  return 0;
}

table_accounts::table_accounts()
  : cursor_by_account(& m_share),
  m_row_exists(false)
{}

void table_accounts::make_row(PFS_account *pfs)
{
  pfs_optimistic_state lock;

  m_row_exists= false;
  pfs->m_lock.begin_optimistic_lock(&lock);

  if (m_row.m_account.make_row(pfs))
    return;

  PFS_connection_stat_visitor visitor;
  PFS_connection_iterator::visit_account(pfs,
                                         true,  /* threads */
                                         false, /* THDs */
                                         & visitor);

  if (! pfs->m_lock.end_optimistic_lock(& lock))
    return;

  m_row.m_connection_stat.set(& visitor.m_stat);
  m_row_exists= true;
}

int table_accounts::read_row_values(TABLE *table,
                                      unsigned char *buf,
                                      Field **fields,
                                      bool read_all)
{
  Field *f;

  if (unlikely(! m_row_exists))
    return HA_ERR_RECORD_DELETED;

  /* Set the null bits */
  DBUG_ASSERT(table->s->null_bytes == 1);
  buf[0]= 0;

  for (; (f= *fields) ; fields++)
  {
    if (read_all || bitmap_is_set(table->read_set, f->field_index))
    {
      switch(f->field_index)
      {
      case 0: /* USER */
      case 1: /* HOST */
        m_row.m_account.set_field(f->field_index, f);
        break;
      case 2: /* CURRENT_CONNECTIONS */
      case 3: /* TOTAL_CONNECTIONS */
        m_row.m_connection_stat.set_field(f->field_index - 2, f);
        break;
      default:
        DBUG_ASSERT(false);
      }
    }
  }
  return 0;
}

 