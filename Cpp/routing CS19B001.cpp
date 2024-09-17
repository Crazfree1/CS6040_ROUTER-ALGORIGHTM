#include <bits/stdc++.h>
using namespace std;

const int INF = numeric_limits<int>::max();

// Structure to store edge information
struct Edge
{
    int to;
    int delay;
    int capacity;

    // Define the < operator
    bool operator<(const Edge &other) const
    {
        if (to != other.to)
            return to < other.to;
        if (delay != other.delay)
            return delay < other.delay;
        return capacity < other.capacity;
    }
};

// Structure to store connection request information
struct ConnectionRequest
{
    int src;
    int dest;
    double b_min, b_ave, b_max;
};

struct Path
{
    int cost;
    int delay;
    vector<int> nodes;

    // Comparator for sorting paths by cost
    bool operator<(const Path &other) const
    {
        return cost < other.cost;
    }
};

class Router
{
private:
    int N, E, R, p;
    int metricFlag; // metric for hop = 1, dist = 0;
    int MAX_VCID;   // max no. of VCID's per link - default : 10

    vector<vector<Edge>> graph; // graph as an adjacency list
    vector<ConnectionRequest> connections;
    vector<vector<vector<Path>>> allPaths;              // allPaths[i][j] = two shortest paths from i to j
    vector<vector<vector<int>>> forwardingTable;        // forwardingTable[i] = forwading table of node i
    vector<pair<Path, vector<int>>> connectionVcidList; // Details for path file

    set<int> droppedConnections; // conn ID's of dropped connections

    map<Edge, int> VCID;             // the min VCID that we can use for that corresponding link
    map<Edge, double> totCapUsed;    // total capacity used up (based on p=0 or p=1) on this link for connections
    map<pair<int, int>, Edge> edges; // edge from i to j is e then edges[{i,j}] = e

public:
    Router(int metric_flag, int p) : metricFlag(metric_flag), p(p), MAX_VCID(10) {}

    void parseTopology(const string &filename)
    {
        ifstream file(filename);
        file >> N >> E;
        forwardingTable.resize(N);
        graph.resize(N);
        for (int i = 0; i < E; ++i)
        {
            int from, to, delay, capacity;
            file >> from >> to >> delay >> capacity;
            graph[from].push_back({to, delay, capacity});
            graph[to].push_back({from, delay, capacity});

            edges[{from, to}] = {to, delay, capacity};
            edges[{to, from}] = {from, delay, capacity};
        }
    }

    void parseConnections(const string &filename)
    {
        ifstream file(filename);
        file >> R;
        connections.resize(R);
        connectionVcidList.resize(R);
        for (int i = 0; i < R; ++i)
        {
            file >> connections[i].src >> connections[i].dest >> connections[i].b_min >> connections[i].b_ave >> connections[i].b_max;
        }
    }

    vector<int> getRootPath(const vector<int> &nodes, int index)
    {
        return vector<int>(nodes.begin(), nodes.begin() + index + 1);
    }

    // Function to run Dijkstra's algorithm to find the shortest path
    Path dijkstra(int start, int end, vector<vector<Edge>> &graph, int useHopMetric)
    {
        int n = graph.size();
        vector<int> d(n, INF), pr(n, -1);
        d[start] = 0;

        using PII = pair<int, int>;
        priority_queue<PII, vector<PII>, greater<PII>> pq;
        pq.push({0, start});

        while (!pq.empty())
        {
            int u = pq.top().second;
            int curDist = pq.top().first;
            pq.pop();

            if (u == end)
            {
                break;
            }

            for (const auto &edge : graph[u])
            {
                int v = edge.to;
                int weight = useHopMetric ? 1 : edge.delay;

                if (curDist + weight < d[v])
                {
                    pr[v] = u;
                    d[v] = curDist + weight;
                    pq.push({d[v], v});
                }
            }
        }

        Path path = {d[end], 0, {}};
        for (int v = end; v != -1; v = pr[v])
        {
            path.nodes.push_back(v);
        }
        reverse(path.nodes.begin(), path.nodes.end());

        // Calculate the delay
        path.delay = calculatePathDelay(path.nodes);

        return path;
    }

