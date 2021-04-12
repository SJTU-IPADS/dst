#!/usr/bin/env python

from courier import *

from func_timeout import func_set_timeout
import getpass
import time
import argparse
import func_timeout

from subprocess import call

"""
Pre-request:
- MySQLD should be initialized at the SQL node.
"""

"""
Warning! these constants are used for **our** usage
/cock/mysql/config is an NFS which is mounted to all server
"""
config_path = "/cock/mysql/config"
build_dir   = "/cock/mysql/build"

start_manage_node = "ndb_mgmd --config-file %s/%s"  ## reload is very important,  to reload the configuration file
start_data_node   = "ndbd --defaults-extra-file=%s/my.conf" %  config_path
start_sql_node    = "mysqld --defaults-file=/cock/mysql/config/my.conf --datadir=/home/wxd/mysqld-data --log-error=/home/wxd/mysqld-base/"
stop_cluster_cmd  = "ndb_mgm -e shutdown"
stop_sqlnode_cmd  = "mysqladmin -u root shutdown --socket=%s"

debug_flag        = "--debug=d,info,error,query,general,where:O,/home/wxd/debug/trace"
reload_flag       = "--reload"
reload_data_flag  = "--initial"

default_mysql_sock = "/home/wxd/mysql-base/mysql.sock"

"""
TODO, currently I hard code this in this script, maybe add a parse program to parse the config.ini
anyway, only I use this
"""
reload_clean       = "rm /cock/mysql/build/mysql-cluster/*;"
reload_dataclean0  = "rm -rf /home/wxd/mysql-data/*;"
reload_dataclean1  = "rm -rf /home/wxd/mysql-data/*;"
reload_clean_data  = "rm -rf %s/*"

"""
The environment used for mysql
"""
base_env  = {"MYPATH":build_dir,
             "PATH":"$MYPATH/bin:$PATH",
             "LD_LIBRARY_PATH":"$MYPATH/lib:$LD_LIBRARY_PATH"}

def check_keywords(lines,keywords,black_keywords):
    match = []
    for l in lines:
        black = False
        for bk in black_keywords:
            if l.find(bk) >= 0:
                black = True
                break
        if black:
            continue
        flag = True
        for k in keywords:
            flag = flag and (l.find(k) >= 0)
        if flag:
            match.append(l)
    return len(match)

@func_set_timeout(10)
def check_manager_live(manager,cr):
    while True:
        ret,err = cr.execute("ps aux | grep ndb_mgmd",manager,False,timeout = 5)
        if not ret:
            return False,err
        if check_keywords(err.split("\n"),["ndb_mgmd","wxd"],["grep"]):
            return True,""
    return False,""

@func_set_timeout(60)
def stop_manager(manager,cr):
    if not manager:
        return True,""
    ret,err = cr.execute(stop_cluster_cmd,manager,True,timeout = 5)
    if not ret:
        return False,err
    ## check that the cluster has really stopped
    while True:
        ret,err = cr.execute("ps aux | grep ndb_mgmd",manager,False,timeout = 5)
        if not ret:
            return False,err
        if check_keywords(err.split("\n"),["ndb_mgmd","wxd"],["grep"]):
            continue
        break
    return True,""

def check_datanodes_liveness(datanodes,cr,live=True):
    hosts = set()
    for d in datanodes:
        hosts.add(d)

    counter = 0
    live_set = []

    while True:
        live_set = []
        for d in hosts:
            ret,err = cr.execute("ps aux | grep ndbd",d,False,timeout = 5)
            if not ret:
                return False,live_set
            match = check_keywords(err.split("\n"),["ndbd","wxd"],["grep"])
            live_set.extend([d for i in range(match)])
        if live:
            if len(live_set) == len(datanodes) * 2: # there are *two* ndb instance on one node
                break
        else:
            if len(live_set) == 0:
                break
        time.sleep(1)
        if counter > 30:
            return False,live_set
        counter += 1
    return True,live_set

