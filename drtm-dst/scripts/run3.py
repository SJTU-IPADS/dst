#! /usr/bin/env python3

from tasks import AbsTask, TaskEngine
from traces  import *

import paramiko
import getpass
import os

import progressbar
import argparse
import signal
import time
import sys
import xml.etree.cElementTree as ET

from subprocess import call

FNULL = open(os.devnull, 'w')
AWS_DIR = "rocc/"
VAL_DIR = "/cock/dst/scripts/"

global_working_dir = VAL_DIR

def print_v(v,s):
    if v:
        print(s)

class Envs:
    def __init__(self,f = ""):
        self.envs = {}
        self.history = []
        try:
            self.load(f)
        except:
            pass

    def set(self,envs):
        self.envs = envs
        self.history += envs.keys()

    def load(self,f):
        self.envs = pickle.load(open(f, "rb"))

    def add(self,name,env):
        self.history.append(name)
        self.envs[name] = env

    def append(self,name,env):
        self.envs[name] = (self.envs[name] + ":" + env)

    def delete(self,name):
        self.history.remove(name)
        del self.envs[name]

    def __str__(self):
        s = ""
        for name in self.history:
            s += ("export %s=%s;" % (name,self.envs[name]))
        return s

    def store(self,f):
        with open(f, 'wb') as handle:
            pickle.dump(self.envs, handle, protocol=pickle.HIGHEST_PROTOCOL)

class ConnectProxy:
    def __init__(self,mac,user=""):
        if user == "": ## use the server user as default
            user = getpass.getuser()
        self.ssh = paramiko.SSHClient()

        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.user = user
        self.mac  = mac
        self.sftp = None

    def connect(self,pwd,timeout = 10):
        self.ssh.connect(hostname = self.mac,username = self.user, password = pwd,
                         timeout = timeout, allow_agent=False,look_for_keys=False)

    def connect_with_pkey(self,keyfile_name,timeout = 10):
        self.ssh.connect(hostname = self.mac,username = self.user,key_filename=keyfile_name,timeout = timeout)

    def execute(self,cmd,pty=False,timeout=None):
        return self.ssh.exec_command(cmd,get_pty=pty,timeout=timeout)

    def copy_file(self,f,dst_dir = ""):
        if self.sftp == None:
            self.sftp = paramiko.SFTPClient.from_transport(self.ssh.get_transport())
        self.sftp.put(f, dst_dir + "/" + f)

    def get_file(self,remote_path,f):
        if self.sftp == None:
            self.sftp = paramiko.SFTPClient.from_transport(self.ssh.get_transport())
        self.sftp.get(remote_path + "/" + f,f)

    def close(self):
        if self.sftp != None:
            self.sftp.close()
        self.ssh.close()

    def copy_dir(self, source, target,verbose = False):

        if self.sftp == None:
            self.sftp = paramiko.SFTPClient.from_transport(self.ssh.get_transport())

        if os.path.isfile(source):
            return self.copy_file(source,target)

        try:
            os.listdir(source)
        except:
            print("[%S] failed to put %s" % (self.mac,source))
            return

        self.mkdir(target,ignore_existing = True)

        for item in os.listdir(source):
            if os.path.isfile(os.path.join(source, item)):
                try:
                    self.sftp.put(os.path.join(source, item), '%s/%s' % (target, item))
                    print_v(verbose,"[%s] put %s done" % (self.mac,os.path.join(source, item)))
                except:
                    print("[%s] put %s error" % (self.mac,os.path.join(source, item)))
            else:
                self.mkdir('%s/%s' % (target, item), ignore_existing=True)
                self.copy_dir(os.path.join(source, item), '%s/%s' % (target, item))

    def mkdir(self, path, mode=511, ignore_existing=False):
        try:
            self.sftp.mkdir(path, mode)
        except IOError:
            if ignore_existing:
                pass
            else:
                raise

