#pragma once

#include "client.hpp"
#include "random.hpp"
#include "dbheader.hpp"
#include "timer.h"

#include <logging.hpp>
#include <string.h>

namespace  db {

class MicroReadClient : public Client {
 public:
  MicroReadClient(int id,int seed,Ndb_cluster_connection *connect,
                  const std::string &sock,const std::string &api_host = "localhost") :
      tid(id),rand(seed),
      cluster_connection(connect),
      mysqld_sock(sock),
      api_host(api_host) {
  }

  void *run_body() {
    RDMA_LOG(2) << "init at #" << tid;
    // init mysql
    MYSQL mysql;
    if ( !mysql_init(&mysql) ) {
      RDMA_ASSERT(false) << "mysql_init failed\n";
    }
    if ( !mysql_real_connect(&mysql, api_host.c_str(), "root", "password", "",
                             0, mysqld_sock.c_str(), 0) )
      MYSQLERROR(mysql);

    // switch to our database
    std::string db_stat = "USE "; db_stat.append(db_name);
    if (mysql_query(&mysql, db_stat.c_str()) != 0) MYSQLERROR(mysql);

    // init the Ndb class
    Ndb myNdb(cluster_connection, db_name.c_str());
    if (myNdb.init()) APIERROR(myNdb.getNdbError());

    // init done, do the tests
    RDMA_LOG(3) << "test started at client #" << tid;

    Timer t; int num_reqs = 5;

    //for(uint i = 0;i < num_reqs;++i)
    //      tx_read_after_write(myNdb,NdbOperation::LM_Read);
    const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();
    const NdbDictionary::Table *myTable= myDict->getTable(table_one.c_str());

    //tx_2pl_read2(myTable,myNdb);
    //tx_read_after_write_v1(myTable, myNdb,NdbOperation::LM_SimpleRead);
    tx_read_after_write_v1(myTable, myNdb);

    double thpt = ((double)num_reqs / t.passed_msec()) * 1000000;
    RDMA_LOG(3) << "Throughput: " << thpt << " reqs / sec.";
    mysql_thread_end();
  }

  void tx_2pl_read2(const NdbDictionary::Table *myTable,Ndb &myNdb,NdbOperation::LockMode mode = NdbOperation::LM_Read) {

    NdbTransaction *myTransaction = myNdb.startTransaction();

    NdbOperation *pop = myTransaction->getNdbOperation(myTable);
    pop->readTuple(mode);
    pop->equal("ID",0);
    NdbRecAttr *myRecAttr= pop->getValue("ATTR", NULL);

    if( myTransaction->execute( NdbTransaction::Commit ) == -1 )
      APIERROR(myTransaction->getNdbError());
    printf("then: %2d    %2d\n", 0, myRecAttr->u_32_value());
    myNdb.closeTransaction(myTransaction);
  }

  void tx_read_after_write(Ndb &myNdb,NdbOperation::LockMode mode = NdbOperation::LM_Read) {

    /**
     * This TX read a value at ID 0, and increments its value by 3.
     */
    const NdbDictionary::Dictionary* myDict= myNdb.getDictionary();
    const NdbDictionary::Table *myTable= myDict->getTable(table_one.c_str());

    if (myTable == NULL)
      APIERROR(myDict->getNdbError());

    NdbTransaction *myTransaction = myNdb.startTransaction();

    NdbOperation *pop = myTransaction->getNdbOperation(myTable);

    if (pop->interpretedUpdateTuple())
      APIERROR (pop->getNdbError());
    if (pop->equal("ID", 0) != 0)
      APIERROR (pop->getNdbError());
    if (pop->read_attr("ATTR", 1) != 0)
      APIERROR (pop->getNdbError());

    // load 3 in register #5
    if (pop->load_const_u32(5, 3) != 0)
      APIERROR (pop->getNdbError());

    // add 3 to register #1, and store it in register #2
    if(pop->add_reg(1,5,2) != 0) {
      APIERROR (pop->getNdbError());
    }
    if (pop->write_attr("ATTR", 2) != 0)
      APIERROR (pop->getNdbError());

    if (pop->interpret_exit_ok() != 0)
      APIERROR (pop->getNdbError());

    NdbRecAttr *myRecAttr= pop->getValue("ATTR", NULL);

    if( myTransaction->execute( NdbTransaction::Commit ) == -1 )
      APIERROR(myTransaction->getNdbError());

    // parse the result
    if (myRecAttr == NULL) APIERROR(myTransaction->getNdbError());
    printf("then: %2d    %2d\n", 0, myRecAttr->u_32_value());

    myNdb.closeTransaction(myTransaction);
  }

  /**
   * The second version pre-execute the TX to read after write
   */
  void tx_read_after_write_v1(const NdbDictionary::Table *myTable,Ndb &myNdb,NdbOperation::LockMode mode = NdbOperation::LM_Read) {

    NdbTransaction *myTransaction = myNdb.startTransaction();

    NdbOperation *pop = myTransaction->getNdbOperation(myTable);
    pop->readTuple(mode);
    pop->equal("ID",0);
    NdbRecAttr *myRecAttr= pop->getValue("ATTR", NULL);

    if( myTransaction->execute( NdbTransaction::NoCommit ) == -1 )
      APIERROR(myTransaction->getNdbError());

    printf("then: %2d    %2d\n", 0, myRecAttr->u_32_value());

    NdbOperation *update_op = myTransaction->getNdbOperation(myTable);
    update_op->updateTuple();
    update_op->equal("ID",0);
    update_op->setValue("ATTR",myRecAttr->u_32_value() + 1);

    if( myTransaction->execute( NdbTransaction::Commit ) == -1 )
      APIERROR(myTransaction->getNdbError());

    myNdb.closeTransaction(myTransaction);
  }

 private:
  const int  tid;  // client id
  FastRandom rand;
  Ndb_cluster_connection *cluster_connection;
  std::string mysqld_sock;
  std::string api_host;
};

}
 