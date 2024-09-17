import heapq
from copy import deepcopy
"""
It is recommended to use heap(priority Q),as we have to extract min every single time,
for which meanHeap will work wonder in just O(log n)
"""
def dijkstra_Algorithm(graph, src, metric):
    distances = {node: float('infinity') for node in graph}
    distances[src] = 0
    prev_vertices = {node: None for node in graph}
    priority_Q = [(0, src)]
    
    while priority_Q:
        current_distance, current_node = heapq.heappop(priority_Q)
        
        if current_distance > distances[current_node]:
            continue
        
        for neighbor, delay, capacity in graph[current_node]:
            if metric == "hop":
                distance = current_distance + 1
            elif metric == "distance":
                distance = current_distance + delay
            
            if distance < distances[neighbor]:
                distances[neighbor] = distance
                prev_vertices[neighbor] = current_node
                heapq.heappush(priority_Q, (distance, neighbor))
    
    return distances, prev_vertices

def shortest_path_1(graph,src,dest, pathMetric):
    total_delay = 0
    path = []
    node = dest

    distances, prev_vertices = dijkstra_Algorithm(graph, src, pathMetric)

    #backtracks through prev nodes to recontruct the path
    while node is not None:
        path.append(node)
        node =prev_vertices[node]
    path = path[::-1]   # reverse the path to get the correct order

    for i in range(len(path) - 1):
        u, v = path[i], path[i + 1]
        for neighbor, delay, capacity in graph[u]:
            if neighbor == v:
                total_delay += delay
    return path, total_delay, distances[dest]



def shortest_path_2(graph, first_path, start_node, end_node, metric):
    second_shortest_paths = []
    
    for i in range(len(first_path) - 1):
        modified_graph = deepcopy(graph)
        u, v = first_path[i], first_path[i + 1]
        modified_graph[u] = [(node, d, c) for node, d, c in modified_graph[u] if node != v]
        modified_graph[v] = [(node, d, c) for node, d, c in modified_graph[v] if node != u]
        try:
            new_path, new_delay, new_cost = shortest_path_1(modified_graph, start_node, end_node, metric)
            if new_path != first_path:
                second_shortest_paths.append((new_path, new_delay, new_cost))
        except:
            continue
    if second_shortest_paths:
        second_shortest_paths.sort(key=lambda x: x[2])
        return second_shortest_paths[0]
    return None