'''
Program to compute forwarding table entries
'''
import ast
from collections import defaultdict

conn_ids = 0 # counter to keep track if number of connections

def check_bandwidth_p(total_link_cap, connection_band_max):
    if connection_band_max <= total_link_cap: # total_link_cap will be subtracted with max_bandwidth after every connection request
        print('\n', connection_band_max, '<=', total_link_cap)
        return True
    else: 
        print('No')
        return False

def check_bandwidth_o(total_link_cap, connection_band):  
    band_eq = min(connection_band[2], connection_band[1] + 0.35 * (connection_band[2] - connection_band[0]))
    if band_eq <= total_link_cap: # total_link_cap will be subtracted with eq_bandwidth after every connection request
        print('\n', band_eq, '<=', total_link_cap)
        return True, band_eq # since the equivalent bandwidth is calculated here, returning it to avoid calculating again
    else:
        print('No')
        return False, -1

def setup_connection(path, linkcap, band_req, vcid, cost_matrix): # p=1 pessimistic, p=0 optimistic, cost_matrix can be shortest or second_shortest, band_req can be optimistic or pessimistic
    # subtract the bandwidth
    # generate VCIDs
    # generate conn id

    conn_vcids = defaultdict(dict) # for 1 connection
    for idx in range(len(path)-1):
        e1 = path[idx]
        e2 = path[idx+1]
        linkcap[e1][e2] -= band_req
        linkcap[e2][e1] -= band_req
        conn_vcids[e1][e2] = vcid[e1][e2] + 1 # starting from 1
        vcid[e1][e2] = vcid[e1][e2] + 1
        vcid[e2][e1] = vcid[e2][e1] + 1

    global conn_ids
    # conn_id = conn_ids + 1
    conn_id = conn_ids
    conn_ids = conn_ids + 1

    return conn_id, linkcap, conn_vcids, cost_matrix[path[0]][path[len(path)-1]] 

def write_forwardingfile(path, conn_vcids, ft):
    with open(ft, 'a') as f:
        for idx in range(len(path)):
            if idx==0 or idx==(len(path)-1):
                continue
            router_id = path[idx]
            incoming_node = path[idx-1]
            outgoing_node = path[idx+1]
            incoming_node_vcid = conn_vcids[path[idx-1]][path[idx]]
            outgoing_node_vcid = conn_vcids[path[idx]][path[idx+1]]
            f.write(f'{router_id} {incoming_node} {incoming_node_vcid} {outgoing_node} {outgoing_node_vcid}\n')

def write_pathsfile(path, conn_vcids, conn_id, src, dst, conn_cost, pathforpaths):
    with open(pathforpaths, 'a') as f:
        conn_vcid_list = []
        for outer_key, inner_dict in conn_vcids.items():
            for inner_key, value in inner_dict.items():
                conn_vcid_list.append(value)
        f.write(f'{conn_id} {src} {dst} {path} {conn_vcid_list} {conn_cost}\n')

