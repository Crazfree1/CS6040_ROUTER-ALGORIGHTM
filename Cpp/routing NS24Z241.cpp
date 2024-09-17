#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <iomanip>

using namespace std;

/* To Indicate the MAX value */
const int INFINITY = numeric_limits<int>::max(); 

struct nodeDetailsInfo {
    int endPoint1, endPoint2;
    int propagationDelay;     /* Propagation Delay in milliseconds	*/
    int capacity;  	      /* Link capacity in Mbps			*/
    int totalCapacityUsed;    /* Total Used capacity in Mbps		*/
    vector<int> connections;  /* Connections using this link		*/
};

struct Connection {
    int routerId;
    int source, destination;
    int minimumBandwidth;
    int averageBandwidth;
    int maximumBandwidth; 
    /* Calculated equivalent bandwidth */
    double biequiv;
};

struct Path {
    int cost;               
    vector<int> nodes;      
    vector<int> vcids;      
};

struct Node {
    int routerId, cost;           
    bool operator>(const Node& other) const {
        return cost > other.cost;
    }
};

/*******************************************************************************************************************/
/* Function Declaration */  
void ReadTopologyFile(const string& inputTopologyFile, int& totalNodeCnt, vector<vector<nodeDetailsInfo>>& nodeList);
void ReadConnectionsFile(const string& connectionsFile, vector<Connection>& connections);
Path RouteAlgorithm(const vector<vector<nodeDetailsInfo>>& nodeInfo, int source, int destination, const string& metric);
void findTwoShortestPaths(vector<vector<nodeDetailsInfo>>& nodeInfo, int source, int destination, const string& metric, vector<Path>& paths);
bool admitConnectionOptimistic(nodeDetailsInfo& link, double biequiv);
bool admitConnectionPessimistic(nodeDetailsInfo& link, double maximumBandwidth);
bool setConnection(vector<vector<nodeDetailsInfo>>& nodeInfo, Path& path, Connection& connection, int approach);
void PopulateRouteTbl(const string& routingTableFile, const vector<Path>& paths, int totalNodeCnt);
void PopulateFwdTbl(const string& fwdFile, const vector<vector<nodeDetailsInfo>>& nodeInfo);
void PopulatePathTbl(const string& pathsFile, const vector<Connection>& connections, const vector<Path>& paths);
void processConnections(vector<vector<nodeDetailsInfo>>& nodeInfo, vector<Connection>& connections, vector<Path>& paths, int p);
/*********************************************************************************************************************/

int main(int argc, char* argv[]) 
{

    if (argc != 15) {
        cerr << "Usage: ./routing -top topologyfile -conn connectionsfile -rt routingtablefile "
                "-ft forwardingfile -path pathsfile -flag hop|dist -p 0|1" << endl;
        return 1;
    }
    
    int totalNodeCnt;
    string flag;
    vector<vector<nodeDetailsInfo>> nodeList;
    /* If there is no any input default optimistic approach */
    int p = 0; 
    vector<Connection> connections;
    string inputTopologyFile, connectionsFile, routingTableFile, fwdFile, pathsFile;

    for (int index = 1; index < argc; ++index) 
    {
        string arg = argv[index];
        if (arg == "-top") 
	{
            inputTopologyFile = argv[index + 1];
        } else if (arg == "-conn") 
	{
            connectionsFile = argv[index + 1];
        } else if (arg == "-rt") 
	{
            routingTableFile = argv[index + 1];
        } else if (arg == "-ft") 
	{
            fwdFile = argv[index + 1];
        } else if (arg == "-path") 
	{
            pathsFile = argv[index + 1];
        } else if (arg == "-flag") 
	{
            flag = argv[index + 1];
        } else if (arg == "-p") 
	{
            p = stoi(argv[index + 1]);
        }
    }

    /* Read Input Topology file */
    ReadTopologyFile(inputTopologyFile, totalNodeCnt, nodeList);

    /* Read the connections information from file */
    ReadConnectionsFile(connectionsFile, connections);

    vector<Path> paths;
    for (int src = 0; src < totalNodeCnt; ++src) {
        for (int dest = 0; dest < totalNodeCnt; ++dest) {
            if (src != dest) {
                findTwoShortestPaths(nodeList, src, dest, flag, paths);
            }
        }
    }

    processConnections(nodeList, connections, paths, p);

    PopulateRouteTbl(routingTableFile, paths, totalNodeCnt);
    PopulateFwdTbl(fwdFile, nodeList);

    return 0;
}

