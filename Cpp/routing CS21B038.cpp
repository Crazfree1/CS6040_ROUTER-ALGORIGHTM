#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <vector>
using namespace std;

const int INF = 1e9;

struct Node {
    int idx; int cost;
    Node(int idx, int cost) : idx(idx), cost(cost) {}
    bool operator>(const Node& node) const {
        return cost > node.cost;
    }
};

struct Edge {
    int node1;
    int node2;
    int delay;
    double link_capacity;
};

struct Connection {
    int src;
    int dst;
    double b_min, b_ave, b_max;
};

struct VCID {
    int inputPort;
    int outputPort;
    int input_VCID;
    int output_VCID;
    VCID(int inputPort, int outputPort, int input_VCID, int output_VCID) : 
        inputPort(inputPort), outputPort(outputPort), input_VCID(input_VCID), output_VCID(output_VCID) {}
};

struct Path {
    int delay;
    int cost;
    vector<int> nodes;
    Path(int delay, int cost, vector<int> nodes) : delay(delay), cost(cost), nodes(nodes) {}
};

vector<Edge> topologyFileParsing(int& N, const string& filename) {
    ifstream infile(filename);
    vector<Edge> edges;
    int E;
    if(infile.is_open()) {
        infile >> N >> E;
        Edge e;
        Edge e2;
        for(int i = 0; i < E; i++) {
            infile >> e.node1 >> e.node2 >> e.delay >> e.link_capacity;
            e2.node1 = e.node2;
            e2.node2 = e.node1;
            e2.delay = e.delay;
            e2.link_capacity = e.link_capacity;
            edges.push_back(e);
            edges.push_back(e2);
        }
        infile.close();
    }
    return edges;
}

vector<Connection> connectionsFileParsing(const string& filename) {
    ifstream infile(filename);
    vector<Connection> connections;
    if(infile.is_open()) {
        int R;
        infile >> R;
        Connection c;
        for(int i = 0; i < R; i++) {
            infile >> c.src >> c.dst >> c.b_min >> c.b_ave >> c.b_max;
            connections.push_back(c);
        }
        infile.close();
    }
    return connections;
}

map<int, vector<int> > findShortestPaths(int N, int src, const string& flag, const vector<Edge>& edges) {
    vector<vector<pair<int, int> > > adj(N);
    vector<int> dist(N, INF);
    vector<int> prev(N, -1);
    map<int, vector<int> > paths;

    for(const Edge& e: edges) {
        int edge_weight = (flag == "dist") ? e.delay: 1;
        adj[e.node1].push_back(make_pair(e.node2, edge_weight));
        adj[e.node2].push_back(make_pair(e.node1, edge_weight));
    }

    dist[src] = 0;
    priority_queue<Node, vector<Node>, greater<Node> > pq;
    pq.push(Node(src, 0));

    while(!pq.empty()) {
        Node cur = pq.top();
        pq.pop();
        for(auto i: adj[cur.idx]) {
            if(dist[i.first] > dist[cur.idx] + i.second) {
                dist[i.first] = dist[cur.idx] + i.second;
                prev[i.first] = cur.idx;
                pq.push(Node(i.first, dist[i.first]));
            }
        }   
    }

    for(int i = 0; i < N; i++) {
        int node = i;
        while(node != -1) {
            paths[i].push_back(node);
            node = prev[node];
        }
        reverse(paths[i].begin(), paths[i].end());
    }

    return paths;
}

