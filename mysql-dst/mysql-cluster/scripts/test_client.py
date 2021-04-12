from courier import *

import cluster_manager
import threading
import paramiko
import os

"""       prog_name id   socket  ndb-manager"""
base_cmd  = './%s       %d  "%s"    "%s"'
base_cmd_flag = "./%s --id %d --manager %s --epoch %d "
kill_cmd  = "pkill %s"
clear_cmd = "rm %s" ## remove the log file
output_log_cmd = " 1>%s 2>&1 &" ## this will flush the log to a file

class BatchClients :
    """ a python controler to start multiple clients, defined in tester """

    """
    clients : tuples which describe the clients, in the following format: host0,host1, ...
    """
    def __init__(self,clients,prog_name,args,my_courier,sock="/home/wxd/mysql-base/mysql.sock",ndb_host="val00",curdir = ""):

        self.courier = my_courier
        #self.courier = courier.Courier(user,pwd)
        #self.courier.envs.set(mysql_helper.base_env)

        self.clients = {}
        for i,c in enumerate(clients):
            self.clients[i] = Client(prog_name,args,i,sock,ndb_host,c,self.courier,curdir)

    """ start all clients """
    def start(self):
        for _,c in self.clients.items():
            c.start()

    """ show the outputs from all the clients """
    def show(self,epoch = 0):
        for i,c in self.clients.items():
            print("[%3d] client #%5d:  %s" % (epoch,i,c.res))
        print()

    def kill_all(self):
        for _,c in self.clients.items():
            c.kill()
        print("kill all clients done")

    def total_thpt(self):
        sum = 0.0
        all_results = []
        for _,c in self.clients.items():
            res = c.report_thpt()
            all_results.append(res)
            sum += res
        print(all_results)
        return sum

    def print_output(self,i):
        c = self.clients[i]
        for line in c.buf:
            print(line.strip())

    def close(self):
        for _,c in self.clients.items():
            c.close()

err_words = ["error","Error","Aborted"]

class Client:
    """ a python controler to start client, defined in tester """

    def __init__(self,prog_name,args,id,sock,ndb_host, host,my_courier,curdir = ""):
        ## config parameters
        self.id = id
        self.prog_name = prog_name
        self.sock      = sock
        self.ndb       = ndb_host
        self.args      = args

        ## just parse a little extra stuff from the args
        self.lock_mode = int(args.split()[1])

        ## execute parameters
        self.host = host
        self.curdir    = curdir

        self.res = ""

        ## init the courier to execute
        self.courier = my_courier

        self.buf = []
        self.error = False

    def start(self):
        ## first we clear the remote log file
        cmd = clear_cmd % (self.__log_name())
        self.courier.execute(cmd,self.host,True,self.curdir)

        ## then we execute the prog_name at the remote server
        cmd = (base_cmd % (self.prog_name,self.id,self.sock,self.ndb)) + " " + self.args + " " #+ (output_log_cmd % self.__output_name())

        p,stdout = self.courier.pre_execute(cmd,self.host,True,self.curdir,10)

        ## finally, we spawn a thread to collect related results
        thread = threading.Thread(target=self.__feed_output, args=(p,stdout))
        thread.daemon = True
        thread.start()

    def kill(self):
        cmd = kill_cmd % self.prog_name
        self.courier.execute(cmd,self.host,False,self.curdir)

    def close(self):
        try:
            os.remove("logs/%d.log" % self.id)
        except:
            pass
        f = open("logs/%d.log" % self.id,"w")
        for line in self.buf:
            f.write(line)
        f.close()

    def log(self,*args):
        header = "[client #%3d] " % self.id
        if len(args) == 0:
            print(header)
        else:
            print(header,*args)

    def report_thpt(self,f = lambda line : float(line)):
        log_file = self.__log_name()

        p,stdout = self.courier.pre_execute("cat %s" % log_file,self.host,True,self.curdir,10)
        if not p:
            return

        sums = []
        try:
            while not stdout.channel.closed:
                for line in iter(lambda: stdout.readline(2048), ""):
                    sums.append(f(line))
        except:
            pass
        p.close()

        self._dump_res(sums)
        if len(sums) > 30:
            sums = sums[15:-15]

        return sum(sums) / float(len(sums))

    def _dump_res(self,res):
        f = open("logs_%d/%s.log" % (self.lock_mode,str(self.task_desc)),"w")
        for l in res:
            f.write(str(l) + "\n")
        f.close()

    """ Feed the client's output back """
    def __feed_output(self,p,stdout):
        if (not p) or (not stdout):
            self.log("connect error")
            return

        try:
            while not stdout.channel.closed:
                for line in iter(lambda: stdout.readline(2048), ""):
                    self.res = line.strip("\n")
                    if self.__has_error(line):
                        self.log("Error!",line)
                        self.error = True
                    self.buf.append(line)
        except:
            self.log("feed output error!")
        p.close()
        self.log("exit")


    def __has_error(self,s):
        for w in err_words:
            if s.find(w) >= 0:
                return True
        return False

    """ parse the output into a more readable format """
    def __parse_res(self,res):
        ## donot ingore the errors
        if self.__has_error(res):
            return res
        ret = ""
        try:
            ret = (res.strip().split("]"))[1]
        except:
            pass
        return ret

    def __log_name(self):
        return "%d.log" % self.id

    def __output_name(self):
        return "%d.output" % self.id

"""
Some simple utilities for count results
"""
def count_single(f):
    res = 0.0
    f = open(f,"r")
    res = [float(line) for line in f]
    return average(res)

def average(l):
    return float(sum(l)) / float(len(l))

def total_thpt(dir_name,c_num):
    return [count_single(dir_name + "/" + "%s.log" % i) for i in range(c_num) ]

def count_total_thpt(dir_name,c_num):
    res = total_thpt(dir_name,c_num)
    return average(res)
 