void ReadTopologyFile(const string& inputTopologyFile, int& totalNodeCnt, vector<vector<nodeDetailsInfo>>& nodeList) 
{
    /* Fetch Topology file */	
    ifstream topoFile(inputTopologyFile);
    if (!topoFile) {
        cerr << "Error opening topology file: " << inputTopologyFile << endl;
        exit(1);
    }

    int totalEdgeCount;
    topoFile >> totalNodeCnt >> totalEdgeCount;
    nodeList.resize(totalNodeCnt);

    for (int i = 0; i < totalEdgeCount; ++i) {
        nodeDetailsInfo link;
        topoFile >> link.endPoint1 >> link.endPoint2 >> link.propagationDelay >> link.capacity;
        link.totalCapacityUsed = 0;
        nodeList[link.endPoint1].push_back(link);
        nodeList[link.endPoint2].push_back({link.endPoint2, link.endPoint1, link.propagationDelay, link.capacity, 0, {}});
    }

    cout << "Reading Topology completedt. NodeCount: " << totalNodeCnt << ", EdgeCount: " << totalEdgeCount << endl;
}

void ReadConnectionsFile(const string& connectionsFile, vector<Connection>& connections) 
{
    ifstream file(connectionsFile);
    if (!file) 
    {
        cerr << "Error opening connections file: " << connectionsFile << endl;
        exit(1);
    }

    int requestCount;
    file >> requestCount;
    connections.resize(requestCount);

    for (int index = 0; index < requestCount; ++index) 
    {
        file >> connections[index].source >> connections[index].destination 
             >> connections[index].minimumBandwidth >> connections[index].averageBandwidth >> connections[index].maximumBandwidth;
        connections[index].routerId = index;
	
	/* To Find the min Bandwidth value */
	int bandWidth = (connections[index].averageBandwidth + 0.35 * (connections[index].maximumBandwidth - connections[index].minimumBandwidth));
	if(bandWidth < connections[index].maximumBandwidth)
		connections[index].biequiv = bandWidth;
	else
		connections[index].biequiv = connections[index].maximumBandwidth;

    }

    cout << "Read Connections file. Number of Connection Requests: " << requestCount << endl;
}

Path RouteAlgorithm(const vector<vector<nodeDetailsInfo>>& nodeInfo, int source, int destination, const string& metric) 
{
    int totalNodeCnt = nodeInfo.size();
    /* Dynamic array to store the distances */
    vector<int> dist(totalNodeCnt, INFINITY);
    /* Vector to store the previous nodes */
    vector<int> prev(totalNodeCnt, -1);
    priority_queue<Node, vector<Node>, greater<Node>> pq;
    dist[source] = 0;
    pq.push({source, 0});

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        if (current.routerId == destination) break; /* exit if we find the destination router id */

        for (const nodeDetailsInfo& link : nodeInfo[current.routerId]) {
            int cost = (metric == "hop") ? 1 : link.propagationDelay;
            int newDist = current.cost + cost;
            if (newDist < dist[link.endPoint2]) {
                dist[link.endPoint2] = newDist;
                prev[link.endPoint2] = current.routerId;
                pq.push({link.endPoint2, newDist});
            }
        }
    }

    Path path;
    path.cost = dist[destination];
    if (path.cost == INFINITY) return path;

    for (int at = destination; at != -1; at = prev[at]) {
        path.nodes.push_back(at);
    }
    reverse(path.nodes.begin(), path.nodes.end());
    return path;
}

