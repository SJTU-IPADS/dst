import itertools

traces = { \
           "progs":["tester","tester0"], \
           "clients":[1,10,20,30,40,50,60,70,80,90,100],\
           #"clients": [100],\
           "args": ["bank 0", "bank 3","bank 2"], \
           #"args": ["bank 3"], \
           "epoches": 70 \
    }

"""
## used to test contention ratio
micro_traces = { \
           "progs":["tester"], \
           #"clients":[1,10,20,30,40,50,60,70,80,90,100],\
           "clients": [100],\
           "args": ["--contention_ratio 1 --read_rounds 10",\
                    "--contention_ratio 2 --read_rounds 10",\
                    "--contention_ratio 4 --read_rounds 10",\
                    "--contention_ratio 6 --read_rounds 10",\
                    "--contention_ratio 8 --read_rounds 10",\
                    "--contention_ratio 10 --read_rounds 10",\
                    "--contention_ratio 12 --read_rounds 10",\
                    "--contention_ratio 14 --read_rounds 10",\
                    "--contention_ratio 16 --read_rounds 10",\
                    "--contention_ratio 18 --read_rounds 10",\
                    "--contention_ratio 20 --read_rounds 10",\
                    "--contention_ratio 22 --read_rounds 10",\
                    "--contention_ratio 25 --read_rounds 10",\
                    "--contention_ratio 50 --read_rounds 10",\
                    "--contention_ratio 75 --read_rounds 10",\
                    "--contention_ratio 100 --read_rounds 10",\
           ],\
           "lock_type": [0,2,3],\
           "type"     : ["micro1"], \
           "epoches": 90 \
    }
"""

## used to test read rounds
micro_traces = { \
           "progs":["tester"], \
           #"clients":[1,10,20,30,40,50,60,70,80,90,100],\
           "clients": [100],\
           "args": ["--contention_ratio 10 --read_rounds 1",\
                    "--contention_ratio 10 --read_rounds 2",\
                    "--contention_ratio 10 --read_rounds 4",\
                    "--contention_ratio 10 --read_rounds 6",\
                    "--contention_ratio 10 --read_rounds 8",\
                    "--contention_ratio 10 --read_rounds 10",\
                    "--contention_ratio 10 --read_rounds 12",\
                    "--contention_ratio 10 --read_rounds 14",\
                    "--contention_ratio 10 --read_rounds 16",\
           ],\
           "lock_type": [0,2,3],\
           "type"     : ["micro1"], \
           "epoches": 90 \
    }

bank_traces = {
    "progs":["tester"], \
    #"clients": [1,10,20,30,40],\
    #"clients" : [10,20,30,40,50,80,120,160,200,240],\
    "clients" : [2,3,4,5,6,7,8,9,50,100,150,200,250,300,350,400,450,500,528],\
    #"clients" : [300],\
    "args": [""] ,\
    "lock_type": [0,2,3],\
    "type"     : ["bank"], \
    "epoches": 70 \

    }

tpcc_traces = {
    "progs":["tester"], \
    #"clients": [1,2,3,4,5,6,7,8,9,10],\
    #"clients" : [10,20,30,40,50,80,120,160,200,240],\
    #"clients" : [1,50,100,150,200,250,300,350,400,450,500,528],\
    "clients" : [300],\
    "args": [""] ,\
    "lock_type": [2, 3, 0],\
    "type"     : ["tpcc"], \
    "epoches": 16 \

    }

test_traces = { \
           "progs":["tester"], \
           "clients":[4], \
           "args": ["micro1 0"], \
           "epoches": 10 \
    }

all_traces = {
    "micro" : micro_traces,
    "bank"  : bank_traces,
    "tpcc"  : tpcc_traces
}

def generate_traces(trace = traces):
    res = ([],trace["epoches"])
    for i in itertools.product(trace["progs"],trace["clients"],trace["args"],trace["lock_type"],trace["type"]):
        res[0].append(i)
    return res

def convert_as_sortable(t):
    prog,clients,args,lock_type,type = t
    args = args.split()
    return (prog,type,clients,lock_type) + tuple(args)

def trace_name(t):
    try:
        res = ""
        for i in t:
            res += (str(i) + "_")
        return res
    except:
        return "tace_name_error_" + str(t)
 