@func_set_timeout(60)
def stop_sqlnode(sqlnode,cr):
    if not sqlnode:
        return True,""
    ret,err = cr.execute(stop_sqlnode_cmd % default_mysql_sock,sqlnode,True,timeout = 5)
    if not ret:
        return False,err
    while True:
        ret,err = cr.execute("ps aux | grep mysqld",sqlnode,False,timeout = 5)
        if not err:
            return False,err
        if check_keywords(err.split("\n"),["mysqld","wxd"],["grep"]):
            continue
        break
    return True,""

@func_set_timeout(60)
def check_sqlnode(sqlnode,cr):
    ret,err = cr.execute("ps aux | grep mysqld",sqlnode,False,timeout = 5)
    if not ret:
        return False,err
    if check_keywords(err.split("\n"),["mysqld","wxd"],["grep"]):
        return True,""
    return False,"sqlnode not live"

def make_dir(host,dir,cr):
    ret,err = cr.execute("mkdir %s" % dir,host,True,dir="~",timeout = 5)
    if not ret:
        return False,err
    return True,""

"""
datanode formats:
[(host,data_dir),(host,data_dir), ... ]
"""
def start_cluster(cr,cluster,reload = False):
    manager,datanodes,sql_node,config_file = cluster["manager"],cluster["data"],cluster["sql"],cluster["config"]

    ## first we start the manager node
    ret,err = False,None
    if reload:
        ret,err = cr.execute(reload_clean + start_manage_node % (config_path,config_file) + " " + reload_flag, \
                             manager,True,timeout = 5)
    else:
        ret,err = cr.execute(start_manage_node,manager,True,timeout = 5)
    if not ret:
        print("start manager node cmd execute error: ",err)
        return False

    ## then we check, that the real cluster has really started
    try:
        ret,err = check_manager_live(manager,cr)
        if not ret:
            print("check manager liveness error!",err)
            return False
    except func_timeout.exceptions.FunctionTimedOut:
        print("check manager timeout")
        return False

    ## then we start data nodes
    success = False
    data_node_hosts = []
    for h,dir in datanodes:
        print("start datanodes: (",h,dir,")")
        data_node_hosts.append(h)
        ret,err = False,None
        if reload:
            print("make dir ",dir," @",h)
            ret,err = make_dir(h,dir,cr)
            if not ret:
                print("error when making dir @(%s,%s)" % (h,dir),err)
                success = False
            clean_cmd = reload_clean_data % dir
            print(clean_cmd)
            ret,err = cr.execute(clean_cmd,h,True,timeout = 5)
            if not ret:
                print("reload clean datanode error (%s,%s):" % (h,dir),err)
                success = False
        cmd = start_data_node
        if reload:
            cmd += (" " + reload_data_flag)
        ret,err = cr.execute(cmd,h,True,timeout = 5)
        if not ret:
            print("start datanode error (%s,%s): " % (h,dir),err)
            return False
        time.sleep(2)  ## a grace period to ensure datanodes starts *in-order*

    ## check all data nodes are alive
    ret,live_set = check_datanodes_liveness(data_node_hosts,cr,True)
    if not ret:
        print("check ndb datanodes error,live nodes: ",live_set)
        return False

    ## Finally, we start the SQL nodes
    ## First, stop any remaining sqlnode
    """
    try:
        ret,err = stop_sqlnode(sql_node,cr)
        if not ret:
            print("stop sql node error: ",err)
            return False
    except func_timeout.exceptions.FunctionTimedOut:
        print("stop sqlnode timeout")
        return False
    """
    ret,err = check_sqlnode(sql_node,cr)
    if ret:
        print("sql node already started, so we donot start it again")
        return True
    if (str(err) == "sqlnode not live"):
        pass
    else:
        print("unknown error: ",err,". please manally start the SQL node")
        return False
    print("sql node not started, try start the SQL node")
    ret,err = cr.execute(start_sql_node,sql_node,True,timeout = 5)
    if not ret:
        print("start sql node error: ",err)
        return False
    ## finally, we check that the SQLnode is ready
    ret,err = cr.execute("ps aux | grep mysqld",sql_node,False,timeout = 5)
    if not ret:
        print("check sqlnode error, but the system maybe started")
        return True
    if check_keywords(err.split("\n"),["mysqld","wxd"],["grep"]):
        return True
    else:
        print("SQL node start failure")
        return False

