#!/usr/bin/python3

import xml.etree.ElementTree as et
from optparse import OptionParser

client_nodes = [
        'node9', 'node10', 'node11', 'node12',
        'node13', 'node14', 'node15', 'node16',
        ];

parser = OptionParser()
parser.add_option("-n", "--clients_per_server", dest="clients_per_server",
        help="the number of clients on each client machine",
        default=1, metavar="CPS")
(options, args) = parser.parse_args()

cps = int(options.clients_per_server)

conf = et.parse('config/tpcc/pdi_deptran_tmpl.xml')
clients = conf.find('clients')

# generate client ids for each node
for i, node in enumerate(client_nodes):
    if cps == 1:
        cli_se = et.SubElement(clients, 'client',
                {"id":"{:02}".format(i),"threads":"1"})
    else:
        cli_se = et.SubElement(clients, 'client',
                {"id":"{:02}-{:02}".format(cps * i, cps * (i + 1) - 1),"threads":"1"})
    cli_se.text = node

# update the total number of clients
clients.set("number", "{}".format(cps * len(client_nodes)))
conf.write('new.xml')
 