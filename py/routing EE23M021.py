import argparse
import numpy as np
import csv
from collections import defaultdict
from heapq import heappush, heappop

debug=0  # To turn on debug mode
def two_shortest_paths(graph, source, target, k=2, metric="hop"):
    # Priority queue to store (metric value, path) tuples
    queue = [(0, [source], 0)]
    paths = []
    seen = set()

    while queue and len(paths) < k:
        (cost, path, delay) = heappop(queue)
        node = path[-1]

        if (node, tuple(path)) in seen:
            continue

        seen.add((node, tuple(path)))

        if node == target:
            paths.append((cost, path, delay))

        for neighbor, properties in graph.get(node, {}).items():
            if neighbor not in path:  # Avoid cycles
                new_path = path + [neighbor]
                
                # Calculate new cost based on the selected metric
                if metric == "hop":
                    new_cost = cost + 1
                    new_delay=delay+int(properties["delay"])
                elif metric == "delay":
                    new_cost = cost + int(properties["delay"])
                    new_delay=new_cost
                else:
                    raise ValueError("Unsupported metric")

                heappush(queue, (new_cost, new_path, new_delay))

    return paths

def all_pair_two_shortest_paths(graph, k=2, metric="hop"):
    all_paths = {}
    nodes = list(graph.keys())

    for source in nodes:
        for target in nodes:
            if source != target:
                all_paths[(source, target)] = two_shortest_paths(graph, source, target, k, metric)

    return all_paths

def create_adjacency_matrix(adjacency_list):
    nodes = sorted(adjacency_list.keys())
    n = len(nodes)
    
    # Create a node index mapping
    node_index = {node: i for i, node in enumerate(nodes)}
    
    # Initialize the adjacency matrix with infinity (no direct path)
    adjacency_matrix = np.full((n, n), np.inf)
    
    # Set the diagonal to zero (distance from node to itself)
    np.fill_diagonal(adjacency_matrix, 0)
    
    # Fill the adjacency matrix with the weights from the adjacency list
    for node, edges in adjacency_list.items():
        for neighbor, weight in edges:
            i = node_index[node]
            j = node_index[neighbor]
            adjacency_matrix[i][j] = weight
    
    return adjacency_matrix, node_index

def add_edge(graph, node1, node2, delay, capacity):
    if node1 not in graph:
        graph[node1] = {}  # making new dictionary to add in a dictionary
    if node2 not in graph:
        graph[node2] = {}

    graph[node1][node2] = {'delay': delay, 'capacity': float(capacity)}
    graph[node2][node1] = {'delay': delay, 'capacity': float(capacity)}  # For undirected graph
def check_connection_acceptance(all_paths, graph, source, target, BWmin, BWavg, BWmax,p ):
    if (source, target) not in all_paths:
        if debug:
            print("exit due to path not found")
        return False, None, None
    if p:
        BWeq=BWmax
    else:
        BWeq=min(BWmax,(BWavg+0.35*(BWmax-BWmin)))
    if debug:
        print("BW eq",BWeq)
        print("Source",source, "Target",target)
    paths =all_paths.get((source, target))
    for j,(cost, path, delay) in enumerate(paths,1): # we will check 2 paths one by one and once 
        #and on connection acceptance we need to update the available cost for that and opposite path
        # Check if the path meets the BW requirement
        path_accepted = True
        for i in range(len(path) - 1):
            node1 = path[i]
            node2 = path[i + 1]
            
            # Assume a global graph dictionary G that stores capacity info
            if float(graph[node1][node2]['capacity']) < BWeq:
                path_accepted = False
                break
        
        if path_accepted:
            for i in range(len(path) - 1):
                node1 = path[i]
                node2 = path[i + 1]
                temp=graph[node1][node2]['capacity'] # To ensure safe copying
                graph[node1][node2]['capacity']=float(temp)-BWeq #updated remaining capacity for entire path 
            #graph[node2][node1]['capacity']=temp-BWeq # Assuming Bidirectional link are independent
            return True, path, delay  # Connection accepted, return the path
        if debug:
            print("Path cost, ",cost, f"Failed Path: {' -> '.join(path)}, Path delay:", delay)
            #breakpoint()
    
    return False, None, None  # Connection rejected
def create_forwarding_table_with_vc_ids(path, forwarding_table, initial_vc_id=1):
    current_vc_id = initial_vc_id
    vc_id_list=[None] # starts with None to accomodate beggining node where previous is None
    for i in range(len(path) - 1):
        source = path[i]
        next_hop = path[i + 1]

        if source not in forwarding_table:
            forwarding_table[source] = {}

        if next_hop not in forwarding_table[source]:
            # Assign a VC ID to the (source, next_hop) pair
            forwarding_table[source][next_hop] = current_vc_id
            vc_id_list.append(current_vc_id)
            #increment ID
            current_vc_id += 1
        elif next_hop in forwarding_table[source]:  # if that path present 
            vc_id_list.append(forwarding_table[source][next_hop])
    return current_vc_id, vc_id_list