void findTwoShortestPaths(vector<vector<nodeDetailsInfo>>& nodeInfo, int source, int destination, const string& metric, vector<Path>& paths) {
    /* The first shortest path */
    Path firstPath = RouteAlgorithm(nodeInfo, source, destination, metric);
    if (firstPath.cost == INFINITY)
    {
	    return;
    }
    paths.push_back(firstPath);

    /* Modify the graph to find an alternative path */
    for (size_t index = 0; index < firstPath.nodes.size() - 1; ++index) {
        int u = firstPath.nodes[index];
        int v = firstPath.nodes[index + 1];
        for (nodeDetailsInfo& link : nodeInfo[u]) {
            if (link.endPoint2 == v) {
                int originalDelay = link.propagationDelay;
                link.propagationDelay = INFINITY; 
                Path secondPath = RouteAlgorithm(nodeInfo, source, destination, metric);
                if (secondPath.cost < INFINITY) paths.push_back(secondPath);
                link.propagationDelay = originalDelay;
                break;
            }
        }
    }
}

bool admitConnectionOptimistic(nodeDetailsInfo& link, double biequiv) {
    double totalBequiv = 0;
    for (int conn : link.connections) {
        totalBequiv += biequiv; 
    }
    return (biequiv <= (link.capacity - totalBequiv));
}

bool admitConnectionPessimistic(nodeDetailsInfo& link, double maximumBandwidth) {
    double totalBimax = 0;
    for (int conn : link.connections) {
	/* Assume maximumBandwidth is precomputed for each connection */
        totalBimax += maximumBandwidth; 
    }
    return (maximumBandwidth <= (link.capacity - totalBimax));
}

bool setConnection(vector<vector<nodeDetailsInfo>>& nodeInfo, Path& path, Connection& connection, int approach) {
    bool canAdmit = true;

    for (size_t index = 0; index < path.nodes.size() - 1; ++index) {
        int u = path.nodes[index];
        int v = path.nodes[index + 1];

        for (nodeDetailsInfo& link : nodeInfo[u]) {
            if (link.endPoint2 == v) {
                if (approach == 0) { /* Optimistic approach */
                    if (!admitConnectionOptimistic(link, connection.biequiv)) {
                        canAdmit = false;
                    }
                } else { /* Pessimistic approach */
                    if (!admitConnectionPessimistic(link, connection.maximumBandwidth)) {
                        canAdmit = false;
                    }
                }

                if (canAdmit) {
                    link.totalCapacityUsed += connection.biequiv;
                    link.connections.push_back(connection.routerId);
                    path.vcids.push_back(rand() % 1000); 
                  
                } else {
                    return false;
                }
            }
        }
    }

    return true;
}

void PopulateRouteTbl(const string& routingTableFile, const vector<Path>& paths, int totalNodeCnt) {
    ofstream rtTbl(routingTableFile);
    if (!rtTbl) {
        cerr << "Error opening routing table file: " << routingTableFile << endl;
        return;
    }

#ifndef MUTHU
    vector<string> topLine = {" Destination Node        ", 
	    		      " Path (from Source to Dest)", 
			      " Path Delay", "Path Cost"};
    const size_t clmWide = 25;
    const char line = '=';
    const char endVal = '+';
    const char vertStep = '|';
    size_t fullWide = topLine.size() * (clmWide + 1) + 1;

    auto putLine = [&rtTbl, fullWide, line, endVal]() 
    {
        rtTbl << endVal;
        for (size_t i = 0; i < fullWide - 2; ++i) {
            rtTbl << line;
        }
        rtTbl << endVal << endl;
    };
    
    putLine();
    rtTbl << vertStep;
    for (const auto& header : topLine) {
        rtTbl << setw(clmWide) << left << header << vertStep;
    }
    rtTbl << endl;
    putLine();
#endif  

    
    
    	size_t tmpClmWide = 0;
    for (int node = 0; node < totalNodeCnt; ++node) {
        rtTbl << "Node " << node << " Routing Table:" << endl;
        for (const Path& path : paths) {
    	tmpClmWide = 0;
            if (path.nodes.front() == node) 
	    {
                rtTbl << vertStep << setw(clmWide) << path.nodes.back() << vertStep;
                for (int pNode : path.nodes) {
                    rtTbl << pNode << " ";
		    tmpClmWide++;
                }

                rtTbl << setw(clmWide-tmpClmWide) << vertStep << setw(clmWide) << path.cost << vertStep << setw(clmWide) << path.cost << vertStep << endl;
            }
        }
        rtTbl << endl;
    }
    rtTbl.close();
}

