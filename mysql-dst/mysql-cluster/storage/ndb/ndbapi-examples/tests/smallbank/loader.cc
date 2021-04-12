#include <mysql.h>
#include <mysqld_error.h>
#include <NdbApi.hpp>

#include <stdio.h>

#ifndef RDMA_DEBUG
#define RDMA_DEBUG
#endif

#include <logging.hpp>

#include <stdlib.h>
// Used for cout
#include <stdio.h>
#include <iostream>

#include <unistd.h>

#include "dbheader.hpp"
#include "random.hpp"
#include "bank.hpp"
#include "db_helper.hpp"

#include "common/error_handling.hpp"
#include "common/array_adapter.hpp"
#include "common/ndb_util.hpp"
#include "common/util.hpp"

using namespace db;

static void db_load(MYSQL &, Ndb_cluster_connection &,int /* num partition*/, int /* whether to force load*/);

NdbOperation::LockMode lock_mode = NdbOperation::LM_Read;
bool sanity_checks = true;

int main(int argc, char** argv)
{
  if (argc < 4)
  {
    std::cout << "Arguments are <socket mysqld> <connect_string cluster> <sql_node> <num_partitions = 2> <whether to force load>.\n";
    exit(-1);
  }
  RDMA_LOG(3) << "Smallbank loader started";

  // ndb_init must be called first
  ndb_init();

  // connect to mysql server and cluster and run application
  {
    char * mysqld_sock  = argv[1];
    const char *connectstring = argv[2];
    const char *sql_address   = argv[3];
    int num_partition = 2;
    if (argc > 4) {
      num_partition = atoi(argv[4]);
    }
    int whether_load = 1;
    if (argc > 5) {
      whether_load = atoi(argv[5]);
    }

    // Object representing the cluster
    Ndb_cluster_connection cluster_connection(connectstring);

    // Connect to cluster management server (ndb_mgmd)
    if (cluster_connection.connect(8 /* retries               */,
                                   1 /* delay between retries */,
                                   1 /* verbose               */))
    {
      std::cout << "Cluster management server was not ready within 30 secs.\n";
      exit(-1);
    }

    // Optionally connect and wait for the storage nodes (ndbd's)
    if (cluster_connection.wait_until_ready(30,0) < 0)
    {
      std::cout << "Cluster was not ready within 30 secs.\n";
      exit(-1);
    }

    // connect to mysql server
    MYSQL mysql;
    if ( !mysql_init(&mysql) ) {
      std::cout << "mysql_init failed\n";
      exit(-1);
    }
    if ( !mysql_real_connect(&mysql,sql_address, "root", "123", "",
                             0,mysqld_sock, 0) )
      MYSQLERROR(mysql);

    // run the application code
    db_load(mysql, cluster_connection,num_partition,whether_load);
  }

  ndb_end(0);

  return 0;
}

static void create_table(MYSQL &,const std::string &tb_name,int num_partition);
static void create_and_usedb(MYSQL &);
static void do_insert(Ndb &);

static void db_load(MYSQL &mysql,
                    Ndb_cluster_connection &cluster_connection,int ps,int whether_load)
{
  if (whether_load) {
    create_and_usedb(mysql);
    create_table(mysql,::db::bank::check_table,ps);
    create_table(mysql,::db::bank::save_table,ps);
  }

  // a Ndb class can only be used in one threads
  Ndb myNdb(&cluster_connection, ::db::bank::db_name.c_str());
  if (myNdb.init()) APIERROR(myNdb.getNdbError());

  if (whether_load) {

    // real load operations
    do_insert(myNdb);
  }

  // do sanity checks
  FastRandom rand(0xdeadbeaf);
  ::db::bank::check_consistency(myNdb,rand);
}

/*********************************************************
 * Create a database named micro if it not exists
 *********************************************************/
static void create_and_usedb(MYSQL &mysql) {
  std::string create_stat = "CREATE DATABASE IF NOT EXISTS "; create_stat.append(::db::bank::db_name);
  if (mysql_query(&mysql, create_stat.c_str()) != 0)
    MYSQLERROR(mysql);

  std::string use_stat = "USE "; use_stat.append(bank::db_name);
  if (mysql_query(&mysql, use_stat.c_str()) != 0)
    MYSQLERROR(mysql);
}