def stop_cluster(cr,cluster):

    manager,sql_node = cluster["manager"],cluster["sql"]
    datanodes = []
    for h,d in cluster["data"]:
        datanodes.append(h)

    ## first we stop the overall cluster
    try:
        ret,err = stop_manager(manager,cr)
        if not ret:
            print("stop cluster manager error: ",err)
            return False
    except func_timeout.exceptions.FunctionTimedOut:
        print("stop cluster timeout")

    ## then we check datanodes are all dead
    ret,err = check_datanodes_liveness(datanodes,cr,False)
    if not ret:
        print("There is still live datanodes: ",err)
        return False

    ## now we stop the sqp nodes
    try:
        ret,err = stop_sqlnode(sql_node,cr)
        if not ret:
            print("stop sql node error: ",err)
            return False
    except func_timeout.exceptions.FunctionTimedOut:
        print("stop sqlnode timeout")
        return False

    return True

class Cluster:
    def __init__(self,name,manager,datas,config,sql,load_cmd = "",general_config = {},ndb_config = {}):
        self.name = name
        self.manager = manager
        self.datanodes = datas
        self.config    = config
        self.sql       = sql
        self.load_cmd  = load_cmd
        self.general_config = general_config
        self.ndb_config      = ndb_config
        pass

    def cluster_desc_dict(self):
        cluster = {}
        cluster["manager"]  = self.manager[0]
        cluster["data"]     = self.datanodes
        cluster["config"]   = self.config
        cluster["sql"]      = self.sql
        if len(self.load_cmd) > 0:
            cluster["load_cmd"] = self.load_cmd
        return cluster

    def generate_config(self,num_of_clients = 1):
        f = open("%s.config" % self.name,"w")
        self._generate_general_config(f)
        self._generate_ndb_mgm(f)
        self._generate_datas(f)
        self._generate_sql(f)
        self._generate_clients(f,num_of_clients)
        f.close()
        call(["scp","%s.config" % self.name,"wxd@val00:/cock/mysql/config"])

    def _generate_general_config(self,f):
        f.write("[ndbd default]\n")
        for k,v in self.general_config.items():
            f.write("%s=%s\n" % (k,v))
        f.write("\n")

    def _generate_ndb_mgm(self,f):
        host,dir = self.manager
        f.write("[ndb_mgmd]\n")
        f.write("HostName=%s\n" % host)
        f.write("DataDir=%s\n"  % dir)
        f.write("NodeId=1\n\n")

    def _generate_datas(self,f):
        for i,d in enumerate(self.datanodes):
            host,dir = d
            f.write("[ndbd]\n")
            f.write("HostName=%s\n" % host)
            f.write("DataDir=%s\n"  % dir)
            f.write("NodeId=%s\n" % (i + 2))
            self._generate_ndb_config(f)
            f.write("\n")
        f.write("\n")

    def _generate_ndb_config(self,f):
        for k,v in self.ndb_config.items():
            f.write("%s=%s\n" % (k,v))

    def _generate_sql(self,f):
        f.write("[mysqld]\n")
        f.write("HostName=%s\n\n" % self.sql)

    def _generate_clients(self,f,num):
        if (num + 1 + len(self.datanodes) + 1) > 256:
            num = 256 - 2 - len(self.datanodes) - 10 ## 10 is a magic number
            print("Too many clients used. ndb support up to 256 nodes! Regulate num to ",num)

        for i in range(num):
            f.write("[mysqld]\n")

"""
NoOfReplicas=2    # Number of replicas
DataMemory=80M    # How much memory to allocate for data storage
IndexMemory=18M   # How much memory to allocate for index storage
                  # For DataMemory and IndexMemory, we have used the
                  # default values. Since the "world" database takes up
Diskless=true     # donot persist/checkpoint tables to disk
"""
global_general_config = { \
    "NoOfReplicas" : "2", \
    "DataMemory"   : "80M", \
    "IndexMemory"  : "18M", \
    "Diskless"     : "true" \
}

