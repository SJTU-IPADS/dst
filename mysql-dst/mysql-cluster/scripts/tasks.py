import datetime
import time
import progressbar

from traces import *
from tabulate import tabulate

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
        return 0

class TaskEngine:
    def __init__(self,tasks,gap = 1,verbose = False):
        self.tasks = tasks
        self.gap   = gap
        self.verbose = verbose

    def run_all(self):
        out_log = []
        err_log = open(str(datetime.datetime.now()) + ".err","w")
        #res_log = open(str(datetime.datetime.now()) + ".res","w")

        for i,t in enumerate(self.tasks):
            if self.verbose:
                print("running ", t.task_desc, "%d/%d" % (i+1,len(self.tasks)) )
            ret,err_code = t.pre_run()
            output = "post_run: "
            if not ret:
                ## record the error in the log
                err_msg = "[%s] in pre_run: " % str(t.task_desc) + err_code
                err_log.write(err_msg + "\n")
                #res_log.write(output + " error." + "\n")
                out_log.append((t.task_desc,"error"))
                #if self.verbose:
                    #print(err_msg)
                continue
            arg = t.run()

            live_set = []
            finished_set = set()
            epoches = 10

            print("wait for clients to end...")
            print(arg)
            # a = progressbar.ProgressBar(max_value = len(arg))

            while True:
                live_set.clear()
                for h in arg:
                    ret,code = t._check_liveness(h)
                    if (not ret) and (code == "not live"):
                        finished_set.add(h)
                    else:
                        live_set.append(h)
                    time.sleep(0.5)
                # a.update(len(finished_set))
                if len(live_set) == 0:
                    break
                epoches += 1
                if epoches >= len(arg) * epoches * 73:
                    err_log.write("force stop a cluster after %d seconds" % epoches)
                    break
                else:
                    time.sleep(len(arg) / 2.0)
            # a.finish()
            ret,res,err_msg = t.post_run(arg)
            output += err_msg
            if not ret:
                err_log.write(output + "\n")
            output += (" " + str(res))
            out_log.append((t.task_desc,output))

            if self.verbose:
                print(*t.task_desc," " + output)

            time.sleep(self.gap)

        err_log.close()
        self.dump_log(out_log)

    def dump_log(self,out_log):
        print("sorting the final results:....")
        sorted_res = []
        for i in out_log:
            t,res = i
            sorted_res.append((convert_as_sortable(t) + (res,)))
        sorted(sorted_res)

        res_log = open(str(datetime.datetime.now()) + ".res","w")
        for x in sorted_res:
            res_log.write(str(x))
        print(tabulate(sorted_res))
        res_log.close()
        pass
 