class Courier2:
    def __init__(self,user=getpass.getuser(),pwd="123",hosts = "hosts.xml",curdir = ".",keyfile = ""):
        self.user = user
        self.pwd = pwd
        self.keyfile = keyfile
        self.cached_host = "localhost"

        self.curdir = curdir
        self.envs   = Envs()

    def cd(self,dir):
        if os.path.isabs(dir):
            self.curdir = dir
            if self.curdir == "~":
                self.curdir = "."
        else:
            self.curdir += ("/" + dir)

    def get_file(self,host,dst_dir,f,timeout=None):
        p = ConnectProxy(host,self.user)
        try:
            if len(self.keyfile):
                p.connect_with_pkey(self.keyfile,timeout)
            else:
                p.connect(self.pwd,timeout = timeout)
        except Exception as e:
            print("[get_file] connect to %s error: " % host,e)
            p.close()
            return False,None
        try:
            p.get_file(dst_dir,f)
        except Exception as e:
            print("[get_file] get %s @%s error " % (f,host),e)
            p.close()
            return False,None
        if p:
            p.close()

        return True,None

    def copy_file(self,host,f,dst_dir = "~/",timeout = None):
        p = ConnectProxy(host,self.user)
        try:
            if len(self.keyfile):
                p.connect_with_pkey(self.keyfile,timeout)
            else:
                p.connect(self.pwd,timeout = timeout)
        except Exception as e:
            print("[copy_file] connect to %s error: " % host,e)
            p.close()
            return False,None
        try:
            p.copy_file(f,dst_dir)
        except Exception as e:
            print("[copy_file] copy %s error " % host,e)
            p.close()
            return False,None
        if p:
            p.close()

        return True,None

    def pre_execute(self,cmd,host,pty=True,dir="",timeout = None,retry_count = 3):
        if dir == "":
            dir = self.curdir

        p = ConnectProxy(host,self.user)
        try:
            if len(self.keyfile):
                p.connect_with_pkey(self.keyfile,timeout)
            else:
                p.connect(self.pwd,timeout = timeout)
        except Exception as e:
            print("[pre execute] connect to %s error: " % host,e)
            p.close()
            return None,e

        try:
            _,stdout,_ = p.execute(("cd %s" % dir) + ";" + str(self.envs) + cmd,pty,timeout)
            return p,stdout
        except Exception as e:
            print("[pre execute] execute cmd @ %s error: " % host,e)
            p.close()
            if retry_count > 0:
                if timeout:
                    timeout += 2
                return self.pre_execute(cmd,host,pty,dir,timeout,retry_count - 1)

    def execute(self,cmd,host,pty=True,dir="",timeout = None,output = True):
        ret = [True,""]
        p,stdout = self.pre_execute(cmd,host,pty,dir,timeout)
        if p and (stdout and output):
            try:
                while not stdout.channel.closed:
                    try:
                        for line in iter(lambda: stdout.readline(2048), ""):
                            if pty and (len(line) > 0): ## ignore null lines
                                print((("[%s]: " % host) + line), end="")
                            else:
                                ret[1] += (line + "\n")
                    except UnicodeDecodeError as e:
                        continue
                    except Exception as e:
                        break
            except Exception as e:
                print("[%s] execute with execption:" % host,e)
        if p:
            p.close()
        else:
            ret[0] = False
            ret[1] = stdout
        return ret[0],ret[1]

BASE_CMD = "./%s %s --bench_type %s --txn-flags 1  --verbose --config %s -p %d --id %d --tcp_port %d"
OUTPUT_CMD_LOG = " 1>~/log 2>&1 &" ## this will flush the log to a file

base_env  = {"DSTPATH":"/home/wxd/local/zeromq",
             "LD_LIBRARY_PATH":"$DSTPATH/lib:$LD_LIBRARY_PATH",
             "PATH":"/cock/dst/:$PATH"
}

whether_aws = False

hosts = []

def parse_hosts(f = "hosts.xml"):
    mac_set = []
    tree = ET.ElementTree(file=f)
    root = tree.getroot()
    assert root.tag == "hosts"

    mac_set = []
    black_list = {}

    # parse black list
    try:
        for e in root.find("black").findall("a"):
            black_list[e.text.strip()] = True
    except:
        pass

    # parse hosts
    for e in root.find("macs").findall("a"):
        server = e.text.strip()
        if not (server in black_list):
            mac_set.append(server)
    return mac_set

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
            #print("matched line: ",l)
            match.append(l)
    return len(match)

