#!/usr/bin/env python

from test_client import *
from courier import *
import cluster_manager

import progressbar
import argparse
import time
import sys

from tasks import AbsTask, TaskEngine
from traces import *

"""
val00 - val04 serves as the ndb storage servers
"""
default_hosts   = ["val06","val07","val08","val09","val05","val10","val11","val12","val13","val14","val15","val16","val17"]
client_per_host = 1 ## each machine has 24 cores
threads_per_client = 48

verbose = False

## setting: in callas, it uses 20 (clients) : 1 (partition) setting.

def max_clients(host):
    return len(host) * threads_per_client

"""
 Generate the clients. It will generate in a round-robin way.
 For example, if we want to generate 6 clients given total hosts ["val03","val04","val05"].
 It will generate ["val03","val04","val05","val03","val04","val05"].
"""
def generate_client_hosts(num,hosts):
    assert (num <= max_clients(hosts))
    res = []
    idx = 0
    alocated = 0
    while (alocated + threads_per_client) <= num:
        res.append((hosts[idx],threads_per_client))
        idx = (idx + 1) % len(hosts)
        alocated += threads_per_client
    if alocated < num:
        assert alocated + threads_per_client > num
        res.append((hosts[idx],num - alocated))
    return res

def str2bool(v):
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

"""
 Run the foreground test, which print the results on the screen
"""
def foreground_test(prog_name,args,num_clients,epoches = 20):

    client_hosts = generate_client_hosts(num_clients,default_hosts)

    print("test %s %s, using " % (prog_name,args),client_hosts, " to host clients")
    cr = courier.Courier2("wxd","123")
    cr.envs.set(cluster_manager.base_env)

    args = args + " "  + str(epoches)
    clients = BatchClients(client_hosts,"./" + prog_name, args, cr, \
                           "/home/wxd/mysql-base/mysql.sock","val00","/cock/mysql/mysql-cluster/ex")

    clients.start()

    for i in range(epoches + 3):
        #clients.show(i)
        time.sleep(1)

    clients.kill_all()
    print("%d client %s has total throughput: [%f] reqs / sec." % (num_clients,prog_name,clients.total_thpt()))
    return clients

"""
 Code for background tests
"""
cr = courier.Courier2("wxd","123")

"""
remove duplicate clients to make display more convenient
"""
def _compress_clients(cs):
    d = {}
    for c in cs:
        if c in d:
            d[c] += 1
        else:
            d[c] = 1
    return d

