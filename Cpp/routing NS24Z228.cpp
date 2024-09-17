#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <limits>
#include <queue>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace std;

enum class InputArgument {
    Top,
    Conn,
    Rt,
    Ft,
    Path,
    Flag,
    P,
};

struct Edge {
    int dest;
    int delay;
    int capacity;
};

struct Request {
    int src;
    int dest;
    int minBand;
    int meanBand;
    int maxBand;
};


class VCIdMgr {
public:
    VCIdMgr() : nextVCId(1) {}

    int getNextVCId() {
        return nextVCId++;
    }

private:
    unsigned int nextVCId ;
};

InputArgument stringToInputArgument(const string& cmdStr) {
    if (cmdStr == "-top") return InputArgument::Top;
    if (cmdStr == "-conn") return InputArgument::Conn;
    if (cmdStr == "-rt") return InputArgument::Rt;
    if (cmdStr == "-ft") return InputArgument::Ft;
    if (cmdStr == "-path") return InputArgument::Path;
    if (cmdStr == "-flag") return InputArgument::Flag;
    if (cmdStr == "-p") return InputArgument::P;
    return InputArgument::Flag;
}

void printHelp() {
    cerr << "Usage: ./routing -top topologyfile -conn connectionsfile -rt routingtablefile -ft forwardingfile -path pathsfile -flag hop|dist -p 0|1" << endl;
}

void parseConnectionFile(const string& filename, vector<Request>& connections) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening connections file." << endl;
        return;
    }

    int numRequests;
    file >> numRequests;
    connections.resize(numRequests);

    for (int i = 0; i < numRequests; ++i) {
        file >> connections[i].src >> connections[i].dest >> connections[i].minBand >> connections[i].meanBand >> connections[i].maxBand;
    }
    file.close();
}

void parseTopologyFile(const string& filename, vector<vector<Edge>>& graph, int& nodeCount) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening topology file." << endl;
        exit(0);
    }

    int edgeCount;
    file >> nodeCount >> edgeCount;
    graph.resize(nodeCount);

    for (int i = 0; i < edgeCount; ++i) {
        int node1, node2, delay, capacity;
        file >> node1 >> node2 >> delay >> capacity;
        graph[node1].push_back({node2, delay, capacity});
        graph[node2].push_back({node1, delay, capacity});
    }
    file.close();
}

vector<int> findShortestRoute(int src, int dest, const vector<vector<Edge>>& graph, bool useDistance) {
    int n = graph.size();
    vector<int> dist(n, numeric_limits<int>::max());
    vector<int> prev(n, -1);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> finalRoutes;

    dist[src] = 0;
    finalRoutes.push({0, src});

    while (!finalRoutes.empty()) {
        int now = finalRoutes.top().second;
        finalRoutes.pop();

        for (const Edge& edge : graph[now]) {
            int end = edge.dest;
            int weight = useDistance ? edge.delay : 1;

            if (dist[now] + weight < dist[end]) {
                dist[end] = dist[now] + weight;
                prev[end] = now;
                finalRoutes.push({dist[end], end});
            }
        }
    }

    vector<int> path;
    for (int lane = dest; lane != -1; lane = prev[lane]) {
        path.push_back(lane);
    }
    reverse(path.begin(), path.end());
    return path;
}

void findTwoShortestPaths(int src, int dest, const vector<vector<Edge>>& graph, vector<int>& path1, vector<int>& path2, bool useDistance) {
    
    path1 = findShortestRoute(src, dest, graph, useDistance);
    vector<vector<Edge>> tempGraph = graph;

    for (int i = 0; i < path1.size() - 1; ++i) {
        int itr1 = path1[i];
        int itr2 = path1[i + 1];

        vector<Edge> tempEdge;
        for (const Edge& edge : tempGraph[itr1]) {
            if (edge.dest != itr2) {
                tempEdge.push_back(edge);
            }
        }
        tempGraph[itr1] = tempEdge;

        vector<Edge> tempEdge1;
        for (const Edge& edge : tempGraph[itr2]) {
            if (edge.dest != itr1) {
                tempEdge1.push_back(edge);
            }
        }
        tempGraph[itr2] = tempEdge1;
    }
    path2 = findShortestRoute(src, dest, tempGraph, useDistance);
}

string pathToString(vector<int> path )
{
    string tempStr("");
    for( int node : path)
    {
        tempStr += to_string(node) + " ";
    }
    return tempStr;
}

string centerAlign(const string text, int width) {
    if (text.length() >= width) {
        return text.substr(0, width);
    }
    int padding = width - text.length();
    int leftPadding = padding / 2;
    int rightPadding = padding - leftPadding;
    return string(leftPadding, ' ') + text + string(rightPadding, ' ');
}

