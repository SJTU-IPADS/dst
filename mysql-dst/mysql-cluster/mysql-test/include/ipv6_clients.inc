--exec $MYSQLADMIN --no-defaults --default-character-set=latin1 -h $IPv6 -P $MASTER_MYPORT  -u root ping 
--disable_result_log
--exec $MYSQL_CHECK -h $IPv6 -P $MASTER_MYPORT  -u root test 
--exec $MYSQL_DUMP -h $IPv6 -P $MASTER_MYPORT  -u root test 
--exec $MYSQL_SHOW -h $IPv6 -P $MASTER_MYPORT  -u root  
--exec $MYSQL --host=$IPv6 --port=$MASTER_MYPORT --user=root test -e "SELECT current_user();SELECT user();"
--enable_result_log
 