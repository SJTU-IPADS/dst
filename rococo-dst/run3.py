#! /usr/bin/env python3

import xml.etree.cElementTree as ET

import argparse
import queue
from subprocess import call

coos_per_clients = 40

default_config_file = "config/tpccd-sample.xml"
output_config_file  = "test.xml"

avaliable_node_list = ["node3","node4","node5","node6","node7","node8","node9","node10","node11","node12","node13","node14","node15","node16"]

def delete_previous_clients(root):
    clients = root.find("clients")
    root.remove(clients)

def update_clients(root,alloced):
    c = ET.Element("clients")
    c.attrib["number"] = str(len(alloced))
    for i,a in enumerate(alloced):
        node,num = a
        n = ET.Element("client")
        n.attrib["id"] = str(i)
        n.attrib["threads"] = str(num)
        n.text = node
        c.insert(1,n)
    root.insert(1,c)

def generate_clients(f,num):
    tree = ET.ElementTree(file=f)
    root = tree.getroot()

    delete_previous_clients(root)

    ## now generate
    alloced_num   = 0
    alloced       = []
    ava_queue     = queue.Queue()

    for i in avaliable_node_list:
        ava_queue.put(i)

    while alloced_num < num:
        if ava_queue.empty():
            assert(False)

        mac = ava_queue.get()

        left = num - alloced_num
        if left < coos_per_clients:
            alloced.append((mac,left))
            alloced_num += left
        else:
            alloced.append((mac,coos_per_clients))
            alloced_num += coos_per_clients

    update_clients(root,alloced)
    print(ET.dump(root))
    tree.write(output_config_file)

def main():
    print("Total ",len(avaliable_node_list)," avaliable.")
    arg_parser = argparse.ArgumentParser(
         description=''' The main test script for ROCOCO DST''')
    arg_parser.add_argument(
        '-n', metavar='C', dest='clients', default=1,type=int)
    args = arg_parser.parse_args()
    generate_clients(default_config_file,args.clients)
    call(["./sync.py"])
#    call(["./run.py","--host=config/hosts-val","-f",output_config_file,"--port=5555"])

if __name__ == "__main__":
    main()
 