#include <stdlib.h>
// Used for cout
#include <stdio.h>
#include <iostream>

#include <unistd.h>

#include "micro_read_client.hpp"
#include "test_client.hpp"

using namespace db;

int main(int argc, char** argv)
{
  if (argc != 3)
  {
    std::cout << "Arguments are <socket mysqld> <connect_string cluster>.\n";
    exit(-1);
  }
  // ndb_init must be called first
  ndb_init();

  char * mysqld_sock  = argv[1];
  const char *connectstring = argv[2];
  {
  // create the cluster connection
  Ndb_cluster_connection cluster_connection(connectstring);
  // make some connection to the cluster
  if (cluster_connection.connect(8 /* retries               */,
                                 1 /* delay between retries */,
                                 1 /* verbose               */))
  {
    RDMA_LOG(4) << "Cluster management server was not ready within 30 secs.\n";
    RDMA_ASSERT(false);
  }

  // Optionally connect and wait for the storage nodes (ndbd's)
  if (cluster_connection.wait_until_ready(30,0) < 0)
  {
    RDMA_LOG(4) << "Cluster was not ready within 30 secs.\n";
    RDMA_ASSERT(false);
  }

  std::vector<MicroReadClient *> clients;
  FastRandom rand(0xdeadbeaf);

  //for(uint i = 0; i < 10;++i) {
  //    auto c = new MicroReadClient(i,rand.next(),&cluster_connection,mysqld_sock);
  //    clients.push_back(c);
  //    c->start();
  //}
  auto c = new MicroReadClient(0,rand.next(),&cluster_connection,mysqld_sock,"val04");
  c->run_body();

  for(auto c : clients) {
    c->join();
    delete c;
  }
  }
  ndb_end(0);

  return 0;
}
 