#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <unordered_set>
#include <fstream>
#include <algorithm>
#include <cstring>

using namespace std;

class Node 
{
public:
    int id;  
    int val;

    Node(): id(0), val(0) {}

    Node(int i, int v) 
    {
        setValues(i, v);
    }

    void setValues(int i, int v) 
    {
        id = i;
        val = v;
    }

    bool operator<(const Node& tp) const 
    {
        return val > tp.val;
    }
};

class Edge 
{
public:
    int destination, delay, capacity;

    Edge(): destination(0), delay(0), capacity(0) {}

    Edge(int dest, int d, int c) 
    {
        setValues(dest, d, c);
    }

    void setValues(int dest, int d, int c) 
    {
        destination = dest;
        delay = d;
        capacity = c;
    }
};

pair<vector<int>, vector<int>> ShortestPath(int startNode, int totalNodes, int useDistanceMetric, const vector<vector<Edge>>& adjacencyList) 
{
    priority_queue<Node> minHeap;
    minHeap.push(Node(startNode, 0));

    vector<int> distances(totalNodes, INT_MAX);
    vector<int> predecessors(totalNodes, -1);

    distances[startNode] = 0;

    while (!minHeap.empty()) 
    {
        Node currentNode = minHeap.top();
        int currentIndex = currentNode.id;
        int currentCost = currentNode.val;
        minHeap.pop();

        if (currentCost > distances[currentIndex]) 
        {
            continue;
        }

        for (const auto& edge : adjacencyList[currentIndex]) 
        {
            int neighborIndex = edge.destination;
            int newCost = (useDistanceMetric) ? currentCost + edge.delay : currentCost + 1;

            if (newCost < distances[neighborIndex]) 
            {
                distances[neighborIndex] = newCost;
                predecessors[neighborIndex] = currentIndex;
                minHeap.push(Node(neighborIndex, newCost));
            }
        }
    }

    return {distances, predecessors}; 
}

pair<vector<int>, vector<int>> SecondShortestPath(int startNode, int totalNodes, int useDistanceMetric, const vector<vector<Edge>>& adjacencyList) 
{
    vector<int> shortestDistances(totalNodes, INT_MAX);
    vector<int> secondShortestDistances(totalNodes, INT_MAX);
    vector<int> shortestPredecessors(totalNodes, -1);
    vector<int> secondShortestPredecessors(totalNodes, -1);

    priority_queue<Node> minHeap;
    minHeap.push(Node(startNode, 0));
    shortestDistances[startNode] = 0;

    while (!minHeap.empty()) 
    {
        Node currentNode = minHeap.top();
        int currentIndex = currentNode.id;
        int currentCost = currentNode.val;
        minHeap.pop();

        for (const auto& edge : adjacencyList[currentIndex]) 
        {
            int neighborIndex = edge.destination;
            int newCost = (useDistanceMetric) ? currentCost + edge.delay : currentCost + 1;

            if (newCost < shortestDistances[neighborIndex]) 
            {
                secondShortestDistances[neighborIndex] = shortestDistances[neighborIndex];
                secondShortestPredecessors[neighborIndex] = shortestPredecessors[neighborIndex];

                shortestDistances[neighborIndex] = newCost;
                shortestPredecessors[neighborIndex] = currentIndex;
                minHeap.push(Node(neighborIndex, newCost));
            } 
            else if (newCost > shortestDistances[neighborIndex] && newCost < secondShortestDistances[neighborIndex]) 
            {
                secondShortestDistances[neighborIndex] = newCost;
                secondShortestPredecessors[neighborIndex] = currentIndex;
                minHeap.push(Node(neighborIndex, newCost));
            }
        }
    }

    return {secondShortestDistances, secondShortestPredecessors}; 
}

vector<int> posPath(int start, int end, const vector<int>& predecessors) 
{
    vector<int> path;
    int current = end;
    
    while (current != -1) 
    {
        path.push_back(current);
        current = predecessors[current];
    }
    
    reverse(path.begin(), path.end());

    if (!path.empty() && path[0] == start) 
    {
        return path;
    }
    
    return {};  
}

int main(int argc, char* argv[]) 
{
    // Argument parsing
    if (argc < 13) 
    {
        cerr << "Usage: ./routing -top topologyfile -conn connectionsfile -rt routingtablefile -ft forwardingfile -path pathsfile -flag hop|dist -p 0|1" << endl;
        return 1;
    }

    string topologyFile, connectionsFile, routingTableFile, forwardingFile, pathsFile, flag;
    int pFlag = 0;

    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], "-top") == 0) 
        {
            topologyFile = argv[++i];
        } 
        else if (strcmp(argv[i], "-conn") == 0) 
        {
            connectionsFile = argv[++i];
        } 
        else if (strcmp(argv[i], "-rt") == 0) 
        {
            routingTableFile = argv[++i];
        } 
        else if (strcmp(argv[i], "-ft") == 0) 
        {
            forwardingFile = argv[++i];
        } 
        else if (strcmp(argv[i], "-path") == 0) 
        {
            pathsFile = argv[++i];
        } 
        else if (strcmp(argv[i], "-flag") == 0) 
        {
            flag = argv[++i];
        } 
        else if (strcmp(argv[i], "-p") == 0) 
        {
            pFlag = stoi(argv[++i]);
        }
    }

    // Reading topology file
    ifstream topoFile(topologyFile);
    if (!topoFile.is_open()) 
    {
        cerr << "Error" << endl;
        return 1;
    }

    int N, E;
    topoFile >> N >> E;
    vector<vector<Edge>> graph(N);

    for (int i = 0; i < E; ++i) 
    {
        int u, v, delay, capacity;
        topoFile >> u >> v >> delay >> capacity;
        graph[u].push_back(Edge(v, delay, capacity));
        graph[v].push_back(Edge(u, delay, capacity));
    }
    topoFile.close();

    // Open files for writing
    ofstream routingFile(routingTableFile), forwardingFileOut(forwardingFile), pathsFileOut(pathsFile);

    if (!routingFile.is_open() || !forwardingFileOut.is_open() || !pathsFileOut.is_open()) 
    {
        cerr << "Error" << endl;
        return 1;
    }

    // Determine whether to use hop or distance
    int useDistance = (flag == "dist") ? 1 : 0;

    for (int source = 0; source < N; ++source) 
    {
        // Calculate shortest and second shortest paths
        auto [shortestDist, prev] = ShortestPath(source, N, useDistance, graph);
        auto [secondShortestDist, _] = SecondShortestPath(source, N, useDistance, graph);

        routingFile << "Routing table for node " << source << ":\n";
        for (int dest = 0; dest < N; ++dest) 
        {
            if (source != dest) 
            {
                // Output shortest path
                if (shortestDist[dest] != INT_MAX) 
                {
                    vector<int> shortestPath = posPath(source, dest, prev);
                    routingFile << "Destination: " << dest << ", Shortest Path: ";
                    for (int node : shortestPath) 
                    {
                        routingFile << node << " ";
                    }
                    routingFile << "Cost: " << shortestDist[dest] << endl;
                }

                // Output second shortest path
                if (secondShortestDist[dest] != INT_MAX) 
                {
                    vector<int> secondShortestPath = posPath(source, dest, prev);
                    routingFile << "Destination: " << dest << ", Second Shortest Path: ";
                    for (int node : secondShortestPath) 
                    {
                        routingFile << node << " ";
                    }
                    routingFile << "Cost: " << secondShortestDist[dest] << endl;
                }
            }
        }
        routingFile << endl;
    }

    routingFile.close();
    forwardingFileOut.close();
    pathsFileOut.close();

    return 0;
}