#ifndef RDMA_DEBUG
static_assert(false, "RDMA_DEBUG must be defined!");
#endif
/*********************************************************
 * Create a table named api_simple if it does not exist *
 * XD: we can use such a function for both smallbank's table, since *
 * they have the same schema".
 *********************************************************/
static void create_table(MYSQL &mysql,const std::string &table_name,int num_partitions = 2)
{
  std::string create_str =  "CREATE TABLE ";
  create_str.append(table_name);
  create_str.append("    (ID INT UNSIGNED NOT NULL,"
                    "     BALANCE DOUBLE NOT NULL,"
                    "     ACCOUNT VARCHAR(20), "
                    "PRIMARY KEY USING HASH (ID) )"
                    "  ENGINE=NDB\n");
  create_str.append("PARTITION BY KEY()\n"); // XD: just like hashing, according to https://dev.mysql.com/doc/mysql-partitioning-excerpt/5.5/en/partitioning-key.html
  create_str.append("PARTITIONS ");// partition keyword
  create_str.append(std::to_string(num_partitions));

  RDMA_LOG(5) << create_str;
  //fprintf(stdout,"%s\n",create_str.c_str());

  std::string delete_str = "DROP TABLE ";
  delete_str.append(table_name);

  while (mysql_query(&mysql,create_str.c_str()))
  {
    if (mysql_errno(&mysql) == ER_TABLE_EXISTS_ERROR)
    {
      std::cout << "MySQL Cluster already has table:  " << table_name << " "
                << "Dropping it..." << std::endl;
      mysql_query(&mysql, delete_str.c_str());
    }
    else MYSQLERROR(mysql);
  }
}

#define INVARIANT(expr) RDMA_ASSERT(expr)

inline int _CheckBetweenInclusive(int v, int lower, int upper) {
  INVARIANT(v >= lower);
  INVARIANT(v <= upper);
  return v;
}

inline int _RandomNumber(FastRandom &r, int min, int max)
{
  return _CheckBetweenInclusive((int) (r.next_uniform() * (max - min + 1) + min), min, max);
}

static void do_insert(Ndb &myNdb)
{
  const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();

  const NdbDictionary::Table *save_table  = myDict->getTable(::db::bank::save_table.c_str());
  const NdbDictionary::Table *check_table = myDict->getTable(::db::bank::check_table.c_str());

  FastRandom rand(9234); // XD: I copy this random number from code

  for (uint i = 0;i < ::db::bank::num_accounts;++i) {
    //  first we load the saving table
    NdbTransaction *tx = myNdb.startTransaction();
    RDMA_ASSERT(tx != nullptr) << "allocate TX failes";

    auto op = tx->getNdbOperation(save_table);
    RDMA_ASSERT(op != nullptr);
    op->insertTuple();
    op->equal("ID",i);
    op->setValue("BALANCE",(double)(_RandomNumber(rand,bank::min_balance,bank::max_balance)));

    //auto col = save_table->getColumn("ACCOUNT");
    auto str = rand.next_string(12);
    if(i == 0) {
      str = "deadbeaf";
      write_string_attr(op,save_table,"ACCOUNT",str);
    } else {
      write_string_attr(op,save_table,"ACCOUNT",str);
    }

    // then we load the check table
    op = tx->getNdbOperation(check_table);
    RDMA_ASSERT(op != nullptr);
    op->insertTuple();
    op->equal("ID",i);
    op->setValue("BALANCE",(double)(_RandomNumber(rand,bank::min_balance,bank::max_balance)));

    // we commit the TX
    if(tx->execute(NdbTransaction::Commit) == -1) {
      APIERROR(tx->getNdbError());
    }
    myNdb.closeTransaction(tx);
  }

  RDMA_LOG(3) << "loader sanity checks";
  auto tx = tx_start(myNdb);
  auto op = read_tuple(tx,save_table,0,NdbOperation::LM_Exclusive);
  std::string res = "none";
  auto col = read_col(op,"ACCOUNT");
  RDMA_ASSERT(commit(tx)) << "TX failed to commit";
  get_string(col,res);
  NdbDictionary::Column::Type column_type = col->getType();

  RDMA_LOG(4) << "name: " << col->getColumn()->getName() << ";"
              << " type: " << (int)column_type
              << " string type: " << column_type_to_string(column_type);

  RDMA_LOG(4) << "get results: " << res << ";"
              << "read back value: " << col->get_size_in_bytes();

  RDMA_LOG(2) << "Smallbank load done, total account loaded per table: " << ::db::bank::num_accounts;
}
 