map<int, vector<int> > findSecondShortestPaths(int N, int src, const string& flag, const vector<Edge>& edges) {
    auto firstPaths = findShortestPaths(N, src, flag, edges);
    map<int, vector<int> > secondShortestPaths;
    for(int ii = 0; ii < N; ii++) {
        if(ii == src) continue;
        auto firstPath = firstPaths[ii];

        int minSecondPathCost = INF;
        vector<int> secondShortestPath; // from src to i

        for(int i = 0; i < firstPath.size() - 1; i++) {
            vector<Edge> newEdges;
            for(const Edge& e: edges) {
                if(!(e.node1 == firstPath[i] && e.node2 == firstPath[i + 1]) && 
                    !(e.node1 == firstPath[i + 1] && e.node2 == firstPath[i])) {
                       newEdges.push_back(e); 
                }
            }

            auto newPaths = findShortestPaths(N, src, flag, newEdges);
            auto newPath = newPaths[ii];
            int newPathCost = 0;

            for(int j = 0; j < newPath.size() - 1; ++j) {
                for(const Edge& e : edges) {
                    if(e.node1 == newPath[j] && e.node2 == newPath[j + 1]) {
                        newPathCost += (flag == "dist") ? e.delay : 1;
                    } else if(e.node1 == newPath[j + 1] && e.node2 == newPath[j]) {
                        newPathCost += (flag == "dist") ? e.delay : 1;
                    }
                }
            }

            if(newPathCost < minSecondPathCost) {
                minSecondPathCost = newPathCost;
                secondShortestPath = newPath;
            }
        }
        secondShortestPaths[ii] = secondShortestPath;
    }
    return secondShortestPaths;
}

void generateRoutingTable(int N, string flag, const vector<Edge>& edges, const string& filename) {
    map<int, int> linkDelays;
    for(const Edge& e: edges) {
        int key = min(e.node1, e.node2) * 1000 + max(e.node1, e.node2);
        linkDelays[key] = e.delay;
    }

    ofstream routingTableFile(filename);
    for(int i = 0; i < N; i++) {
        routingTableFile << "Source Node: " << i << endl;
        routingTableFile << left << setw(15) << "Destination" 
         << setw(50) << "Path" 
         << setw(10) << "Delay" 
         << setw(10) << "Cost" << endl;

        auto shortestPaths = findShortestPaths(N, i, flag, edges);
        auto secondShortestPaths = findSecondShortestPaths(N, i, flag, edges);

        for (int j = 0; j < N; j++) {
            if(j == i) continue;
            routingTableFile << left << setw(15) << j;
            int pathDelay = 0;
            int pathCost = 0;
            
            string pathString = "";
            for (int k = 0; k < shortestPaths[j].size(); k++) {
                if(k > 0) {
                    pathString += "-> ";
                }
                int node = shortestPaths[j][k];
                pathString += to_string(node) + " ";

                if (k > 0) {
                    int prev_node = shortestPaths[j][k - 1];
                    int linkKey = std::min(prev_node, node) * 1000 + std::max(prev_node, node);
                    pathDelay += linkDelays[linkKey];
                    pathCost += (flag == "dist") ? linkDelays[linkKey] : 1;
                }
            }
            routingTableFile << left << setw(50) << pathString << setw(10) << pathDelay << setw(10) << pathCost << "\n";

            routingTableFile << left << setw(15) << j;
            pathDelay = 0;
            pathCost = 0;
            
            pathString = "";
            for (int k = 0; k < secondShortestPaths[j].size(); k++) {
                if(k > 0) {
                    pathString += "-> ";
                }
                int node = secondShortestPaths[j][k];
                pathString += to_string(node) + " ";

                if (k > 0) {
                    int prev_node = secondShortestPaths[j][k - 1];
                    int linkKey = std::min(prev_node, node) * 1000 + std::max(prev_node, node);
                    pathDelay += linkDelays[linkKey];
                    pathCost += (flag == "dist") ? linkDelays[linkKey] : 1;
                }
            }
            routingTableFile << left << setw(50) << pathString << setw(10) << pathDelay << setw(10) << pathCost << "\n";
        }
        routingTableFile << endl;
    }
}

bool tryConnection(const Connection& connection, const Path& path, const vector<Edge>& edges, map<pair<int, int>, double>& linkCapacities, string p) {
    double b_equiv = min(connection.b_max, connection.b_ave + 0.35 * (connection.b_max - connection.b_min));
    double b_max = connection.b_max;

    for(int i = 0; i < path.nodes.size() - 1; i++) {
        int node1 = path.nodes[i];
        int node2 = path.nodes[i + 1];
        if(p == "0") {
            if(!(b_equiv <= linkCapacities[make_pair(node1, node2)])) {
                return false;
            }
        } else if(p == "1") {
            if(!(b_max <= linkCapacities[make_pair(node1, node2)])) {
                return false;
            }
        }
    }
    return true;
}