    vector<Path> yen2ShortestPaths(int start, int end, int useHopMetric)
    {
        // The shortest path found by Dijkstra is the first path
        Path firstPath = dijkstra(start, end, graph, useHopMetric);
        if (firstPath.cost == INF)
            return {}; // No path found

        // Calculate the delay for the first path
        firstPath.delay = calculatePathDelay(firstPath.nodes);

        // Store the first path as the initial shortest path
        vector<Path> shortestPaths = {firstPath};

        // A set to store candidate paths
        set<Path> candidates;

        const Path &prevPath = shortestPaths[0];

        // Iterate through the nodes in the previous shortest path to create deviations
        for (int j = 0; j < prevPath.nodes.size() - 1; ++j)
        {
            int spurNode = prevPath.nodes[j];
            vector<int> rootPath = getRootPath(prevPath.nodes, j);

            // Create a copy of the graph to modify for this spur path
            vector<vector<Edge>> modifiedGraph = graph;

            // Remove edges that are part of the rootPath from the graph
            for (const auto &path : shortestPaths)
            {
                if (equal(rootPath.begin(), rootPath.end(), path.nodes.begin()))
                {
                    int u = path.nodes[j];
                    int v = path.nodes[j + 1];

                    // Remove the edge u -> v
                    modifiedGraph[u].erase(remove_if(modifiedGraph[u].begin(), modifiedGraph[u].end(),
                                                     [v](const Edge &e)
                                                     { return e.to == v; }),
                                           modifiedGraph[u].end());
                }
            }

            // Calculate the spur path from the spurNode to the destination
            Path spurPath = dijkstra(spurNode, end, modifiedGraph, useHopMetric);
            if (spurPath.cost == INF)
                return shortestPaths; // No valid spur path

            // Combine the rootPath and spurPath
            Path totalPath = {0, 0, rootPath};
            totalPath.nodes.insert(totalPath.nodes.end(), spurPath.nodes.begin() + 1, spurPath.nodes.end());

            // Calculate the cost and delay for the total path
            totalPath.cost = calculatePathCost(totalPath.nodes, useHopMetric);
            totalPath.delay = calculatePathDelay(totalPath.nodes);

            // Add the new candidate path to the set
            candidates.insert(totalPath);
        }

        // If there are no candidates, break the loop
        if (candidates.empty())
            return shortestPaths;

        // The next shortest path is the one with the lowest cost
        shortestPaths.push_back(*candidates.begin());
        candidates.erase(candidates.begin());

        return shortestPaths;
    }

    // Helper function to calculate path delay
    int calculatePathDelay(const vector<int> &path)
    {
        int totalDelay = 0;
        for (int i = 0; i < path.size() - 1; ++i)
        {
            int u = path[i];
            int v = path[i + 1];
            for (const auto &edge : graph[u])
            {
                if (edge.to == v)
                {
                    totalDelay += edge.delay;
                    break;
                }
            }
        }
        return totalDelay;
    }

    // Helper function to calculate path cost
    int calculatePathCost(const vector<int> &path, bool useHopMetric)
    {
        if (useHopMetric)
            return path.size() - 1; // hop count is just the number of edges
        else
            return calculatePathDelay(path); // for distance metric, cost is the same as delay
    }

    void computeRoutingTables(const string &filename)
    {
        allPaths.resize(N);
        for (int i = 0; i < N; ++i)
        {
            allPaths[i].resize(N);
            for (int j = 0; j < N; ++j)
            {
                if (i != j)
                {
                    // compute the two shortest paths
                    vector<Path> paths = yen2ShortestPaths(i, j, metricFlag);

                    // fill in the routing table with these paths
                    writeRoutingTable(filename, paths);

                    // store the paths
                    move(paths.begin(), paths.end(), back_inserter(allPaths[i][j]));
                }
            }
        }
    }

