--exec $NDB_RESTORE --no-defaults $ndb_restore_opts -b $the_backup_id -n 1 $NDB_BACKUPS-$the_backup_id $ndb_restore_filter > $MYSQLTEST_VARDIR/tmp/tmp.dat
--exec $NDB_RESTORE --no-defaults $ndb_restore_opts -b $the_backup_id -n 2 $NDB_BACKUPS-$the_backup_id $ndb_restore_filter >> $MYSQLTEST_VARDIR/tmp/tmp.dat
--exec sort $MYSQLTEST_VARDIR/tmp/tmp.dat
--error 0,1
--remove_file $MYSQLTEST_VARDIR/tmp/tmp.dat
--let ndb_restore_opts=
--let ndb_restore_filter=
 