void buildRoutingTable(const vector<vector<Edge>>& graph, const string& flag, const string& routingTableFile) {
    ofstream file(routingTableFile);
    if (!file.is_open()) {
        cerr << "Error opening routing table file." << endl;
        return;
    }

    bool useDistance = (flag == "dist");
    int nodeCount = graph.size();
    file << left;
    file << "======================================================================================================================================================="<< "\n";
    file << "||        Destination Node        |                  Path from (source to dest)                  |        Path Delay        |        Path Cost        ||" << "\n";	
    file << "======================================================================================================================================================="<< "\n";
    for (int src = 0; src < nodeCount; ++src) {
        for (int dest = 0; dest < nodeCount; ++dest) {
            if (src != dest) {
                vector<int> path1, path2;
                findTwoShortestPaths(src, dest, graph, path1, path2, useDistance);

                int path1Delay = 0, path2Delay = 0;
                if (!path1.empty()) {
                    for (int i = 0; i < path1.size() - 1; ++i) {
                        for (const Edge& edge : graph[path1[i]]) {
                            if (edge.dest == path1[i + 1]) {
                                path1Delay += edge.delay;
                                break;
                            }
                        }
                    }
                }
                if (!path2.empty()) {
                    for (int i = 0; i < path2.size() - 1; ++i) {
                        for (const Edge& edge : graph[path2[i]]) {
                            if (edge.dest == path2[i + 1]) {
                                path2Delay += edge.delay;
                                break;
                            }
                        }
                    }
                }
                file << "|| "; 
		file << centerAlign(to_string(dest),31) << "|";
		file << centerAlign(pathToString(path1),42);
		file << "|";
                
		file << centerAlign(to_string(path1Delay),26) << "|" << centerAlign(to_string(path1.size()-1),25) ;
		file << "|| \n";
                file << "|| "; 
		file << centerAlign(to_string(dest),31) << "|";
		file << centerAlign(pathToString(path2),42);
		file << "|";
		file << centerAlign(to_string(path2Delay),26) << "|" << centerAlign(to_string(path2.size()-1),25) ;
		file << "|| \n";
            }
        }
    }
    file << "======================================================================================================================================================="<< "\n";
    file.close();
}

bool checkAdmission(int pathID,const Request& req, const vector<vector<int>>& paths, const vector<vector<Edge>> graph, bool pessimistic) {
    for (int i = 0; i < paths[pathID].size() - 1; ++i) {
        int src = paths[pathID][i];
        int dest = paths[pathID][i + 1];

        int totalBand = 0;
        for (const Edge& edge : graph[src]) {
            if (edge.dest == dest) {
                totalBand = edge.capacity;
                break;
            }
        }

        if (pessimistic) {
            if (req.maxBand > totalBand) {
                return false;
            }
        } else {
            int equivBand = min(req.maxBand,static_cast<int>( req.meanBand + 0.35 * (req.maxBand - req.minBand)));
            if (equivBand > totalBand) {
                return false;
            }
        }
    }
    return true;
}