    void writeRoutingTable(const string &filename, vector<Path> &paths)
    {
        ofstream file(filename, ios::app);

        for (auto &path : paths)
        {
            int destinationNode = path.nodes.back();

            // Format the output
            file << "Destination Node : " << destinationNode << " | ";
            file << "Path ( Node : " << path.nodes[0] << " to Node : " << destinationNode << " ) : ";

            // Path (Source -> Destination)
            for (size_t i = 0; i < path.nodes.size(); ++i)
            {
                file << path.nodes[i];
                if (i != path.nodes.size() - 1)
                {
                    file << " -> ";
                }
            }
            file << " | ";

            // Path Delay
            file << "Path Delay : " << path.delay << " | ";

            // Path Cost
            file << "Path Cost " << path.cost << "\n";
        }

        file.close();
    }

    void processConnections(int p)
    {
        for (int i = 0; i < R; i++)
        {
            bool dropped = false;
            int src = connections[i].src, dest = connections[i].dest;
            int noOfShortestPaths = allPaths[src][dest].size();

            Path path;
            if (noOfShortestPaths > 0)
            {
                path = allPaths[src][dest][0];
            }
            else
                dropped = true;

            if (!dropped && isPathPossible(i, path, p))
            {
                // build the forwarding table and vcid list
                buildVcidListAndForwadingTable(i, path, p);
            }
            else if (!dropped && noOfShortestPaths > 1)
            { // do the same for second shortest path
                path = allPaths[src][dest][1];
                if (isPathPossible(i, path, p))
                {
                    buildVcidListAndForwadingTable(i, path, p);
                }
                else
                    dropped = true;
            }
            else
                dropped = true;

            if (dropped)
                droppedConnections.insert(i);
        }
        return;
    }

    bool isPathPossible(int conId, Path &path, int p)
    {
        double capNeededP1 = connections[conId].b_max;
        double leftP0 = connections[conId].b_max;
        double rightP0 = connections[conId].b_ave + (0.35) * (connections[conId].b_max - connections[conId].b_min);
        double capNeededP0 = 0.0;
        if (leftP0 < rightP0)
            capNeededP0 = leftP0;
        else
            capNeededP0 = rightP0;

        int n = path.nodes.size();
        for (int i = 0; i < n - 1; i++)
        {
            Edge e = edges[{path.nodes[i], path.nodes[i + 1]}];
            if (VCID[e] < MAX_VCID)
            { // VCID is available

                double capNeeded = 0.0;
                if (p == 1)
                    capNeeded = capNeededP1;
                else
                    capNeeded = capNeededP0;

                if (capNeeded > ((e.capacity * 1.0) - totCapUsed[e]))
                { // Needed capacity available
                    return false;
                }
            }
            else
                return false;
        }
        return true;
    }

    void buildVcidListAndForwadingTable(int conId, Path &path, int p)
    {
        int n = path.nodes.size();
        vector<int> vcidList = {};

        if (n <= 2)
        {
            connectionVcidList[conId] = {path, vcidList};
            return;
        }

        double leftP0 = connections[conId].b_max;
        double rightP0 = connections[conId].b_ave + (0.35) * (connections[conId].b_max - connections[conId].b_min);
        double capNeededP0 = 0.0;
        if (leftP0 < rightP0)
            capNeededP0 = leftP0;
        else
            capNeededP0 = rightP0;

        for (int i = 1; i < n - 1; i++)
        {
            // populate forwaring table of router path.nodes[i]
            Edge e = edges[{path.nodes[i - 1], path.nodes[i]}];
            Edge e1 = edges[{path.nodes[i], path.nodes[i + 1]}];

            if (forwardingTable[path.nodes[i]].size() == 0)
            {
                forwardingTable[path.nodes[i]] = vector<vector<int>>();
            }

            vector<int> forwardingTableEntry = {path.nodes[i - 1], VCID[e], path.nodes[i + 1], VCID[e1]};
            forwardingTable[path.nodes[i]].push_back(forwardingTableEntry);

            vcidList.push_back(VCID[e]);
            if (i == n - 2)
            {
                vcidList.push_back(VCID[e1]);

                VCID[e1]++;
                if (p == 1)
                    totCapUsed[e1] += leftP0;
                else
                    totCapUsed[e1] += capNeededP0;
            }

            VCID[e]++;
            if (p == 1)
                totCapUsed[e] += leftP0;
            else
                totCapUsed[e] += capNeededP0;
        }

        connectionVcidList[conId] = {path, vcidList};
        return;
    }