debug_config = { \
    "NoOfReplicas" : "1", \
    "DataMemory"   : "80M", \
    "IndexMemory"  : "18M", \
    "Diskless"     : "true" \
}


tpcc_global_config = {
    "NoOfReplicas" : "2", \
    "DataMemory"   : "10000M", \
    "Diskless"     : "true",  \
    "SharedGlobalMemory" : "1000M"
    }

"""
TransactionDeadlockDetectionTimeout=120000 #deadlock timeout
"""
global_ndb_config = {
    "TransactionDeadlockDetectionTimeout":"120000" \
}

tpcc_ndb_config= {
    "TransactionDeadlockDetectionTimeout":"120000", \
    "MaxNoOfConcurrentOperations": "200000"
    }

debug_testbed = Cluster("debug",("192.168.15.100","/home/wxd/ndb_mgm"),
                        [("192.168.15.101","/home/wxd/mysql-data")],
                        "debug.config", \
                        "192.168.15.104",    \
#                        load_cmd = './micro_loader "/home/wxd/mysql-base/mysql.sock" 10', \
                        load_cmd = '',
                        general_config = debug_config, \
                        ndb_config     = global_ndb_config)  \

micro_testbed = Cluster("micro",("192.168.15.100","/home/wxd/ndb_mgm"),
                        [("192.168.15.100","/home/wxd/mysql-data"),
                         ("192.168.15.100","/home/wxd/mysql-data1"),
                         ("192.168.15.101","/home/wxd/mysql-data"),
                         ("192.168.15.101","/home/wxd/mysql-data1"),
                         ("192.168.15.102","/home/wxd/mysql-data"),
                         ("192.168.15.102","/home/wxd/mysql-data1"),
                         ("192.168.15.103","/home/wxd/mysql-data"),
                         ("192.168.15.103","/home/wxd/mysql-data1"),
                         ("192.168.15.104","/home/wxd/mysql-data"),
                         ("192.168.15.104","/home/wxd/mysql-data1")],
                        "micro.config", \
                        "192.168.15.104",    \
                        load_cmd = './micro_loader "/home/wxd/mysql-base/mysql.sock" 10', \
                        general_config = global_general_config,\
                        ndb_config     = global_ndb_config)  \

bank_testbed = Cluster("bank",("192.168.15.100","/home/wxd/ndb_mgm"),
                        [("192.168.15.100","/home/wxd/mysql-data"),
                         ("192.168.15.100","/home/wxd/mysql-data1"),
                         ("192.168.15.101","/home/wxd/mysql-data"),
                         ("192.168.15.101","/home/wxd/mysql-data1"),
                         ("192.168.15.102","/home/wxd/mysql-data"),
                         ("192.168.15.102","/home/wxd/mysql-data1"),
                         ("192.168.15.103","/home/wxd/mysql-data"),
                         ("192.168.15.103","/home/wxd/mysql-data1"),
                         ("192.168.15.104","/home/wxd/mysql-data"),
                         ("192.168.15.104","/home/wxd/mysql-data1")],
                        "bank.config", \
                        "192.168.15.104",    \
                        load_cmd = './bank_loader "/home/wxd/mysql-base/mysql.sock" 10 1', \
                        general_config = global_general_config,\
                        ndb_config     = global_ndb_config)  \

tpcc_testbed = Cluster("tpcc",("192.168.15.100","/home/wxd/ndb_mgm"),
                        [("192.168.15.100","/home/wxd/mysql-data"),
                         ("192.168.15.100","/home/wxd/mysql-data1"),
                         ("192.168.15.101","/home/wxd/mysql-data"),
                         ("192.168.15.101","/home/wxd/mysql-data1"),
                         ("192.168.15.102","/home/wxd/mysql-data"),
                         ("192.168.15.102","/home/wxd/mysql-data1"),
                         ("192.168.15.103","/home/wxd/mysql-data"),
                         ("192.168.15.103","/home/wxd/mysql-data1"),
                         ("192.168.15.104","/home/wxd/mysql-data"),
                         ("192.168.15.104","/home/wxd/mysql-data1")],
                        "tpcc.config", \
                        "192.168.15.104",    \
                        general_config = tpcc_global_config,\
                        ndb_config     = tpcc_ndb_config)  \

