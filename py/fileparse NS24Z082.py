'''
Parsing the input files
Example command (arguments need to be changed): python3 main.py -top topofile.txt -conn connections.txt -rt routingtable.txt -ft forwardingtable.txt -path pathsfile.txt -flag hop -p 1

'''
import argparse
from collections import defaultdict

def parse_topofile(topofile):
    with open(topofile, 'r') as f:
        l1 = list(map(int, f.readline().strip().split()))
        n, e = l1

        prop = [[0] * n for _ in range(n)]
        linkcap = [[0] * n for _ in range(n)]
        graph = defaultdict(dict)
        for x in range(n):
            graph[x] = list()

        print('Nodes:', n, 'Edges:', e)

        for _ in range(e):
            l2 = list(map(int, f.readline().strip().split()))
            e1, e2, pd, lc = l2
            print('Pt1:', e1, 'Pt2:', e2, 'Propogation Delay:', pd, 'Link Capacity:', lc)

            # Bidirectional link -> so creating a symmetric matrix
            graph[e1].append(e2)
            graph[e2].append(e1)
            prop[e1][e2] = pd
            prop[e2][e1] = pd
            linkcap[e1][e2] = lc
            linkcap[e2][e1] = lc
    return n, e, prop, linkcap, graph

def parse_connfile(connfile):
    with open(connfile, 'r') as f:
        l3 = list(map(int, f.readline().strip().split()))
        n = l3[0]
        print('Connections:', n)

        # conn_band = defaultdict(dict)
        conn_band = list()
        for _ in range(n):
            l4 = list(map(int, f.readline().strip().split()))
            src, dst, bmin, bavg, bmax = l4
            print('Source:', src, 'Dest:', dst, 'Min band req.:', bmin, 'Avg band req.:', bavg, 'Max band req.:', bmax)

            # conn_band[src][dst] = [bmin, bavg, bmax]
            conn_band.append([src, dst, bmin, bavg, bmax])

    return conn_band, n
       

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('-top', type=str, required=True, help='Path to topology file [Input]')
    parser.add_argument('-conn', type=str, required=True, help='Path to connections file [Input]')
    parser.add_argument('-rt', type=str, required=True, help='Path to routing table file [Output]')
    parser.add_argument('-ft', type=str, required=True, help='Path to forwarding table file')
    parser.add_argument('-path', type=str, required=True, help='Path to paths file')
    parser.add_argument('-flag', type=str, required=True, choices = ['hop', 'dist'], help='Specify \"hop\"/\"dist\" within quotes')
    parser.add_argument('-p', type=int, required=True, choices=[0, 1], help='Specify 0/1: 0 for Optimistic approach and 1 for pessimistic approach')

    args = parser.parse_args()
    # print(args.top)

    n, e, prop, linkcap, graph = parse_topofile(args.top)
    conn_band, no_conn = parse_connfile(args.conn)

    return n, e, prop, linkcap, graph, conn_band, no_conn, args.rt, args.ft, args.path, args.flag, args.p  # returns to main