    void allForwardingTables(const string &filename)
    {
        ofstream file(filename);

        if (!file.is_open())
        {
            cerr << "Failed to open file: " << filename << endl;
            return;
        }

        for (int i = 0; i < N; i++)
        {
            int n = forwardingTable[i].size();
            for (int j = 0; j < n; j++)
            {
                file << "Routers ID : " << i << " | ";
                file << " Node ID of Incoming port " << forwardingTable[i][j][0] << " | ";
                file << " VCID " << forwardingTable[i][j][1] << " | ";
                file << " Node ID of Outcoming port " << forwardingTable[i][j][2] << " | ";
                file << " VCID " << forwardingTable[i][j][3] << "\n";
                if (j == n - 1)
                    file << "------------------------------------------\n";
            }
        }

        file.close();
        return;
    }

    void writePaths(const string &filename)
    {
        ofstream file(filename);

        file << R << " " << R - droppedConnections.size() << "\n";

        for (int i = 0; i < R; i++)
        {
            if (!droppedConnections.count(i))
            {
                int n = connectionVcidList[i].first.nodes.size();
                int src = connectionVcidList[i].first.nodes[0];
                int dest = connectionVcidList[i].first.nodes[n - 1];

                file << "Conn. ID " << i << " | ";
                file << "Source : " << src << " | ";
                file << "Destination : " << dest << " | ";
                file << "Path : ";
                for (int j = 0; j < n; j++)
                {
                    file << connectionVcidList[i].first.nodes[j];
                    if (j != n - 1)
                        file << " -> ";
                }
                file << " | ";

                file << "VCID List : ";
                n = connectionVcidList[i].second.size();
                for (int j = 0; j < n; j++)
                {
                    file << connectionVcidList[i].second[j];
                    if (j != n - 1)
                        file << " -> ";
                }
                file << " | ";
                file << "Path cost : " << connectionVcidList[i].first.cost << "\n";
            }
        }
    }
};

int main(int argc, char *argv[])
{
    if (argc != 15)
    {
        cerr << "Usage: ./routing -top topologyfile -conn connectionsfile -rt routingtablefile -ft forwardingfile -path pathsfile -flag hop|dist -p 0|1" << endl;
        return 1;
    }

    int metricFlag, p;
    string topologyFile, connectionFile;
    string routingTableFile, forwardingFile, pathsFile;

    for (int i = 1; i < argc; i += 2)
    {
        string arg = argv[i];
        if (arg == "-top")
            topologyFile = argv[i + 1];
        else if (arg == "-conn")
            connectionFile = argv[i + 1];
        else if (arg == "-rt")
            routingTableFile = argv[i + 1];
        else if (arg == "-ft")
            forwardingFile = argv[i + 1];
        else if (arg == "-path")
            pathsFile = argv[i + 1];
        else if (arg == "-flag")
            metricFlag = (string(argv[i + 1]) == "hop");
        else if (arg == "-p")
            p = stoi(argv[i + 1]);
        else
        {
            cerr << "Input format is not correct\n";
            return 1;
        }
    }

    Router router(metricFlag, p);
    router.parseTopology(topologyFile);
    router.parseConnections(connectionFile);
    router.computeRoutingTables(routingTableFile);
    router.processConnections(p);
    router.allForwardingTables(forwardingFile);
    router.writePaths(pathsFile);

    return 0;
}