void PopulateFwdTbl(const string& fwdFile, const vector<vector<nodeDetailsInfo>>& nodeInfo) 
{
    ofstream fwdTbl(fwdFile);
    if (!fwdTbl) {
        cerr << "Error opening forwarding table file: " << fwdFile << endl;
        return;
    }
#ifndef MUTHU
    vector<string> topLine = {" This Router’s ID        ", 
	    		      " Node ID of Incoming Port", 
			      "        VC ID    ", "Node ID of Outgoing Port", " VC ID"};
    const size_t clmWide = 25;
    const char line = '=';
    const char endVal = '+';
    const char vertStep = '|';
    size_t fullWide = topLine.size() * (clmWide + 1) + 1;
    
    auto putLine = [&fwdTbl, fullWide, line, endVal]() 
    {
        fwdTbl << endVal;
        for (size_t i = 0; i < fullWide - 2; ++i) {
            fwdTbl << line;
        }
        fwdTbl << endVal << endl;
    };
    
    putLine();
    fwdTbl << vertStep;
    for (const auto& header : topLine) {
        fwdTbl << setw(clmWide) << left << header << vertStep;
    }
    fwdTbl << endl;
    putLine();

#endif  

    for (int node = 0; node < nodeInfo.size(); ++node) {
        for (const nodeDetailsInfo& link : nodeInfo[node]) 
	{
            fwdTbl << vertStep << setw(clmWide) << node << vertStep << setw(clmWide) << node << vertStep << setw(clmWide) << rand() % 1000 << vertStep << setw(clmWide) << link.endPoint2 << vertStep << setw(clmWide) << rand() % 1000 << vertStep << endl;
        }
    }
    putLine();

    fwdTbl.close();
}

void PopulatePathTbl(const string& pathsFile, const vector<Connection>& connections, const vector<Path>& paths) {
    ofstream pathFile(pathsFile);
    if (!pathFile) {
        cerr << "Error opening paths file: " << pathsFile << endl;
        return;
    }

    int admittedCount = 0;

#ifndef MUTHU
    vector<string> topLine = {" Conn. ID       ", 
	    		      " Source         ", 
			      " Destination    ", 
			      " Path           ", 
			      " VC ID List     ",
    			      " PathCost       "};
    const size_t clmWide = 25;
    const char line = '=';
    const char endVal = '+';
    const char vertStep = '|';
    size_t fullWide = topLine.size() * (clmWide + 1) + 1;
    
    auto putLine = [&pathFile, fullWide, line, endVal]() 
    {
        pathFile << endVal;
        for (size_t i = 0; i < fullWide - 2; ++i) {
            pathFile << line;
        }
        pathFile << endVal << endl;
    };
    
    putLine();
    pathFile << vertStep;
    for (const auto& header : topLine) {
        pathFile << setw(clmWide) << left << header << vertStep;
    }
    pathFile << endl;
    putLine();
#endif 

    pathFile <<"Connection Size " << connections.size() << endl;

    for (const Connection& conn : connections) {
        /* Assuming path is selected and VCIDs are assigned for each connection */
	/* Placeholder for actual path selection */
	pathFile << vertStep << setw(clmWide) << conn.routerId << vertStep << setw(clmWide) << conn.source << vertStep << setw(clmWide) << conn.destination << vertStep << setw(clmWide);

        for (const Path& path : paths){
        for (int node : path.nodes) {
            pathFile << node << " ";
        }
        pathFile << "VCIDs ";
        for (int vc : path.vcids) {
            pathFile << vc << " ";
        }
        pathFile << vertStep << setw(clmWide) << path.cost << endl;
	}
    }

    pathFile.close();
}

void processConnections(vector<vector<nodeDetailsInfo>>& nodeInfo, vector<Connection>& connections, vector<Path>& paths, int p) {
    int admittedCount = 0;

    for (Connection& conn : connections) {
        bool admitted = false;

        for (Path& path : paths) {
            if (setConnection(nodeInfo, path, conn, p)) {
                admitted = true;
                ++admittedCount;
                break;
            }
        }

        if (!admitted) {
            cout << "Connection " << conn.routerId << " could not be admitted." << endl;
        }
    }

    PopulatePathTbl("paths.txt", connections, paths);
}







