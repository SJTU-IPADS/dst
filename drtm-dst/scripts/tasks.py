import pickle
import datetime
import time
import progressbar

#from traces import *
from tabulate import tabulate

from subprocess import call

def convert_as_sortable(t):
    return t

def check_point(f,content):
    f.write(str(content))
    f.write("\n")
    f.flush()

class AbsTask:
    def __init__(self,task_desc,e):
        self.task_desc = task_desc
        self.epoches   = e

    """
      do some preparation job before task run
      shoule return (whether success, (if not, the error message)
    """
    def pre_run(self):
        return (True,"")

    def run(self):
        return None

    def post_run(self,ret):
        return (True,0,"")

    def _check_liveness(self,host):
        return True,"Not implemented"

    def num_flying(self):
        return

    def merge(self,ress):
        return ress[0]

def default_merge_func(x,y):
    x1 = [float(i) for i in x.split(" ")]
    y1 = [float(i) for i in y.split(" ")]
    res = []
    for i in range(x1):
        res.append((x1[i] + y1[i]))
    return res

log_file = "out.log"

class TaskEngine:
    def __init__(self,tasks,gap = 1,verbose = False, time = 1, merge_func = default_merge_func,load = False):
        self.tasks = tasks
        self.gap   = gap
        self.verbose = verbose
        self.time = time  # how many time for each task
        self.merge_func = merge_func

        if self.verbose:
            print("TaskEngine will run: ",self.time, " per task.")

        self.out_log = []
        if load:
            try:
                self.out_log = pickle.load(open(log_file, "rb"))
            except:
                print("load from previsous session failes")
                self.out_log = []
        print("initial out log: ",self.out_log)

    def run_all(self):
        out_log = self.out_log
        err_log = open(str(datetime.datetime.now()) + ".err","w")
        check_pt = open("checkpoint.log","w")
        #res_log = open(str(datetime.datetime.now()) + ".res","w")

        a = progressbar.ProgressBar(maxval = len(self.tasks))
        a.start()
        for i,t in enumerate(self.tasks):
            a.update(i)
            if i < len(self.out_log):
                print("\nskip task " ,t.task_desc)
                continue
            if self.verbose:
                print("running ", t.task_desc, "%d/%d" % (i+1,len(self.tasks)) )

            ress = []
            for _ in range(self.time):
                ret,res = self._run_one_task(t,out_log,err_log)
                if ret:
                    if self.verbose:
                        print((t.task_desc,res))
                    ress.append(res)
            merged_results = t.merge(ress)
            if self.verbose:
                print(merged_results)
            ## make a checkpoint so as to tolerate failure
            out_log.append((t.task_desc,str(merged_results)))
            with open(log_file, 'wb') as handle:
                pickle.dump(out_log, handle, protocol=pickle.HIGHEST_PROTOCOL)

        a.finish()
        err_log.close()
        self.dump_log(out_log)

    def _run_one_task(self,t,out_log,err_log):
        ret,err_code = t.pre_run()
        output = "post_run: "
        if not ret:
            ## record the error in the log
            err_msg = "[%s] in pre_run: " % str(t.task_desc) + err_code
            err_log.write(err_msg + "\n")
            #res_log.write(output + " error." + "\n")
            out_log.append((t.task_desc,"error"))
            print("FATAL error in pre_run: exit",err_code)
            exit(-1)
            return
        arg = t.run()

        live_set = []
        finished_set = set()
        local_epoches = 0

        #print("wait for clients to end...")

        while True:
            local_epoches += 1
            live_set.clear()
            for h in arg:
                ret,code = t._check_liveness(h)
                if (not ret) and (code == "not live"):
                    finished_set.add(h)
                else:
                    live_set.append(h)
                    time.sleep(1)
            if len(live_set) == 0:
                break

            if local_epoches >= 120:
                print("manal kill")
                err_log.write("force stop a cluster after %d seconds" % local_epoches)
                break
        ret,res,err_msg = t.post_run(arg)
        output += err_msg
        if not ret:
            err_log.write(output + "\n")
        if self.verbose:
            print(*t.task_desc," " + output)

        #time.sleep(1)
        return ret,res

    def dump_log(self,out_log):
        print("sorting the final results:....")
        sorted_res = []
        for i in out_log:
            t,res = i
            sorted_res.append((convert_as_sortable(t) + (res,)))
        sorted(sorted_res)

        res_log = open(str(datetime.datetime.now()) + ".res","w")
        print(tabulate(sorted_res))
        res_log.write(tabulate(sorted_res))
        res_log.close()
 