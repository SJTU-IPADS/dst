/*#include <mysql.h>*/
/*#include <mysqld_error.h>*/
/*#include <NdbApi.hpp>*/

/*#include <stdio.h>*/
/*#include <logging.hpp>*/

/*#include <stdlib.h>*/
/*// Used for cout*/
/*#include <stdio.h>*/
/*#include <iostream>*/

/*#include <unistd.h>*/

/*#include "dbheader.hpp"*/
/*#include "tpcc.hpp"*/

/*using namespace db;*/

/*static void db_load(MYSQL &, Ndb_cluster_connection &);*/

/*int main(int argc, char** argv)*/
/*{*/
/*  if (argc != 3)*/
/*  {*/
/*    std::cout << "Arguments are <socket mysqld> <connect_string cluster>.\n";*/
/*    exit(-1);*/
/*  }*/
/*  RDMA_LOG(3) << "micro loader started";*/

/*  // ndb_init must be called first*/
/*  ndb_init();*/

/*  // connect to mysql server and cluster and run application*/
/*  {*/
/*    char * mysqld_sock  = argv[1];*/
/*    const char *connectstring = argv[2];*/

/*    // Object representing the cluster*/
/*    Ndb_cluster_connection cluster_connection(connectstring);*/

/*    // Connect to cluster management server (ndb_mgmd)*/
/*    if (cluster_connection.connect(8 /* retries               */,*/
/*                                   1 /* delay between retries */,*/
/*                                   1 /* verbose               */))*/
/*    {*/
/*      std::cout << "Cluster management server was not ready within 30 secs.\n";*/
/*      exit(-1);*/
/*    }*/

/*    // Optionally connect and wait for the storage nodes (ndbd's)*/
/*    if (cluster_connection.wait_until_ready(30,0) < 0)*/
/*    {*/
/*      std::cout << "Cluster was not ready within 30 secs.\n";*/
/*      exit(-1);*/
/*    }*/

/*    // connect to mysql server*/
/*    MYSQL mysql;*/
/*    if ( !mysql_init(&mysql) ) {*/
/*      std::cout << "mysql_init failed\n";*/
/*      exit(-1);*/
/*    }*/
/*    if ( !mysql_real_connect(&mysql, "192.168.15.108", "root", "password", "",*/
/*                             0,mysqld_sock, 0) )*/
/*      MYSQLERROR(mysql);*/

/*    // run the application code*/
/*    db_load(mysql, cluster_connection);*/
/*  }*/

/*  ndb_end(0);*/

/*  return 0;*/
/*}*/

/*static void create_table(MYSQL &);*/
/*static void create_and_usedb(MYSQL &);*/
/*static void do_insert(Ndb &);*/

/*static void db_load(MYSQL &mysql,*/
/*                    Ndb_cluster_connection &cluster_connection)*/
/*{*/
/*  create_and_usedb(mysql);*/
/*  create_table(mysql);*/
/*  // a Ndb class can only be used in one threads*/
/*  Ndb myNdb(&cluster_connection, micro1::db_name.c_str());*/
/*  if (myNdb.init()) APIERROR(myNdb.getNdbError());*/

/*  // real load operations*/
/*  do_insert(myNdb);*/
/*}*/

/*/**********************************************************/
/* * Create a database named micro if it not exists*/
/* *********************************************************/*/
/*static void create_and_usedb(MYSQL &mysql) {*/
/*  std::string create_stat = "CREATE DATABASE IF NOT EXISTS "; create_stat.append(micro1::db_name);*/
/*  if (mysql_query(&mysql, create_stat.c_str()) != 0)*/
/*    MYSQLERROR(mysql);*/

/*  std::string use_stat = "USE "; use_stat.append(micro1::db_name);*/
/*  if (mysql_query(&mysql, use_stat.c_str()) != 0)*/
/*    MYSQLERROR(mysql);*/
/*}*/

/*/**********************************************************/
/* * Create a table named api_simple if it does not exist **/
/* *********************************************************/*/
/*static void create_table(MYSQL &mysql)*/
/*{*/
/*  std::string create_str =  "CREATE TABLE ";*/
/*  create_str.append(micro1::table_name);*/
/*  create_str.append("    (ID INT UNSIGNED NOT NULL PRIMARY KEY,"*/
/*                    "     ATTR INT UNSIGNED NOT NULL)"*/
/*                    "  ENGINE=NDB\n");*/
/*  create_str.append("PARTITION BY KEY()\n"); // XD: just like hashing, according to https://dev.mysql.com/doc/mysql-partitioning-excerpt/5.5/en/partitioning-key.html*/
/*  create_str.append("PARTITIONS 2"); // 2 is the number of NDB nodes*/
/*  RDMA_LOG(3) << create_str;*/
/*  std::string delete_str = "DROP TABLE ";*/
/*  delete_str.append(micro1::table_name);*/

/*  while (mysql_query(&mysql,create_str.c_str()))*/
/*  {*/
/*    if (mysql_errno(&mysql) == ER_TABLE_EXISTS_ERROR)*/
/*    {*/
/*      std::cout << "MySQL Cluster already has table:  " << micro1::table_name << " "*/
/*                << "Dropping it..." << std::endl;*/
/*      mysql_query(&mysql, delete_str.c_str());*/
/*    }*/
/*    else MYSQLERROR(mysql);*/
/*  }*/
/*}*/

/*static void do_insert(Ndb &myNdb)*/
/*{*/
/*  const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();*/
/*  const NdbDictionary::Table *myTable= myDict->getTable(micro1::table_name.c_str());*/

/*  if (myTable == NULL)*/
/*    APIERROR(myDict->getNdbError());*/

/*  for(int i = 0;i < micro1::total_records;++i) {*/
/*    NdbTransaction *myTransaction= myNdb.startTransaction();*/
/*    if (myTransaction == NULL) APIERROR(myNdb.getNdbError());*/

/*    NdbOperation *myOperation= myTransaction->getNdbOperation(myTable);*/
/*    if (myOperation == NULL) APIERROR(myTransaction->getNdbError());*/

/*    myOperation->insertTuple();*/
/*    myOperation->equal("ID", i);*/
/*    myOperation->setValue("ATTR", i + 73); // dummy attribute field*/

/*    if (myTransaction->execute( NdbTransaction::Commit) == -1)*/
/*      APIERROR(myTransaction->getNdbError());*/

/*    myNdb.closeTransaction(myTransaction);*/
/*  }*/
/*  RDMA_LOG(5) << "Micro1 insert done";*/
/*}*/

 