map<int, vector<VCID> > processConnections(int N, const vector<Edge>& edges, const vector<Connection>& connections, 
                            const string& flag, const string& p, const string& pathsfile) {
    map<pair<int, int>, double> linkCapacities;
    map<int, vector<VCID> > forwardingTable;
    for(const Edge& e: edges) {
        linkCapacities[make_pair(e.node1, e.node2)] = e.link_capacity;
    }

    map<pair<int, int>, int> link_vcid;
    ofstream pathFile(pathsfile);
    pathFile << left << setw(10) << "Conn. ID" << 
                        setw(10) << "Source" << 
                        setw(10) << "Dest" << 
                        setw(60) << "Path" << 
                        setw(20) << "VC ID List" << 
                        setw(10) << "PathCost" << endl;
    int admittedCount = 0;
    int rejectedCount = 0;

    for(int i = 0; i < connections.size(); i++) {
        const Connection& connection = connections[i];
        vector<int> firstPath = findShortestPaths(N, connection.src, flag, edges)[connection.dst];
        vector<int> secondPath = findSecondShortestPaths(N, connection.src, flag, edges)[connection.dst];
        Path path1 = Path(0, 0, firstPath);
        Path path2 = Path(0, 0, secondPath);

        for(int j = 0; j < path1.nodes.size() - 1; j++) {
            for(const Edge& e: edges) {
                if(e.node1 == path1.nodes[j] && e.node2 == path1.nodes[j + 1]) {
                    path1.delay += e.delay;
                    path1.cost += (flag == "hop") ? 1 : e.delay;
                } 
            }
        }

        for(int j = 0; j < path2.nodes.size() - 1; j++) {
            for(const Edge& e: edges) {
                if(e.node1 == path2.nodes[j] && e.node2 == path2.nodes[j + 1]) {
                    path2.delay += e.delay;
                    path2.cost += (flag == "hop") ? 1 : e.delay;
                }
            }
        }

        if (tryConnection(connection, path1, edges, linkCapacities, p)) {
            admittedCount++;
            string pathString = "";
            string vcid_list = "";

            for (int j = 0; j < path1.nodes.size() - 1; j++) {
                pathString += to_string(path1.nodes[j]) + " -> ";
                int input_vcid = -1;
                int output_vcid = -1;
                int input_port = -1;
                int output_port = -1;
                if(j > 0) {
                    input_vcid = link_vcid[make_pair(path1.nodes[j - 1], path1.nodes[j])];
                    link_vcid[make_pair(path1.nodes[j - 1], path1.nodes[j])] += 1;
                } 
                output_vcid = link_vcid[make_pair(path1.nodes[j], path1.nodes[j + 1])];
                vcid_list += to_string(output_vcid) + " ";

                link_vcid[make_pair(path1.nodes[j], path1.nodes[j + 1])] += 1;

                if(j > 0) {
                    input_port = path1.nodes[j - 1];
                }
                output_port = path1.nodes[j + 1];

                forwardingTable[path1.nodes[j]].push_back(VCID(input_port, output_port, input_vcid, output_vcid));
                int node1 = path1.nodes[j];
                int node2 = path1.nodes[j + 1];

                double b_equiv = min(connection.b_max, connection.b_ave + 0.35 * (connection.b_max - connection.b_min));
                linkCapacities[make_pair(node1, node2)] -= (p == "0") ? b_equiv : connection.b_max;
            }
            pathString += to_string(path1.nodes[path1.nodes.size() - 1]);
            pathFile << left << setw(10) << i << 
                        setw(10) << connection.src << 
                        setw(10) << connection.dst << 
                        setw(60) << pathString << 
                        setw(20) << vcid_list << 
                        setw(10) << path1.cost << endl;
        }
        else if (tryConnection(connection, path2, edges, linkCapacities, p)) {
            admittedCount++;
            string pathString = "";
            string vcid_list = "";

            for (int j = 0; j < path2.nodes.size() - 1; j++) {
                pathString += to_string(path2.nodes[j]) + " -> ";
                int input_vcid = -1;
                int output_vcid = -1;
                int input_port = -1;
                int output_port = -1;
                if(j > 0) {
                    input_vcid = link_vcid[make_pair(path2.nodes[j - 1], path2.nodes[j])];
                    link_vcid[make_pair(path2.nodes[j - 1], path2.nodes[j])] += 1;
                } 
                output_vcid = link_vcid[make_pair(path2.nodes[j], path2.nodes[j + 1])];
                vcid_list += to_string(output_vcid) + " ";
                link_vcid[make_pair(path2.nodes[j], path2.nodes[j + 1])] += 1;

                if(j > 0) {
                    input_port = path2.nodes[j - 1];
                }
                output_port = path2.nodes[j + 1];

                forwardingTable[path2.nodes[j]].push_back(VCID(input_port, output_port, input_vcid, output_vcid));
                int node1 = path2.nodes[j];
                int node2 = path2.nodes[j + 1];
                int linkKey = min(node1, node2) * 1000 + max(node1, node2);

                double b_equiv = min(connection.b_max, connection.b_ave + 0.35 * (connection.b_max - connection.b_min));
                linkCapacities[make_pair(node1, node2)] -= (p == "0") ? b_equiv : connection.b_max;
            }
            pathString += to_string(path2.nodes[path2.nodes.size() - 1]);
            pathFile << left << setw(10) << i << 
                        setw(10) << connection.src << 
                        setw(10) << connection.dst << 
                        setw(60) << pathString << 
                        setw(20) << vcid_list << 
                        setw(10) << path2.cost << endl;
        }
        else {
            rejectedCount++;
        }
    }
    // cout << connections.size() << " " << admittedCount << " " << 1.0 * (connections.size() - admittedCount) / connections.size() << endl;
    pathFile << "\n";
    pathFile << "No of Connection Requests: " << connections.size() << "\nAdmitted Requests: " << admittedCount << endl;
    return forwardingTable;
}