cr = Courier2("wxd","123")
cr.envs.set(base_env)

cr0 = Courier2("ubuntu",keyfile = "tp.pem")
cr0.envs.set(base_env)

tcp_port = 5555

class DSTTask(AbsTask):
    def __init__(self,task_desc,hosts):
        super().__init__(task_desc,10)
        self.prog_name,self.bench_type,self.config,num,self.args = task_desc
        self.hosts = hosts[0:num]
        assert(len(self.hosts) > 0)
        self.working_dir = global_working_dir

    def merge(self,ress):

        if len(ress) == 0:
            return ""
        if len(ress) <= 2:
            return ress[0]

        #thpt,m,n,nn,avg_lat = 0.0,0.0,0.0,0.0,0.0
        sums = [ 0.0 for i in range(5)]
        max_thpt = 0.0
        min_lat = 999999999

        for i in ress:
            res = [float(j) for j in i.split(" ")]
            try:
                max_thpt = max(max_thpt,res[0])
                min_lat  = min(min_lat,res[3])
            except:
                pass

        c = 0
        for i in ress:
            res = [float(j) for j in i.split(" ")]
            if len(res) != 5:
                continue
            t,m,n,nn,a = res
            #if (max_thpt == t) or (min_lat == a) and False:
                #print("ignore: ", res)
                #continue
            sums = [ sums[i] + res[i] for i in range(len(res))]
            c += 1
        if c == 0:
            c += 1
        #return str(thpt / c) + " " + str(avg_lat / c) + " " + "{" + str(ress) + "}"
        res = ""
        for s in [ float(i) / c for i in sums]:
            res += (str(s) + " ")
        return res

    def run(self):
        self.core_run(True)
        return self.hosts[0:1]

    def pre_run(self):
        global cr
        if whether_aws:
            cr = cr0
#        print("in pre run!")
        ## check whether there are remaining nocc instances
        ret = [True,""]
        for i,c in enumerate(self.hosts):
#            print("pre run check: ",c)
            res,err_code = self._check_liveness(c)
            if res:
                ret[0] = False
                ret[1] += ("(%d,%s) still alive " % (i,c) + err_code + "\n")
        self._clean_logs((0,self.hosts[0]))
        return ret

    def post_run(self,arg):
        if whether_aws:
            cr = cr0
        res = True
        err_msg = ""

        ret,data = self._gather_res((0,self.hosts[0]))
        if not ret:
            err_msg += res

        for i,c in enumerate(self.hosts):
            ret,err_code = self._kill_and_checkliveness(c)
            if (not ret) and (i == 0):
                res = False;
                err_msg += ("; %s kill_and_checkliveness failes: " % c + err_code)
        return res,data,err_msg

    def core_run(self,background = False):
        global cr
        if whether_aws:
#            print("core run use aws")
            cr = cr0
        global tcp_port,BASE_CMD
        if whether_aws:
            BASE_CMD = BASE_CMD + " --host aws-private.xml"
        for i,m in enumerate(self.hosts):
            if i == 0 and (not background):
                continue ## we will start this client later
            cmd = BASE_CMD % (self.prog_name,self.args,self.bench_type,self.config,len(self.hosts),i,tcp_port)
            ret,err = cr.execute(cmd + " " + OUTPUT_CMD_LOG,m,False,self.working_dir,10,output = False)

        if not background:
            ## manally start the 0 machine (which servers as the master)
            cmd = BASE_CMD % (self.prog_name,self.args,self.bench_type,self.config,len(self.hosts),0,tcp_port)
            print(cmd,self.hosts[0])
            ret,err = cr.execute(cmd,self.hosts[0],True,self.working_dir,output = True)
        #tcp_port += 1

    def _check_liveness(self,host):
        global cr
        if whether_aws:
            cr = cr0
        ret,output = cr.execute("ps aux | grep %s" % self.prog_name,host,False,self.working_dir,10)
        if not ret:
            return False,output

        #if output.find("/%s" % self.prog_name) >= 0:
        if check_keywords(output.split("\n"),["rocc",cr.user],["grep","defunct","[","python3"]) > 0:
            return True," still live " + output
        return False,"not live"

    def _kill_and_checkliveness(self,host):
        global cr
        if whether_aws:
            cr = cr0
        ret,err = cr.execute("pkill %s" % self.prog_name,host,False,self.working_dir,10)
        if not ret:
            return False,err
        if (self._check_liveness(host))[0]:
            return False,"%s still alive @%s, or error happens" % (self.prog_name,host)
        return True,""

    def _gather_res(self,c):
        global cr
        if whether_aws:
            cr = cr0

        id,host = c

        ## first we get the log file to local
        try:
            cr.get_file(host,".","log")
            mv_cmd = ["mv","log","logs/" + str(self.task_desc)]
            call(mv_cmd)
        except:
            pass

        log_file = self._log_name(id)
        p,stdout = cr.pre_execute("cat %s" % log_file,host,True,self.working_dir + "results",10)
        if not p:
            return False,stdout

        lines = []
        try:
            while not stdout.channel.closed:
                for line in iter(lambda: stdout.readline(2048), ""):
                    lines.append(line)
        except Exception as e:
            p.close()
            return False,e
        p.close()
        if len(lines) == 0:
            lines.append("0 0")
        res = lines[0].strip()
        return True,res

    def _clean_logs(self,entry):
        global cr
        if whether_aws:
            cr = cr0
        id,host = entry
        log_file = self._log_name(id)
        ret,err = cr.execute("rm %s" % log_file,host,False,self.working_dir + "results",10)
        if not ret:
            return False,err
        return True,""

    def _log_name(self,id):
        return "%d.log" % id

def get_host_config():
    if whether_aws:
        return "aws-pub.xml"
    return "hosts.xml"

def kill_rocc():
    print("kill servers",whether_aws,get_host_config())
    global cr
    if whether_aws:
        cr = cr0
    hosts = parse_hosts(get_host_config())
    a = progressbar.ProgressBar(maxval = len(hosts))
    a.start()
    for (i,c) in enumerate(hosts):
        ret,err = cr.execute("pkill %s" % "rocc",c,False,timeout = 10)
        a.update(i)
    a.finish()
    print("kill all rocc instance done")

def print_logs(args):
    hosts = parse_hosts(get_host_config())
    hosts = hosts[0:args.num]
    for h in hosts:
        ret,err = cr.execute("cat log",h,True,timeout = 10,output = True)

def check_rocc(args):
    hosts = parse_hosts()
    hosts = hosts[0:args.num]
    return

def foreground_test(args):
    global tcp_port
    task_desc = (args.prog,args.bench,args.config,args.num,args.args)
    tcp_port = args.tcp_port

    hosts = parse_hosts(args.host)

    print(hosts[0:args.num])

    t = DSTTask(task_desc,hosts)
    ret,msg = t.pre_run()
    if not ret:
        print("pre-run check failed @(%s): %s" % (str(task_desc),msg))
        return
    t.core_run(False)
    print(t.post_run(None))


def background_test(args):
    all_hosts = parse_hosts(args.host)
    tasks = []
    print("use trace: ",args.id)
    for t in generate_traces(all_traces[args.id]):
        tasks.append(DSTTask(t,all_hosts))

    e = TaskEngine(tasks,10,True,time = 3,load = args.load)
    e.run_all()

def change_permission_aws(dir,fs,h):
    for f in fs:
        cmd = "chmod +x %s" % (dir + "/" + f)
        print(cmd)
        cr0.execute(cmd,h)

