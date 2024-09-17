import fileparse
import routing_hop
import routing_delay
import connections
import shutil
    
# PARSING THE FILE
n, e, prop, linkcap, graph, conn_band, no_conn, rt, ft, pathforpaths, flag, p = fileparse.parse_arguments() #conn_band is a list of requested connections

# ROUTING
print('HOP RESULTS:\n')
routing_hop.compute_route(n, prop, graph)
print('DELAY RESULTS:\n')
routing_delay.compute_route(n, prop, graph)

# FORWARDING AND PATHS COMPUTATION
print(conn_band)
print(linkcap)
print('\n\n')
no_active_conn = connections.check_connection(n, e, conn_band, linkcap, flag, p, ft, pathforpaths)

#BLOCKING PROBABILITY
count = 0
with open(pathforpaths, 'r') as f:
    line = f.readline().strip()
    while line:
        count = count + 1
        line = f.readline().strip()

# blocking_prob = (no_conn - no_active_conn)/no_conn
blocking_prob = (no_conn - count)/no_conn
print('\nNumber of connections requested:', no_conn, 'No of connections serviced:', count, 'Blocking Probability: ', f'{blocking_prob:.4f}')

# ROUTING FILE DUMP
with open('routingtablefile_new.txt', 'w') as f:
    # f.write('Routing table with shortest path for hops\n')
    with open('./routingfiles/routingtablefile.txt', 'r') as f1:
        line = f1.readline().strip()
        while line:
            f.write(f'{line}\n')
            line = f1.readline().strip()

    # f.write('\nRouting table with second shortest path for hops\n')
    with open('./routingfiles/routingtablefile_second.txt', 'r') as f1:
        line = f1.readline().strip()
        while line:
            f.write(f'{line}\n')
            line = f1.readline().strip()

    # f.write('\nRouting table with shortest path for delay/distance\n')
    with open('./routingfiles/routingtablefiledelay.txt', 'r') as f1:
        line = f1.readline().strip()
        while line:
            f.write(f'{line}\n')
            line = f1.readline().strip()

    # f.write('\nRouting table with second shortest path for delay/distance\n')
    with open('./routingfiles/routingtablefiledelay_second.txt', 'r') as f1:
        line = f1.readline().strip()
        while line:
            f.write(f'{line}\n')
            line = f1.readline().strip()

shutil.move('routingtablefile_new.txt', rt)
    
# rt - './routingfile/routingtablefile.txt'
# ft - './forwardingfile/forwardingfile.txt'
# paths - './pathsfile/pathsfile.txt'