int main(int argc, char* argv[]) {
    if (argc != 15) {
        std::cerr << "Usage ./routing -top topologyfile -conn connectionsfile -rt routingtablefile "
                  << "-ft forwardingfile -path pathsfile -flag hop|dist -p 0|1" << endl;
        return 1;
    }
    map<string, string> args;
    for(int i = 1; i <= 13; i += 2) {
        args[argv[i]] = argv[i + 1];
    }

    string topologyfile = args["-top"];
    string connectionsfile = args["-conn"];
    string routingtablefile = args["-rt"];
    string forwardingfile = args["-ft"];
    string pathsfile = args["-path"];
    string flag = args["-flag"];
    string p = args["-p"];

    int N;
    vector<Edge> edges = topologyFileParsing(N, topologyfile);
    vector<Connection> connections = connectionsFileParsing(connectionsfile);

    generateRoutingTable(N, flag, edges, routingtablefile);
    map<int, vector<VCID> > forwardingTable = processConnections(N, edges, connections, flag, p, pathsfile);
    
    ofstream forwardingFile(forwardingfile);
    forwardingFile << left << setw(20) << "This Router’s ID " 
        << setw(40) << "Node ID of Incoming Port" 
        << setw(20) << "VC ID" 
        << setw(40) << "Node ID of Outgoing Port"
        << setw(20) << "VC ID" << endl;
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < forwardingTable[i].size(); j++) {
            forwardingFile << left << setw(20) << i 
            << setw(40) << (forwardingTable[i][j].inputPort == -1 ? "-" : to_string(forwardingTable[i][j].inputPort))
            << setw(20) << (forwardingTable[i][j].input_VCID == -1 ? "-" : to_string(forwardingTable[i][j].input_VCID))
            << setw(40) << forwardingTable[i][j].outputPort
            << setw(20) << forwardingTable[i][j].output_VCID << endl;
        }
    }
    return 0;
} 