#!/usr/bin/env python

import subprocess # execute commands

mac_set = ["val00","val01","val02","val03","val04","val05","val06","val07","val08","val09","val10","val11","val12","val13","val14","val15"]

def copy_file(f):
        for host in mac_set:
                #subprocess.call(["scp","-r",".","%s:~/rococo/" % host])
                subprocess.call(["rsync","-i","-rtuv",".","%s:~/rococo/" % host,"--exclude",".git","--exclude","log"])


copy_file(1)
 