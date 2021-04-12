DBNAME=$1
WH=$2
STEP=1

mysql -h 10.0.0.101 -P 3306 -uroot -p123 tpcc1000 < create_table.sql
./tpcc_load -h 10.0.0.101 -P 3306 -d $DBNAME -u root -p "123" -w $WH -l 1 -m 1 -n $WH >> 1.out &

x=1

while [ $x -le $WH ]
do
 echo $x $(( $x + $STEP - 1 ))
./tpcc_load -h 10.0.0.101 -P 3306 -d $DBNAME -u root -p "123" -w $WH -l 2 -m $x -n $(( $x + $STEP - 1 ))  >> 2_$x.out &
./tpcc_load -h 10.0.0.101 -P 3306 -d $DBNAME -u root -p "123" -w $WH -l 3 -m $x -n $(( $x + $STEP - 1 ))  >> 3_$x.out &
./tpcc_load -h 10.0.0.101 -P 3306 -d $DBNAME -u root -p "123" -w $WH -l 4 -m $x -n $(( $x + $STEP - 1 ))  >> 4_$x.out &
 x=$(( $x + $STEP ))
done

 