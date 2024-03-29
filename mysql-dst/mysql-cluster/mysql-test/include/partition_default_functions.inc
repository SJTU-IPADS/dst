# Help file for avoid duplicated tests of different DEFAULT functions
#
# Needs $get_handler_status_counts defined
# normally as:
# let $get_handler_status_counts=
# SELECT * FROM INFORMATION_SCHEMA.SESSION_STATUS
# WHERE VARIABLE_NAME LIKE 'HANDLER_%' AND VARIABLE_VALUE > 0;
#
# And a table t3 where a is a DATETIME/DATE/TIME/TIMESTAMP column
# and b is a char/varchar() column.

if ($null_as_now)
{
FLUSH STATUS;
SET TIMESTAMP = 1234567890;
INSERT INTO t3 (a) VALUES (NULL);
--source include/get_handler_status_counts.inc
}
FLUSH STATUS;
SET TIMESTAMP = 1234567891;
INSERT INTO t3 VALUES ();
--source include/get_handler_status_counts.inc
FLUSH STATUS;
INSERT INTO t3 (a) VALUES ('2011-01-01 00:00:00');
--source include/get_handler_status_counts.inc
FLUSH STATUS;
INSERT INTO t3 (a) VALUES ('2011-01-01 00:00:01');
--source include/get_handler_status_counts.inc
FLUSH STATUS;
INSERT INTO t3 (a) VALUES ('2011-01-01 00:00:02'), ('2011-01-01 00:00:03');
--source include/get_handler_status_counts.inc
--echo # 2 writes
FLUSH STATUS;
SET TIMESTAMP = 1234567892;
INSERT INTO t3 (a) VALUES ('2011-01-01 00:00:00')
ON DUPLICATE KEY UPDATE b = "DUP_KEY";
--source include/get_handler_status_counts.inc
if ($default_update)
{
--echo # No pruning due to DEFAULT function on partitioning column
--echo # 1 read_key + 1 delete + 2 write (1 failed + 1 ok)
--echo # 1 delete + 1 write due to moved to different partition
}
if (!$default_update)
{
--echo # 1 read_key + 1 update (same partition)
--echo # 1 (failed) write
}
FLUSH STATUS;
INSERT INTO t3 (a) VALUES ('2011-01-01 00:00:01')
ON DUPLICATE KEY UPDATE a = '2011-01-01 00:00:05', b = "DUP_KEY2";
--source include/get_handler_status_counts.inc
--echo # No pruning due to updating partitioning field.
--echo # 1 read_key + 1 delete + 2 write (1 failed + 1 ok)
--echo # 1 delete + 1 write due to moved to different partition
FLUSH STATUS;
SET TIMESTAMP = 1234567893;
UPDATE t3 SET b = 'Updated' WHERE a = '2011-01-01 00:00:02';
--source include/get_handler_status_counts.inc
if ($default_update)
{
--echo # No pruning due to DEFAULT function on partitioning column
--echo # 2 read_key + 1 read_rnd (1 read_key due to index lookup,
--echo # 1 read_rnd + 1 read_key due to positioning before update)
--echo # 1 delete + 1 write due to moved to different partition
--echo # + 1 (failed) write
}
if (!$default_update)
{
--echo # 1 read_key + 1 update (same partition)
--echo # 1 (failed) write
}
FLUSH STATUS;
REPLACE INTO t3 VALUES ('2011-01-01 00:00:04', 'Replace1');
--source include/get_handler_status_counts.inc
FLUSH STATUS;
REPLACE INTO t3 VALUES ('2011-01-01 00:00:04', 'Replace2');
--source include/get_handler_status_counts.inc
--echo # 1 read_key + 1 update + 1 failed write

--echo #
--echo # Test of replace of default PK (delete might be needed first)
--echo #
DELETE FROM t3 WHERE a = 0;
FLUSH STATUS;
SET TIMESTAMP = 1234567894;
REPLACE INTO t3 (b) VALUES ('Replace3');
--source include/get_handler_status_counts.inc
FLUSH STATUS;
SET TIMESTAMP = 1234567894;
REPLACE INTO t3 (b) VALUES ('Replace4');
--source include/get_handler_status_counts.inc
--echo # 1 read_key + 1 update + 1 failed write
--sorted_result
SELECT * FROM t3;
 