class NdbTask(AbsTask):
    def __init__(self,task_desc,e):
        super().__init__(task_desc,e)
        ## parse names from trace
        self.prog_name, client_num, self.args,self.lock_type,self.type = task_desc
        self.clients = generate_client_hosts(client_num,default_hosts)
        self.working_dir = "/cock/mysql/mysql-cluster/ex"

    def run(self):
        s = set()
        for i,c in enumerate(self.clients):
            c,t = c
            cmd = (base_cmd_flag % (self.prog_name,i,"val00",int(self.epoches))) + \
                self.args + " " + "--type " + self.type + " --lock_mode " + str(self.lock_type) + " --threads %d" % t
            if verbose and i == 0:
                print(cmd)
            ret,err = cr.execute(cmd,c,False,self.working_dir ,self.epoches,output = False)
            s.add(c)

        gap_time = int(float(len(self.clients)) / 20) + 2
        if verbose:
            print("[%s] run @" % str(self.task_desc),_compress_clients(self.clients), "; with gap time ",gap_time)

        return s

    def pre_run(self):
        ret = [True,""]
        for i,c in enumerate(self.clients):
            c,_ = c
            res,err_code = self._clean_logs((i,c))
            if not res:
                ret[0] = False
                ret[1] += err_code
        return ret

    def post_run(self,arg):
        thpt = 0.0
        err_msg = ""
        res = True

        if verbose:
            print("[%s]: gathering results" % str(self.task_desc))
        # a = progressbar.ProgressBar(max_value = len(self.clients))

        for i,c in enumerate(self.clients):
            c,t = c
            if (self._check_liveness(c))[0]:
                time.sleep(10)
                #res,err_msg = (False,("error before gather res in (%d,%s), host still alive or there is fatal error " % (i,c)))
            res,output = self._gather_res((i,c))
            if not res:
                res,err_msg = (False,("error gather res in (%d,%s) " % (i,c)) + str(output).strip())
            else:
                thpt += output
            # if verbose:
                # a.update(i + 1)
        ## end iterating all clients
        # a.finish()

        for c in arg:
            res,err_code = self._kill_and_checkliveness(c)
            if not res:
                res = False;
                err_msg += ("; %s kill_and_checkliveness failes: " % c + err_code)
        return res,thpt,err_msg

    def _kill_and_checkliveness(self,host):
        ret,err = cr.execute("pkill %s" % self.prog_name,host,False,self.working_dir,10)
        if not ret:
            return False,err
        #time.sleep(1)
        if (self._check_liveness(host))[0]:
            return False,"%s still alive @%s, or error happens" % (self.prog_name,host)
        return True,""

    def _gather_res(self,c):
        id,host = c
        log_file = self._log_name(id)
        p,stdout = cr.pre_execute("cat %s" % log_file,host,True,self.working_dir,10)
        if not p:
            return False,stdout

        sums = []
        try:
            while not stdout.channel.closed:
                for line in iter(lambda: stdout.readline(2048), ""):
                    sums.append(float(line))
        except Exception as e:
            p.close()
            return False,e
        p.close()

        self._dump_res(sums,id)
        if len(sums) > 15:
            sums = sums[3:-5]
        else:
            if verbose:
                print("results not enough: ", len(sums), self.task_desc)
        output = 0.0
        if len(sums):
            output = sum(sums) / float(len(sums))

        return True,output

    def _dump_res(self,res,id):
        #f = open("logs%d/%d.log" % (self.lock_mode,id),"w")
        #for l in res:
            #f.write(str(l) + "\n")
        #f.close()
        pass

    def _clean_logs(self,entry):
        id,host = entry
        log_file = self._log_name(id)
        ret,err = cr.execute("rm %s" % log_file,host,False,self.working_dir,10)
        if not ret:
            return False,err
        return True,""

    def _check_liveness(self,host):
        ret,output = cr.execute("ps aux | grep %s" % self.prog_name,host,False,self.working_dir,10)
        if not ret:
            return False,output

        #if output.find("/%s" % self.prog_name) >= 0:
        if cluster_manager.check_keywords(output.split("\n"),[self.prog_name,"wxd"],["grep"]) > 0:
            return True," still live"
        return False,"not live"

    def num_flying(self):
        return len(self.clients)

    def _log_name(self,id):
        return "%d.log" % id


def main():
    arg_parser = argparse.ArgumentParser(
        description=''' The main test program for MySQL cluster. ''')
    arg_parser.add_argument(
        '-p', metavar='PROG', dest='prog', default="tester",
        help='Default client executable (default: tester)')
    arg_parser.add_argument(
        '-n', metavar='NUM', dest='num', default=2, type=int,
        help='Number of clients to run (default: 2)')
    arg_parser.add_argument(
        '-a', metavar='ARGS', dest='args', default="",type=str,
        help='The arguments that passed to the *prog_name*')
    arg_parser.add_argument(
        '-e', metavar='EPOCH', dest='epoch', default=20, type=int,
        help='Number of time clients to run (default: 20)')
    arg_parser.add_argument(
        "-c",metavar='ACT',dest='action',default="f",
        help='whether to run foreground or background test')
    arg_parser.add_argument("-v", type=str2bool, nargs='?',
        const=True, default=True,dest='verbose',
        help='whether to print detailed information')
    arg_parser.add_argument(
        '-i', metavar='ID', dest='id', default=False,
        help='the traces to use %s' % str(all_traces))

    args = arg_parser.parse_args()
    global verbose
    verbose = args.verbose

    if args.action == "f":
        ## foreground test
        clients = foreground_test(args.prog,args.args,args.num)
        clients.close()
    else:
        ## background test, in batch

        ## prepare test environments
        cr.envs.set(cluster_manager.base_env)

        ## generate test traces
        tasks = []
        #tt = test_traces
        tt = all_traces[args.id]
        for t in generate_traces(tt)[0]:
            tasks.append(NdbTask(t,generate_traces(tt)[1]))

        e = TaskEngine(tasks,3,args.verbose)
        e.run_all()

def kill_clients(name,hosts = default_hosts):
    for i in hosts:
        ret,err = cr.execute("pkill %s" % name,i,True,"",10)

if __name__ == "__main__":
    main()
 