void buildForwardingTable(const vector<vector<Edge>>& graph, const vector<Request>& connections, const string& forwardingFile, const string& pathsFile , bool pessimistic , bool useDistance) {
    ofstream file(forwardingFile);
    if (!file.is_open()) {
        cerr << "Error opening forwarding table file." << endl;
        exit(1);
    }
    ofstream pathFile(pathsFile);
    if (!pathFile.is_open()) {
        cerr << "Error opening path file." << endl;
        exit(1);
    }
    file << "==========================================================================================================================" << "\n";
    file << "||    This Router’s ID    |    Node ID of Incoming Port    |    VC ID    |    Node ID of Outgoing Port    |    VC ID    ||" << "\n";
    file << "==========================================================================================================================" << "\n";
    VCIdMgr vcIdMgr;
    int connId = 1;
    int admitted = 0;
    for (Request connection : connections) {
	vector<int> path1;
	vector<int> path2;
	findTwoShortestPaths(connection.src, connection.dest, graph, path1,path2,useDistance);
        vector<vector<int>> paths = {path1,path2}; 
        if (checkAdmission(0, connection, paths, graph, pessimistic)) {
            admitted++;
        } else if (checkAdmission(1, connection, paths, graph, pessimistic)) {
            admitted++;
        }
    }
    
    pathFile << "The total number of requested connections : " << connections.size() <<"\n";
    pathFile << "The total number of admitted connections  : " << admitted << "\n\n";
    pathFile << "========================================================================================================================================================================================" << "\n";
    pathFile << "||    Conn.ID    |    Source    |    Dest    |                        Path                        |                            VC ID List                            |    Path Cost    ||" << "\n";
    pathFile << "========================================================================================================================================================================================" << "\n";
    for (Request connection : connections) {
	vector<int> path1;
	vector<int> path2;
	findTwoShortestPaths(connection.src, connection.dest, graph, path1,path2,useDistance);
        vector<vector<int>> paths = {path1,path2}; 
        bool admitted = false;
        int pathID = -1;

        if (checkAdmission(0, connection, paths, graph, pessimistic)) {
            pathID = 0;
            admitted = true;
        } else if (checkAdmission(1, connection, paths, graph, pessimistic)) {
            pathID = 1;
            admitted = true;
        }

        if (admitted) {
            string pathVCs("");
            for (int i = 0; i < paths[pathID].size() - 1; i++) {
                int srcNode = paths[pathID][i];
                int destNode = paths[pathID][i + 1];
                string VC1 = to_string(vcIdMgr.getNextVCId());
                string VC2 = to_string(vcIdMgr.getNextVCId());
                pathVCs += VC1 + " " + VC2 + " ";
                file << "||"<< centerAlign(to_string(srcNode),24) << "|" << centerAlign(to_string(srcNode),32) << "|" << centerAlign(VC1,13) << "|" << centerAlign(to_string(destNode),32) << "|" << centerAlign(VC2,13) << "||\n";
            }
            pathFile << "||" << centerAlign(to_string(connId),15) << "|" << centerAlign(to_string(connection.src),14) << "|" << centerAlign(to_string(connection.dest),12) << "|"<< centerAlign(pathToString(paths[pathID]),52) << "|" << centerAlign(pathVCs,66) << "|" << centerAlign(to_string((paths[pathID]).size()),17)<<  "||\n";
        }
        connId++;
    }
    file << "==========================================================================================================================" << "\n";
    pathFile << "========================================================================================================================================================================================" << "\n";

    pathFile.close();
    file.close();
}

void buildPathsFile(const vector<Request>& connections, const vector<vector<int>>& paths, const string& pathsFile) {
    ofstream file(pathsFile);
    if (!file.is_open()) {
        cerr << "Error opening paths file." << endl;
        exit(1);
    }

    int admittedConnections = 0;
    file << connections.size() << " " << admittedConnections << "\n";

    for (int i = 0; i < connections.size(); ++i) {
        if (!paths[i].empty()) {
            file << i << " " << connections[i].src << " " << connections[i].dest << " ";
            for (int node : paths[i]) {
                file << node << " ";
            }
            file << "\n";
            ++admittedConnections;
        }
    }

    file.close();
}

int main(int argc, char* argv[]) {
    if (argc != 15) {
        printHelp();
        return 0;
    }

    string flag("");
    string topologyFile("");
    string connectionsFile("");
    string routingTableFile("");
    string forwardingFile("");
    string pathsFile("");
    int port = -1;

    for (int i = 1; i < argc; ++i) {
        string param = argv[i];
        string arg = argv[++i];

        switch (stringToInputArgument(param)) {
            case InputArgument::Top:
                topologyFile = arg;
                break;
            case InputArgument::Conn:
                connectionsFile = arg;
                break;
            case InputArgument::Rt:
                routingTableFile = arg;
                break;
            case InputArgument::Ft:
                forwardingFile = arg;
                break;
            case InputArgument::Path:
                pathsFile = arg;
                break;
            case InputArgument::Flag:
                if ((arg == "hop") || (arg == "dist")) {
                    flag = arg;
                } else {
                    printHelp();
                    return 0;
                }
                break;
            case InputArgument::P:
                if ((arg == "0") || (arg == "1")) {
                    port = stoi(arg);
                } else {
                    printHelp();
                    return 0;
                }
                break;
            default:
                printHelp();
                return 0;
        }
    }

    if (!topologyFile.empty() && !connectionsFile.empty() && !routingTableFile.empty() && !forwardingFile.empty() && !pathsFile.empty() && !flag.empty() && (port == 0 || port == 1)) {
        vector<Request> connections;
        vector<vector<Edge>> graph;
        int nodeCount = 0;
        parseTopologyFile(topologyFile, graph, nodeCount);
        parseConnectionFile(connectionsFile, connections);
	buildRoutingTable(graph,flag,routingTableFile);
	buildForwardingTable(graph,connections, forwardingFile,pathsFile,(1 == port),("dist" == flag));
    } else {
        printHelp();
    }

    return 0;
}

