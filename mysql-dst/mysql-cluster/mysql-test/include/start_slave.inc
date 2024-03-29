# ==== Purpose ====
#
# Issues START SLAVE on the current connection. Then waits until both
# the IO and SQL threads have started, or until a timeout is reached.
#
# Please use this instead of 'START SLAVE', to reduce the risk of test
# case bugs.
#
#
# ==== Usage ====
#
# [--let $rpl_channel_name= NAME]
# [--let $slave_timeout= NUMBER]
# [--let $rpl_debug= 1]
# --source include/start_slave.inc
#
# Parameters:
#   $rpl_channel_name
#     If this is not '', a FOR CHANNEL  '$rpl_channel_name' is added.
#
#   $slave_timeout
#     See include/wait_for_slave_param.inc
#
#   $rpl_debug
#     See include/rpl_init.inc


--let $include_filename= start_slave.inc
--source include/begin_include_file.inc


if (!$rpl_debug)
{
  --disable_query_log
}

if ($rpl_channel_name != '')
{
  eval START SLAVE FOR CHANNEL  '$rpl_channel_name';
}
if ($rpl_channel_name == '')
{
  START SLAVE;
}

source include/wait_for_slave_to_start.inc;


--let $include_filename= start_slave.inc
--source include/end_include_file.inc
 