## aws related functions
def sync_to_aws(args):
    ## this function should be called in the EC2 instances!
    hosts = parse_hosts("aws-private.xml")
    cr = cr0
    for i,h in enumerate(hosts):
        call(["scp","-o","StrictHostKeyChecking=no","-i","tp.pem","config.xml","%s:~/%s" % (h,AWS_DIR)])
        call(["scp","-o","StrictHostKeyChecking=no","-i","tp.pem","aws-private.xml","%s:~/%s" % (h,AWS_DIR)])
        if i == 0:
            continue
        #print("copy to: " ,h)
        #cr.copy_file(h,"config.xml",AWS_DIR)
        #cr.copy_file(h,"aws-private.xml",AWS_DIR)
        #cr.copy_file(h,"roccocc",AWS_DIR)
        #cr.copy_file(h,"roccsi",AWS_DIR)
        #cr.copy_file(h,"roccrad",AWS_DIR)
        call(["scp","-o","StrictHostKeyChecking=no","-i","tp.pem","../roccocc","%s:~/%s" % (h,AWS_DIR)])
        call(["scp","-o","StrictHostKeyChecking=no","-i","tp.pem","../roccrad","%s:~/%s" % (h,AWS_DIR)])
        call(["scp","-o","StrictHostKeyChecking=no","-i","tp.pem","../roccsi","%s:~/%s" % (h,AWS_DIR)])

        change_permission_aws(AWS_DIR,["roccrad","roccsi","roccocc"],h)

        #source,dest = (".","wxd@%s:~/rocc/scripts" % h)
        #command = ['rsync', "-rtuv",source,dest]

        #source,dest = (".","ubuntu@%s:~/rocc/scripts" % h)
        #command = ['rsync', "-rtuv", "-e", "ssh -i ../aws/tp.pem", source, dest]
        #call(command)
        ## we use courier's copy-file functionality

def main():
    arg_parser = argparse.ArgumentParser(
        description=''' The main test script for DRTM+DST''')
    arg_parser.add_argument(
        '-p', metavar='PROG', dest='prog', default="roccsi",
        help='Default client executable (default: roccsi)')
    arg_parser.add_argument(
        '-n', metavar='NUM', dest='num', default=1, type=int,
        help='Number of machines to run (default: 1)')
    arg_parser.add_argument(
        '-a', metavar='ARGS', dest='args', default="",type=str,
        help='The arguments that passed to the *prog_name*')
    arg_parser.add_argument(
        '-hh', metavar='HOST', dest='host', default="hosts.xml",type=str,
        help='The host configuration file.')
    arg_parser.add_argument(
        '-f', metavar='CONFIG', dest='config', default="config.xml",type=str,
        help='The configuration file of nocc')
    arg_parser.add_argument(
        '-b', metavar='BENCH', dest='bench', default="micro",type=str,
        help='Benchmark type')
    arg_parser.add_argument(
        '-c', metavar='ACT', dest='action', default="f",type=str,
        help='Test script type')
    arg_parser.add_argument(
        "-i",metavar="ID",dest='id',default = 'micro',type=str,
        help='The test traces to run')
    arg_parser.add_argument(
        "-t",metavar="PORT",dest='tcp_port',default = 6666,type=int,
        help='The TCP port for the test')
    arg_parser.add_argument("-w", "--aws", dest="aws",action="store_true",
                          help="start aws instances",default=False)
    arg_parser.add_argument("-l", "--load", dest="load",action="store_true",
                          help="whether to load from checkpoint",default=False)

    args = arg_parser.parse_args()
    if args.aws:
        print("Running on aws!")
        cr = Courier2("ubuntu",keyfile = "tp.pem")
        global whether_aws, global_working_dir
        whether_aws = True
        args.host = "aws-pub.xml"
        global_working_dir = AWS_DIR

    try:
        if args.action == "f":
            foreground_test(args)
        elif args.action == "b":
            try:
                background_test(args)
            except Exception as e:
                print(e)
        elif args.action == "k":
            kill_rocc()
        elif args.action == "p":
            print_logs(args)
        elif args.action == "s":
            sync_to_aws(args)
        else:
            print("Action should either be foreground (f) or background (b).")
    except:
        try:
            kill_rocc()
        except:
            pass

if __name__ == "__main__":
    main()
 