from dijkstraAlgo import dijkstra_Algorithm
from dijkstraAlgo import shortest_path_1
from dijkstraAlgo import shortest_path_2

def build_graph_with_topo_file(topoFile):
    
    # file is opened in read mode.
    with open(topoFile, "r") as file:
        
        line_1 = file.readline()
        vertex_count, edge_count = map(int, line_1.split(" "))
        
        # I am representing graph in form of Adjacency list
        graph = {}
        for vertex in range(vertex_count):
            graph[vertex] = []

        for line in file:
            temp = line.split()
            src = int(temp[0])
            dest = int(temp[1])
            tp = float(temp[2])
            bw = float(temp[3])
            graph[src].append((dest, tp, bw))
            graph[dest].append((src, tp, bw))

    return graph


def build_routing_table(G,N,pathMetric):
    r_table = {}

    for src in range(N):
        
        r_table[src] = {}
        for dest in range(N):
            if src != dest:
                first_path, first_delay, first_cost = shortest_path_1(G, src, dest, pathMetric)
                second_path_data = shortest_path_2(G, first_path, src, dest, pathMetric)

                if second_path_data:
                    second_path, second_delay, second_cost = second_path_data
                else:
                    second_path, second_delay, second_cost = None, None, None
                r_table[src][dest] = {'Path_1': first_path, 'Path_1_delay': first_delay,'Path_1_cost': first_cost,
                    'Path_2': second_path, 'Path_2_delay': second_delay, 'Path_2_cost': second_cost }
    return r_table