def check_connection(n, e, conn_req, linkcap, flag, p, ft, pathforpaths):
    global conn_ids
    shorest_routes = [[0]*n for _ in range(n)]
    second_shortest_routes = [[0]*n for _ in range(n)]
    shortest_cost = [[0]*n for _ in range(n)]
    second_shortest_cost = [[0]*n for _ in range(n)]

    vcid = [[0]*n for _ in range(n)]
    if flag == 'hop':
        file1 = './routingfiles/routingtablefile.txt'
        file2 = './routingfiles/routingtablefile_second.txt'
    elif flag == 'dist':
        file1 = './routingfiles/routingtablefiledelay.txt'
        file2 = './routingfiles/routingtablefiledelay_second.txt'

    print('Using..', file1, 'and', file2, '\n')

    print('SHORTEST ROUTES:') #Reading from file
    with open(file1, 'r') as f1:
        for line in f1:
            line = line.strip()  # Remove any leading/trailing whitespace, including '\n'
            if line:  # Only process non-empty lines
                # print(line)
                first_space_index = line.find(' ')
                dest = int(line[:first_space_index])

                start_bracket_index = line.find('[')
                end_bracket_index = line.find(']') + 1
                list_str = line[start_bracket_index:end_bracket_index]
                list_data = ast.literal_eval(list_str)

                remaining_parts = line[end_bracket_index:].strip().split()
                path_delay = int(remaining_parts[0])
                path_cost = int(remaining_parts[1])

                print(dest, list_data, path_delay, path_cost)

                shorest_routes[list_data[0]][list_data[len(list_data)-1]] = list_data
                shortest_cost[list_data[0]][list_data[len(list_data)-1]] = path_cost
            '''
            Comment print(line), parse lines, create routing array where r[i][j] = path

            '''
    for i in range(n):
        for j in range(n):
            print(shorest_routes[i][j], end = ' ')
        print()

    print('SECOND SHORTEST ROUTES:') # Reading from file
    with open(file2, 'r') as f2:
        for line in f2:
            line = line.strip()  # Remove any leading/trailing whitespace, including '\n'
            if line:  # Only process non-empty lines
                # print(line) 
                first_space_index = line.find(' ')
                dest = int(line[:first_space_index])

                start_bracket_index = line.find('[')
                end_bracket_index = line.find(']') + 1
                list_str = line[start_bracket_index:end_bracket_index]
                list_data = ast.literal_eval(list_str)

                remaining_parts = line[end_bracket_index:].strip().split()
                path_delay = int(remaining_parts[0])
                path_cost = int(remaining_parts[1])

                print(dest, list_data, path_delay, path_cost)

                second_shortest_routes[list_data[0]][list_data[len(list_data)-1]] = list_data
                second_shortest_cost[list_data[0]][list_data[len(list_data)-1]] = path_cost

    for i in range(n):
        for j in range(n):
            print(second_shortest_routes[i][j], end = ' ')
        print()
                     
    for eve_conn in conn_req: # conn_req is a list of connections passed by main.py
        src = eve_conn[0]
        dst = eve_conn[1]
        print(n, src, dest, len(shorest_routes), len(shorest_routes[0]), eve_conn)
        path_1 = shorest_routes[src][dst]
        path_2 = second_shortest_routes[src][dst]
        conn_band = [eve_conn[2], eve_conn[3], eve_conn[4]] #bmin, bavg, bamx -> will be used when p==0

        can_admit_short = True
        can_admit_secshort = True

        print('----------------------------------------')
        print('\nChecking for path_1:', path_1)
        if p==1: #pessimistic
            print('PESSIMISTIC')
            for idx in range(len(path_1) -1): # check for path_1 -> shortest path
                print('\nFor', path_1[idx], path_1[idx+1], 'Total bandwidth:', linkcap[path_1[idx]][path_1[idx+1]], 'Required bandwidth:', eve_conn[4])

                tmp = check_bandwidth_p(linkcap[path_1[idx]][path_1[idx+1]], eve_conn[4]) # eve_conn[4] is the max bandwidth
                can_admit_short = can_admit_short and tmp

            if can_admit_short == True: # if can_admit: --> shortest path is OK
                conn_id, linkcap, conn_vcids, conn_cost = setup_connection(path_1, linkcap, eve_conn[4], vcid, shortest_cost)
                write_forwardingfile(path_1, conn_vcids, ft)
                write_pathsfile(path_1, conn_vcids, conn_id, src, dst, conn_cost, pathforpaths)

                print('connection id:', conn_id, 'vcids:', conn_vcids, 'path_1:', path_1, 'cost:', conn_cost)
                print('updated linkcap:')
                for i in range(n):
                    for j in range(n):
                        print(linkcap[i][j], end = ' ')
                    print()
            else:
                print('\nChecking for path_2:', path_2)

                for idx in range(len(path_2) -1): # check for path_2 -> second shortest path
                    print('\nFor', path_2[idx], path_2[idx+1], 'Total bandwidth:', linkcap[path_2[idx]][path_2[idx+1]], 'Required bandwidth:', eve_conn[4])

                    tmp = check_bandwidth_p(linkcap[path_2[idx]][path_2[idx+1]], eve_conn[4]) # eve_conn[4] is the max bandwidth
                    can_admit_secshort = can_admit_secshort and tmp
                
                if can_admit_secshort == True: #second shortest OK
                    conn_id, linkcap, conn_vcids, conn_cost = setup_connection(path_2, linkcap, eve_conn[4], vcid, second_shortest_cost)
                    write_forwardingfile(path_2, conn_vcids, ft)
                    write_pathsfile(path_2, conn_vcids, conn_id, src, dst, conn_cost, pathforpaths)                    
                    
                    print('connection id:', conn_id, 'vcids:', conn_vcids, 'path_2:', path_2, 'cost:', conn_cost)
                    print('updated link:')
                    for i in range(n):
                        for j in range(n):
                            print(linkcap[i][j], end=' ')
                        print()
                else: # connection rejected
                    conn_ids = conn_ids + 1
                
        
        elif p==0: #optimistic -- same as above, chnage is only in check_bandwidth
            print('OPTIMISTIC')
            req_band = min(conn_band[2], conn_band[1] + 0.35 * (conn_band[2] - conn_band[0]))

            for idx in range(len(path_1) -1): # check for path_1 -> shortest path               
                print('\nFor', path_1[idx], path_1[idx+1], 'Total bandwidth:', linkcap[path_1[idx]][path_1[idx+1]], 'Required bandwidth:', req_band)

                tmp, band_eq = check_bandwidth_o(linkcap[path_1[idx]][path_1[idx+1]], conn_band) # eve_conn[4] is the max bandwidth
                can_admit_short = can_admit_short and tmp
            if can_admit_short == True: # if can_admit: --> shortest path is OK
                conn_id, linkcap, conn_vcids, conn_cost = setup_connection(path_1, linkcap, band_eq, vcid, shortest_cost)
                write_forwardingfile(path_1, conn_vcids, ft)
                write_pathsfile(path_1, conn_vcids, conn_id, src, dst, conn_cost, pathforpaths)

                print('connection id:', conn_id, 'vcids:', conn_vcids, 'path_1:', path_1, 'cost:', conn_cost)
                print('updated linkcap:')
                for i in range(n):
                    for j in range(n):
                        print(linkcap[i][j], end = ' ')
                    print()
            else:
                print('\nChecking for path_2:', path_2)
                for idx in range(len(path_2) -1): # check for path_2 -> second shortest path
                    print('\nFor', path_2[idx], path_2[idx+1], 'Total bandwidth:', linkcap[path_2[idx]][path_2[idx+1]], 'Required bandwidth:', req_band)

                    tmp, band_eq = check_bandwidth_o(linkcap[path_2[idx]][path_2[idx+1]], conn_band) # eve_conn[4] is the max bandwidth
                    can_admit_secshort = can_admit_secshort and tmp
                
                if can_admit_secshort == True: # second shortest OK
                    conn_id, linkcap, conn_vcids, conn_cost = setup_connection(path_2, linkcap, band_eq, vcid, second_shortest_cost)
                    write_forwardingfile(path_2, conn_vcids, ft)
                    write_pathsfile(path_2, conn_vcids, conn_id, src, dst, conn_cost, pathforpaths)   

                    print('connection id:', conn_id, 'vcids:', conn_vcids, 'path_2:', path_2, 'cost:', conn_cost)
                    print('updated link:')
                    for i in range(n):
                        for j in range(n):
                            print(linkcap[i][j], end = ' ')
                        print()
                else: # connection rejected
                    conn_ids = conn_ids + 1 

    return conn_ids     
        
        # break
        # open forwarding file, pathsfile and dump the values
                
        #p=0
        # check for flag...
                # '''
                # Process each request:
                # // first check if pessi/opti
                # //Pessi:
                #     for each_edge in range(no of edges in the path -> len(path)-1):
                #         check bandwidth
                #     if True at all times:
                #         link_cap_arr[i][j] and link_cap_arr[j][i] for each edge
                #     else:
                #         No connection
                # //Opti:
                #     Same as above-> but also have an if condition (elif) for second path
                    
                # '''


