# This is the first half of include/restart_mysqld.inc.
if ($rpl_inited)
{
  if (!$allow_rpl_inited)
  {
    --die ERROR IN TEST: When using the replication test framework (master-slave.inc, rpl_init.inc etc), use rpl_restart_server.inc instead of restart_mysqld.inc. If you know what you are doing and you really have to use restart_mysqld.inc, set allow_rpl_inited=1 before you source restart_mysqld.inc
  }
}

# Write file to make mysql-test-run.pl expect the "crash", but don't start it
--let $_server_id= `SELECT @@server_id`
--let $_expect_file_name= $MYSQLTEST_VARDIR/tmp/mysqld.$_server_id.expect
--exec echo "wait" > $_expect_file_name

# Send shutdown to the connected server
--shutdown_server
--source include/wait_until_disconnected.inc

 