micro_cluster = micro_testbed.cluster_desc_dict()

sample_cluster = { "manager": "val00", \
                   "data":  [("val01","/home/wxd/mysql-data/"),("val02","/home/wxd/mysql-data/"),\
                             ("val01","/home/wxd/mysql-data/"),("val03","/home/wxd/mysql-data1/")],\
                   "config": "config.ini",\
                   "sql":    "val04" , \
                   "load_cmd" : './micro_loader "/home/wxd/mysql-base/mysql.sock"'
}
sample_cluster = debug_testbed.cluster_desc_dict()

bank_cluster = bank_testbed.cluster_desc_dict()
tpcc_cluster = tpcc_testbed.cluster_desc_dict()

debug_cluster = { "manager": "val00", \
                   "data":  [("val01","/home/wxd/mysql-data/")],\
                   "config": "debug.config",\
                   "sql":    "val04" }

clusters = {}

def main():
    clusters["sample"] = sample_cluster
    clusters["debug"]  = debug_cluster
    clusters["bank"]   = bank_cluster
    clusters["micro"]  = micro_cluster
    clusters["tpcc"]  = tpcc_cluster

    arg_parser = argparse.ArgumentParser(
    description=''' Utilities for (re)-starting MySQL cluster. ''')
    arg_parser.add_argument(
        '-a', metavar='PROG', dest='action', default="rs",
        help='The action for the cluster, *rs* for rstart; *s* for stop, *act* for start.')
    arg_parser.add_argument(
        '-i', metavar='ID', dest='id', default=False,
        help='the cluster id to operate, current supported name: %s' % str(clusters.keys()))
    arg_parser.add_argument(
        '-r', metavar='RELOAD', dest='reload', default=False,
        help='whether to reload the cluster')

    arg_parser.add_argument(
        '-l', metavar='LOAD', dest='load', default=False,
        help='whether to load the default data')

    arg_parser.add_argument(
        '-m', metavar='MAKE', dest='make', default=False,
        help='whether to rebuild the mysql cluster')

    args = arg_parser.parse_args()
    print("action: ",args.action,";"," reload: ",args.reload, "make: ",args.make)

    cr = courier.Courier2("wxd","123")
    cr.envs.set(base_env)

    if args.action == "sql":
        sql_node = clusters[args.id]["sql"]
        ret,err = cr.execute(start_sql_node,sql_node,True,timeout = 5)
        if not ret:
            print("start sql node error: ",err)
            return False
        ret,err = check_sqlnode(sql_node,cr)
        if ret:
            print("sql node started")
            return True
        return False

    if args.action == "s" or args.action == "stop" \
       or args.action == "rs" or args.action == "restart":
        ret = stop_cluster(cr,clusters[args.id])
        print("stop cluster",ret)
        assert(ret)
        time.sleep(2) ## sleep a grace period
        return True

    if args.make == True:
        ret,err = cr.execute('make install -j48',"val04",True,dir="/cock/mysql/mysql-cluster/",timeout = 5)
        if not ret:
            print("build database error:",err)
            assert(False)

    if args.action == "rs" or args.action == "restart" \
       or args.action == "a" or args.action == "start" or args.action == "start":
        ret = start_cluster(cr,clusters[args.id],args.reload)
        assert(ret)

    if args.load:
        if not "load_cmd" in clusters[args.id]:
            print("cluster %s not provide load cmd, ignore" % args.id)
        else:
            print('cluster %s use "[%s]" to load.' % (args.id,clusters[args.id]["load_cmd"]))
            ret,err = cr.execute(clusters[args.id]["load_cmd"],clusters[args.id]["sql"],True,dir="/cock/mysql/mysql-cluster/ex",timeout = 5)
            if not ret:
                print("load database error:",ret)

if __name__ == "__main__":
    main()
 