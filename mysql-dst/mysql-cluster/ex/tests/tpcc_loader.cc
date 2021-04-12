#include <mysql.h>
#include <mysqld_error.h>
#include <NdbApi.hpp>

#include <stdio.h>
#include <logging.hpp>

#include <stdlib.h>
// Used for cout
#include <stdio.h>
#include <iostream>

#include <unistd.h>

#include "dbheader.hpp"

using namespace db;

static void db_load(MYSQL &, Ndb_cluster_connection &,int);

int main(int argc, char** argv)
{
  if (argc != 4)
  {
    std::cout << "Arguments are <socket mysqld> <connect_string cluster> <num_records>.\n";
    exit(-1);
  }
  // ndb_init must be called first
  ndb_init();

  // connect to mysql server and cluster and run application
  {
    char * mysqld_sock  = argv[1];
    const char *connectstring = argv[2];
    int num_records = atoi(argv[3]);

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
    if ( !mysql_real_connect(&mysql, "localhost", "root", "", "",
			     0, mysqld_sock, 0) )
      MYSQLERROR(mysql);

    // run the application code
    db_load(mysql, cluster_connection,num_records);
  }

  ndb_end(0);

  return 0;
}

static void create_table(MYSQL &);
static void do_insert(Ndb &,int);

static void db_load(MYSQL &mysql,
                    Ndb_cluster_connection &cluster_connection,
                    int num)
{
  /********************************************
   * Connect to database via mysql-c          *ndb_examples
   ********************************************/
  std::string db_stat = "USE "; db_stat.append(db_name);
  if (mysql_query(&mysql, db_stat.c_str()) != 0) MYSQLERROR(mysql);
  create_table(mysql);
  // a Ndb class can only be used in one threads
  Ndb myNdb(&cluster_connection, db_name.c_str());
  if (myNdb.init()) APIERROR(myNdb.getNdbError());

  /*
   * Do different operations on database
   */
  do_insert(myNdb,num);
}

/*********************************************************
 * Create a table named api_simple if it does not exist *
 *********************************************************/
static void create_table(MYSQL &mysql)
{
  std::string create_str =  "CREATE TABLE ";
  create_str.append(table_one);
  create_str.append("    (ID INT UNSIGNED NOT NULL PRIMARY KEY,"
                    "     ATTR INT UNSIGNED NOT NULL)"
                    "  ENGINE=NDB\n");
  create_str.append("PARTITION BY KEY()\n"); // XD: just like hashing, according to https://dev.mysql.com/doc/mysql-partitioning-excerpt/5.5/en/partitioning-key.html
  create_str.append("PARTITIONS 2"); // 2 is the number of NDB nodes
  RDMA_LOG(3) << create_str;
  std::string delete_str = "DROP TABLE ";
  delete_str.append(table_one);

  while (mysql_query(&mysql,create_str.c_str()))
  {
    if (mysql_errno(&mysql) == ER_TABLE_EXISTS_ERROR)
    {
      std::cout << "MySQL Cluster already has example table:  " << table_one << " "
      << "Dropping it..." << std::endl;
      mysql_query(&mysql, delete_str.c_str());
    }
    else MYSQLERROR(mysql);
  }
}

static void do_insert(Ndb &myNdb,int nums = 1000)
{
  const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();
  const NdbDictionary::Table *myTable= myDict->getTable(table_one.c_str());

  if (myTable == NULL)
    APIERROR(myDict->getNdbError());

  for (int i = 0; i < nums; i++) {
    NdbTransaction *myTransaction= myNdb.startTransaction();
    if (myTransaction == NULL) APIERROR(myNdb.getNdbError());

    NdbOperation *myOperation= myTransaction->getNdbOperation(myTable);
    if (myOperation == NULL) APIERROR(myTransaction->getNdbError());

    myOperation->insertTuple();
    myOperation->equal("ID", i);
    myOperation->setValue("ATTR", i + 73); // dummy attribute field

    if (myTransaction->execute( NdbTransaction::Commit) == -1)
      APIERROR(myTransaction->getNdbError());

    myNdb.closeTransaction(myTransaction);
  }
  RDMA_LOG(5) << "DB insert done";
}

 