def main(args):
    # Print out the received arguments to ensure they are read correctly
    #'''
    print("ignore this internal print")
    print("The input given is")
    print("Topology file:", args.topologyfile)
    print("Connections file:", args.connectionsfile)
    print("Routing table file:", args.routingtablefile)
    print("Forwarding table file:", args.forwardingfile)
    print("Paths file:", args.pathsfile)
    print("Metric type:", args.metric)
    if args.mode:
        print("Mode:", args.mode," ie. (Pessimistic Approach)")
    else:
        print("Mode:", args.mode, "ie. (Optimistic Approach)")
    #'''
    adjacency_list = defaultdict(list)
    G={}
    filename=args.topologyfile
    with open(filename, 'r') as file:
        beg_line=1
        for line in file:
            if beg_line:
                nodecount,edgecount=line.split()
                beg_line=0
            else:
                node1, node2, delay, capacity = line.split()
                #capacity = float(capacity)  # Convert weight to a number (float)
                add_edge(G,node1,node2,delay,capacity)
                # Add the edge to the adjacency list (both directions for undirected graph)
                adjacency_list[node1].append((node2, capacity))
                adjacency_list[node2].append((node1, capacity))  
    #print(G)
    '''
    adjacency_matrix, node_index = create_adjacency_matrix(adjacency_list)
    print(adjacency_matrix)
    breakpoint()
    print("Connections file:", args.connectionsfile)
    print("Routing table file:", args.routingtablefile)
    print("Forwarding table file:", args.forwardingfile)
    print("Paths file:", args.pathsfile)
    print("Metric type:", "Hop count" if args.metric == "hop" else "Distance")
    print("Mode:", "Basic" if args.mode == 0 else "Simulation")
    #'''
    #Calculating 2 shortest paths
    if args.metric=='hop':
        all_paths = all_pair_two_shortest_paths(G, k=2, metric="hop")
        '''
        all_paths_hops = all_pair_two_shortest_paths(G, k=2, metric="hop")
        print("Shortest paths based on Hops:")
        for (source, target), paths in all_paths_hops.items():
            print(f"Paths between {source} and {target}:")
            for i, (cost, path, delay) in enumerate(paths, 1):
                print(f"  {i}th shortest path (hops: {cost}),delay:{delay}: {' -> '.join(path)}")
        #'''
        with open(args.routingtablefile, 'w', newline='') as file:
            writer = csv.writer(file)
            
            # Write the header
            writer.writerow(['Source', 'Destination', 'Path Number', 'Path Cost','Path Delay', 'Path(Source->dest)'])
            
            for (source, target), paths in all_paths.items():
                for i, (cost, path, delay) in enumerate(paths, 1):
                    path_str = ' -> '.join(path)
                    # Write each path to the CSV file
                    writer.writerow([source, target, i, cost, delay, path_str])

    elif args.metric=='dist':
        all_paths = all_pair_two_shortest_paths(G, k=2, metric="delay")
        '''
        if debug:
            print(all_paths)
        all_paths_delay = all_pair_two_shortest_paths(G, k=2, metric="delay")
        print("Shortest paths based on delay/Distance:")
        for (source, target), paths in all_paths_delay.items():
            print(f"Paths between {source} and {target}:")
            for i, (cost, path, delay) in enumerate(paths, 1):
                #print("Source",source,target)
                print(f"  {i}th shortest path (delay: {cost}): {' -> '.join(path)}")
        #'''
        with open(args.routingtablefile, 'w', newline='') as file:
            writer = csv.writer(file)
            
            # Write the header
            writer.writerow(['Source', 'Destination', 'Path Number', 'Path Cost','Path Delay', 'Path(Source->dest)'])
            
            for (source, target), paths in all_paths.items():
                for i, (cost, path, delay) in enumerate(paths, 1):
                    path_str = ' -> '.join(path)
                    # Write each path to the CSV file
                    writer.writerow([source, target, i, cost, delay, path_str])

    forwarding_table={}
    forewarding_list={}
    accepted_conn=[] #stores connection information of accepted connections
    current_vc_id=2300
    filename=args.connectionsfile
    with open(filename, 'r') as file:
        beg_line=1
        conn_addmitted=0
        for line in file:
            if beg_line:
                total_conn_num=line
                beg_line=0
            else:
                node1, node2, BWmin, BWavg, BWmax = line.split()
                if debug:
                    print("Source:",node1, "dest",node2, "BWmin",int(BWmin), float(BWavg),"BWmax: ",BWmax)
                accepted,path,path_delay=check_connection_acceptance(all_paths, G, node1, node2, int(BWmin), float(BWavg), int(BWmax),int(args.mode) )
                if accepted:
                    conn_addmitted=conn_addmitted+1
                    current_vc_id, vc_id_list=create_forwarding_table_with_vc_ids(path, forwarding_table, current_vc_id)
                    prev_node=None
                    curr_node=None
                    count=0
                    if debug:
                        print("vc_id_list",vc_id_list)
                    for i in range( len(path)):
                        if i and i<(len(path) - 1):
                            prev_node = path[i - 1]
                            Prev_VID=vc_id_list[i]
                            this_node = path[i]
                            Next_VID=vc_id_list[i + 1]
                            next_node = path[i + 1]
                        elif i==0 and i<(len(path) - 1):
                            prev_node = None
                            Prev_VID=None
                            this_node = path[i]
                            Next_VID=vc_id_list[i + 1]
                            next_node = path[i + 1]
                        else:
                            prev_node = path[i - 1]
                            Prev_VID=vc_id_list[i]
                            this_node = path[i]
                            Next_VID=None
                            next_node = None
                        temp=[]
                        temp.append([this_node,prev_node,Prev_VID,next_node,Next_VID])
                        if debug:
                            print(temp)
                            #breakpoint()
                        if this_node not in forewarding_list:
                            forewarding_list[this_node]=temp
                        else:
                            temp_n=forewarding_list[this_node]
                            temp_n.append(temp)
                            forewarding_list[this_node]=temp_n
                        if debug:
                            print("prev_node",prev_node,"Prev_VID",Prev_VID,"this_node",this_node,f"(i={i})","next_node",next_node,"Next_VID",Next_VID)
                    path_str = ' -> '.join(path)
                    accepted_conn.append(((node1,node2),path_str,vc_id_list,path_delay))
                    if debug:
                        print("Sucess path",path,"Path delay",path_delay)
                        #breakpoint()
                    #prev_node,Prev_VID,this_node,next_node,Next_VID
                    '''
                    for node in path:
                        
                        if node not in forewarding_list and count:
                            
                            print("length of sucess path",len(path)," Path",path)
                            if count<len(vc_id_list) and count==1:
                                forewarding_list[curr_node]=(None,None,node,vc_id_list[count-1])
                            elif count<len(vc_id_list) and count>=1:
                                forewarding_list[curr_node]=(prev_node,vc_id_list[count-2],node,vc_id_list[count])  
                            elif count>len(vc_id_list) :
                                forewarding_list[curr_node]=(prev_node,vc_id_list[count-1],None,None)
                            elif count==len(vc_id_list) and count==1:
                                forewarding_list[curr_node]=(None,None,node,vc_id_list[count-1])
                            
                            if debug:
                                print(forewarding_list[node])
                                breakpoint()
                        prev_node=curr_node
                        curr_node=node
                        count=count+1
                    #'''
                    

    print("connection addmitted: ",conn_addmitted," total connections: ",total_conn_num)
    if debug:
        print(forwarding_table)
        print(forwarding_table.get('1'))
    with open(args.pathsfile, 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow([int(total_conn_num),int(conn_addmitted)])
        writer.writerow(['ConnID', 'Source', 'Destination', 'Path(Source->dest)', 'VC ID list', 'Path Cost(delay)'])
        for i in range(len(accepted_conn)):
            writer.writerow([i, accepted_conn[i][0][0],accepted_conn[i][0][1], accepted_conn[i][1], accepted_conn[i][2][1:], accepted_conn[i][3]])
    with open(args.forwardingfile, 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Router ID', 'Incoming ID', 'Incoming VC ID', 'Outgoing ID', 'Outgoing VC ID'])
        
        for router in forwarding_table:
            if router in forewarding_list:
                temp=forewarding_list[router]
                if debug:
                    print(router," ....")
                    print(forewarding_list[router])
                    print("temp ",temp)
                for value in temp:
                    writer.writerow([value[0],value[1],value[2],value[3],value[4]])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Virtual Packet-Switching Network Routing Simulation")

    parser.add_argument("-top","--topologyfile", type=str, help="Path to the topology file")
    parser.add_argument("-conn", "--connectionsfile", type=str, required=True, help="Path to the connections file")
    #'''
    parser.add_argument("-rt", "--routingtablefile", type=str, required=True, help="Path to the routing table file")
    parser.add_argument("-ft", "--forwardingfile", type=str, required=True, help="Path to the forwarding table file")
    parser.add_argument("-path", "--pathsfile", type=str, required=True, help="Path to the paths file")
    parser.add_argument("-flag", "--metric", choices=["hop", "dist"], required=True, help="Metric for shortest path: hop (hop count) or dist (distance)")
    parser.add_argument("-p", "--mode", type=int, choices=[0, 1], required=True, help="Mode: 0 for Optimistic approach, 1 for Pessimistic approach")

    args = parser.parse_args()
    main(args)
