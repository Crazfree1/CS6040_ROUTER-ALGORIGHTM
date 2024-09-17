import heapq

def dijkstra(graph, start, n, ignore_edges=set()):
    dist = [float('inf')] * n
    paths = [[] for _ in range(n)]
    
    dist[start] = 0
    paths[start] = [start]

    pq = [(0, start)]

    while pq:
        current_dist, node = heapq.heappop(pq)

        # Explore neighbors
        for neighbor in graph[node]:
            next_node, hop_count = neighbor, 1  # Hop count is 1 for each edge
            new_dist = current_dist + hop_count

            # Skip if the edge is in the ignore set (for second shortest path calculation)
            if (node, next_node) in ignore_edges or (next_node, node) in ignore_edges:
                continue

            # Check if this new distance is shorter
            if new_dist < dist[next_node]:
                dist[next_node] = new_dist
                paths[next_node] = paths[node] + [next_node]
                heapq.heappush(pq, (new_dist, next_node))

    return dist, paths

def find_second_shortest(graph, start, end, n, shortest_path):
    second_shortest_dist = float('inf')
    second_shortest_path = []

    # Try to ignore each edge in the shortest path and find an alternative path
    for i in range(len(shortest_path) - 1):
        ignore_edges = {(shortest_path[i], shortest_path[i + 1])}

        dist, paths = dijkstra(graph, start, n, ignore_edges)

        if dist[end] < second_shortest_dist and paths[end] != shortest_path:
            second_shortest_dist = dist[end]
            second_shortest_path = paths[end]

    return second_shortest_dist, second_shortest_path

def all_pairs_shortest_and_second_shortest(graph, n):
    all_distances_and_paths = []

    for node in range(n):
        shortest_distances, shortest_paths = dijkstra(graph, node, n)

        second_shortest_distances = []
        second_shortest_paths = []
        for target in range(n):
            if node != target:
                second_dist, second_path = find_second_shortest(graph, node, target, n, shortest_paths[target])
            else:
                second_dist, second_path = float('inf'), []

            second_shortest_distances.append(second_dist)
            second_shortest_paths.append(second_path)

        all_distances_and_paths.append((shortest_distances, shortest_paths, second_shortest_distances, second_shortest_paths))

    return all_distances_and_paths

def compute_route(nodes, prop, graph):
    '''
    prop is for printing the path delay, path cost is the number of hops
    '''
    # # Example usage:
    # graph = {
    #     0: [1, 2],
    #     1: [0, 3, 2],
    #     2: [0, 1, 4],
    #     3: [1, 4],
    #     4: [2, 3]
    # }

    n = nodes
    # n = 5  # Number of nodes
    all_distances_and_paths = all_pairs_shortest_and_second_shortest(graph, n)


    # Print the shortest and second shortest paths between all pairs of nodes
    with open('./routingfiles/routingtablefile.txt', 'w') as f:
        for i in range(n):
            for j in range(n):

                shortest_dist, shortest_path = all_distances_and_paths[i][0][j], all_distances_and_paths[i][1][j]
                second_shortest_dist, second_shortest_path = all_distances_and_paths[i][2][j], all_distances_and_paths[i][3][j]

                acc = 0
                for x in range(len(shortest_path)-1):
                    acc += prop[shortest_path[x]][shortest_path[x+1]]

                acc_second = 0
                for x in range(len(second_shortest_path)-1):
                    acc_second += prop[second_shortest_path[x]][second_shortest_path[x+1]]
                
                print(f"From node {i} to node {j}:")
                print(f"  Shortest path = {shortest_dist} hops, Path: {shortest_path}, Delay: {acc} ms")
                print(f"  Second shortest path = {second_shortest_dist} hops, Path: {second_shortest_path}, Delay: {acc_second} ms")

                f.write(f"{shortest_path[len(shortest_path)-1]} {shortest_path} {acc} {shortest_dist}\n")
                # if len(second_shortest_path):
                    # f.write(f"{second_shortest_path[len(second_shortest_path)-1]} {second_shortest_path} {acc_second} {second_shortest_dist}\n")

    with open('./routingfiles/routingtablefile_second.txt', 'w') as f:
        for i in range(n):
            for j in range(n):

                # shortest_dist, shortest_path = all_distances_and_paths[i][0][j], all_distances_and_paths[i][1][j]
                second_shortest_dist, second_shortest_path = all_distances_and_paths[i][2][j], all_distances_and_paths[i][3][j]

                # acc = 0
                # for x in range(len(shortest_path)-1):
                    # acc += prop[shortest_path[x]][shortest_path[x+1]]

                acc_second = 0
                for x in range(len(second_shortest_path)-1):
                    acc_second += prop[second_shortest_path[x]][second_shortest_path[x+1]]
                
                print(f"From node {i} to node {j}:")
                print(f"  Shortest path = {shortest_dist} hops, Path: {shortest_path}, Delay: {acc} ms")
                print(f"  Second shortest path = {second_shortest_dist} hops, Path: {second_shortest_path}, Delay: {acc_second} ms")

                # f.write(f"{shortest_path[len(shortest_path)-1]} {shortest_path} {acc} {shortest_dist}\n")
                if len(second_shortest_path):
                    f.write(f"{second_shortest_path[len(second_shortest_path)-1]} {second_shortest_path} {acc_second} {second_shortest_dist}\n")

            
                