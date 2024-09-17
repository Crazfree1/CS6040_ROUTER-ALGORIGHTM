#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

#define INF 0x3f3f3f3f

struct route {
    vector<int> path;
    int delay;
    int capacity;
};

struct conn {
    int src;
    int dest;
    int b_min;
    int b_avg;
    int b_max;
};

struct fwdRow {
    int rouID;
    int node1;
    int VCID1;
    int node2;
    int VCID2;
    int b_max;
};

struct connPath {
    int connID;
    int src;
    int dest;
    vector<int> path;
    vector<int> VCIDList;
    int pathcost;
};

class Connection {
public:
    int numofNodes;
    int numofEdges;
    vector<vector<pair<int, int>>> edges;
    string routingTableFile;
    vector<vector<pair<route, route>>> routingTable;
    //path, propagation delay, bandwidth capcity
    vector<vector<pair<int, int>>> adjList;

    Connection(string topologyFile, string routingTableFile) : routingTableFile(routingTableFile) {
        ifstream topoFile(topologyFile);
        string line;
        int node1, node2, propagationDelay, bandwidthCapacity;
        int i = 0;
        while (getline(topoFile, line)) {
            if (i==0) {
                sscanf(line.c_str(), "%d %d", &numofNodes, &numofEdges);
                adjList.resize(numofNodes);
                edges.resize(numofNodes);
                routingTable.resize(numofNodes);
                for (int j = 0; j < numofNodes; j++) {
                    edges[j].resize(numofNodes);
                    routingTable[j].resize(numofNodes);
                }
                i+=1;
            }else {
                sscanf(line.c_str(), "%d %d %d %d", &node1, &node2, &propagationDelay, &bandwidthCapacity);
                edges[node1][node2] = {propagationDelay, bandwidthCapacity};
                edges[node2][node1] = {propagationDelay, bandwidthCapacity};
                adjList[node1].push_back({node2, propagationDelay});
                adjList[node2].push_back({node1, propagationDelay});
            }
        }
    }

    pair<vector<int>, vector<int>> dikstra(int src) {
        vector<int> dist(numofNodes, INF);
        vector<bool> added(numofNodes, false);
        vector<int> parents(numofNodes, -1);
        parents[src] = -1;
        dist[src] = 0;
        for (int i = 0; i < numofNodes; i++) {
            int nearestVertex = -1;
            int shortestDistance = INF;
            for (int vertexIndex=0; vertexIndex < numofNodes; vertexIndex++) {
                if (!added[vertexIndex] && dist[vertexIndex] < shortestDistance) {
                    nearestVertex = vertexIndex;
                    shortestDistance = dist[vertexIndex];
                }
            }
            added[nearestVertex] = true;
            for (auto neighbour : adjList[nearestVertex]) {
                int edgeDistance = neighbour.second;
                if (edgeDistance > 0 && (shortestDistance+edgeDistance) < dist[neighbour.first]) {
                    dist[neighbour.first] = shortestDistance+edgeDistance;
                    parents[neighbour.first] = nearestVertex;
                }
            }
        }
        return {parents, dist};
    }

    vector<int> printPath(int currentVertex, vector<int> parents)
    {
        if (currentVertex == -1) {
            return {};
        }
        vector<int> path1 = printPath(parents[currentVertex], parents);
        path1.push_back(currentVertex);
        return path1;
    }

    pair<int, int> findCapacity(vector<int> path) {
        int maxCapacity = INF;
        int edgeCapacity = 0;
        int delay = 0;
        for (int i=0; i<path.size()-1; i++) {
            edgeCapacity = edges[path[i]][path[i+1]].second;
            delay += edges[path[i]][path[i+1]].first;
            if(edgeCapacity < maxCapacity) {
                maxCapacity = edgeCapacity;
            }
        }
        return {maxCapacity, delay};
    }

    route shortestpath(int src, int dest, int midIndex, vector<vector<int>> parents, vector<vector<int>> dist) {
        route shortest;
        pair<int, int> tempPair;
        if (src == midIndex || dest == midIndex) {
            shortest.path = printPath(dest, parents[src]);
            tempPair = findCapacity(shortest.path);
            shortest.delay = tempPair.second;
            shortest.capacity = tempPair.first;
            return shortest;
        }
        shortest.path = printPath(midIndex, parents[src]);
        vector<int> temp = printPath(dest, parents[midIndex]);
        shortest.path.insert(shortest.path.end(), temp.begin()+1, temp.end());
        tempPair = findCapacity(shortest.path);
        shortest.delay = tempPair.second;
        shortest.capacity = tempPair.first;
        return shortest;
    }

    vector<vector<pair<route, route>>> routing() {
        vector<vector<int>> dist(numofNodes);
        vector<vector<int>> parents(numofNodes);
        for (int i = 0; i < numofNodes; i++) {
            pair<vector<int>, vector<int>> result = dikstra(i);
            parents[i] = result.first;
            dist[i] = result.second;
        }
        for (int i = 0; i < numofNodes; i++) {
            for (int j = 0; j < i; j++) {
                int firstmin = INF;
                int firstminIndex = -1;
                int secondmin = INF;
                int secondminIndex = -1;
                for (int k = 0; k < numofNodes; k++) {
                    if (dist[k][i] + dist[k][j] < secondmin) {
                        if (dist[k][i] + dist[k][j] > firstmin) {
                            secondmin = dist[k][i] + dist[k][j];
                            secondminIndex = k;
                        }else if (dist[k][i] + dist[k][j] < firstmin) {
                            if (firstmin < INF) {
                                secondmin = firstmin;
                                secondminIndex = firstminIndex;
                            }
                            firstmin = dist[k][i] + dist[k][j];
                            firstminIndex = k;
                        }
                    }
                }
                if (secondminIndex == -1) {
                    secondminIndex = firstminIndex;
                }
                // cout << i << " " << j << " " << firstminIndex << " " << secondminIndex << endl;
                routingTable[i][j] = {shortestpath(i, j, firstminIndex, parents, dist), shortestpath(i, j, secondminIndex, parents, dist)};
                routingTable[j][i] = {shortestpath(j, i, firstminIndex, parents, dist), shortestpath(j, i, secondminIndex, parents, dist)};
            }
            // cout << endl;
        }
        printRoutingFile();
        return routingTable;
    }

    void printRoutingFile() {
        ofstream routingFile(routingTableFile);
        for (int i = 0; i < numofNodes; i++) {
            for (int j = 0; j < numofNodes; j++) {
                if (i!=j){
                    routingFile << "Destination Node: " << j << " | Path: ";
                    for (auto k : routingTable[i][j].first.path) {
                        routingFile << k << " ";
                    }
                    routingFile << "| Path Delay: ";
                    routingFile << routingTable[i][j].first.delay << " | Path Cost: " << routingTable[i][j].first.path.size()-1 << endl;
                    routingFile << "Destination Node: " << j << " | Path: ";
                    for (auto k : routingTable[i][j].second.path) {
                        routingFile << k << " ";
                    }
                    routingFile << "| Path Delay: ";
                    routingFile << routingTable[i][j].second.delay << " | Path Cost: " << routingTable[i][j].second.path.size()-1 << endl;
                }
            }
        }
        routingFile.close();
    }

};

class Paths{
public:
    int flag; // 0 to dist, 1 to hop
    int p;
    int requestedConnections;
    int admittedConnections;
    Connection connection;
    vector<vector<pair<route, route>>> routingTable;
    vector<fwdRow> forwardingFile;
    vector<conn> reqConnectionList;
    vector<connPath> connPathList;

    Paths(int flag, int p, string topologyFile, string connectionsFile, string routingTableFile): connection(topologyFile, routingTableFile), flag(flag), p(p) {
        routingTable = connection.routing();
        ifstream connectionFilestr(connectionsFile);
        string line;
        conn temp;
        int start = 0;
        while (getline(connectionFilestr, line)) {
            if (start == 0) {
                requestedConnections = stoi(line);
                // cout << requestedConnections<< endl;
                start += 1;
            }else {
                // cout << line << endl;
                sscanf(line.c_str(), "%d %d %d %d %d", &temp.src, &temp.dest, &temp.b_min, &temp.b_avg, &temp.b_max);
                // cout << temp.src << temp.dest << temp.b_min << temp.b_avg << temp.b_max << endl;
                reqConnectionList.push_back(temp);
            }
        }
    }

    float min(float a, float b){
        return (a < b) ? a : b;
    }

    float findB_equ(conn reqConn){
        return min(reqConn.b_max, reqConn.b_avg+(0.35*(reqConn.b_max-reqConn.b_min)));
    }

    int checkIfFree(route path, conn reqConn) {
        int sum_b= 0;
        for (int i = 0; i < path.path.size()-1; i++) {
            if (p == 0) {
                for(auto j: connPathList) {
                    for (int k=0; k<j.path.size()-1; k++) {
                        if ((j.path[k] == path.path[i] && j.path[k+1] == path.path[i+1]) || (j.path[k] == path.path[i+1] && j.path[k+1] == path.path[i])) {
                            sum_b += findB_equ(reqConnectionList[j.connID]);
                        }
                    }
                }
                if (findB_equ(reqConn) > connection.edges[path.path[i]][path.path[i+1]].second - sum_b) {
                    return -1;
                }
            }else { //p==1
                for(auto j: connPathList) {
                    for (int k=0; k<j.path.size()-1; k++) {
                        if ((j.path[k] == path.path[i] && j.path[k+1] == path.path[i+1]) || (j.path[k] == path.path[i+1] && j.path[k+1] == path.path[i])) {
                            sum_b += reqConnectionList[j.connID].b_max;
                        }
                    }
                    if (reqConn.b_max > connection.edges[path.path[i]][path.path[i+1]].second - sum_b) {
                        return -1;
                    }
                }
            }
        }
        return 0;
    }

    int findVCID(int src, int dest) {
        int max_value = 0;
        for(auto i: forwardingFile) {
            if ((src == i.node1 && dest == i.rouID)|| (src == i.rouID && dest == i.node1)) {
                if (i.b_max > max_value) {
                    max_value = i.b_max;
                }
            }
        }
        return max_value;
    }

    void printforwardingFile(string file) {
        ofstream opfile(file);
        for (auto i: forwardingFile) {
            opfile << "Routers ID : " << i.rouID << " |  Node ID of Incoming port: " << i.node1 << " |  VCID " << i.VCID1 << " |  Node ID of Outcoming port: " << i.node2 << " |  VCID " << i.VCID2 << endl;
            // opfile << i.node1 << " " << i.node2 << " " << i.b_max << endl;
        }
        opfile.close();
    }

    vector<int> forwardingConn(conn reqConn, route path){
        fwdRow temp;
        vector<int> result;
        for (int i = 1; i < path.path.size()-1; i++) {
            temp.rouID = path.path[i];
            temp.node1 = path.path[i-1];
            temp.node2 = path.path[i+1];
            temp.VCID1 = findVCID(path.path[i-1], path.path[i]);
            temp.VCID2 = findVCID(path.path[i], path.path[i+1]);
            if (i==1) {
                result.push_back(temp.VCID1);
                result.push_back(temp.VCID2);
            }else {
                result.push_back(temp.VCID2);
            }
            if (p==0) {
                temp.b_max = findB_equ(reqConn)+0.5;
            }else {
                temp.b_max = reqConn.b_max;
            }
            forwardingFile.push_back(temp);
        }
        return result;
    }

    void establishConnection(int ID, conn reqConn, route path, vector<int> VCIDList){
        connPath temp;
        temp.connID = ID;
        temp.src = reqConn.src;
        temp.dest = reqConn.dest;
        temp.path = path.path;
        temp.VCIDList = VCIDList;
        if (flag == 0){
            temp.pathcost = path.delay;
        }else {
            temp.pathcost = path.path.size()-1;
        }
        connPathList.push_back(temp);
    }

    void findConnection(int ID, conn reqConn){
        route temp;
        vector<int> VCIDList;
        if (checkIfFree(routingTable[reqConn.src][reqConn.dest].first, reqConn) == 0) {
            temp = routingTable[reqConn.src][reqConn.dest].first;
        }else if (checkIfFree(routingTable[reqConn.src][reqConn.dest].second, reqConn) == 0){
            temp = routingTable[reqConn.src][reqConn.dest].second;
        }else {
            return;
        }
        VCIDList = forwardingConn(reqConn, temp);
        establishConnection(ID, reqConn, temp, VCIDList);
    }

    void printConnectionPaths(string file) {
        ofstream opfile(file);
        opfile << requestedConnections << " " << connPathList.size() << endl;
        for (auto i: connPathList) {
            opfile << "Conn. ID: " << i.connID << " | Source: " << i.src << " | Destination: "<< i.dest << " | Path: ";
            for (auto j: i.path) {
                opfile << j << " ";
            }
            opfile << "| VCID List: ";
            for (auto k: i.VCIDList) {
                opfile << k << " ";
            }
            opfile << "| Path cost: " << i.pathcost << endl;
        }
        opfile.close();
    }

    void connect() {
        for (int i = 0; i < reqConnectionList.size(); i++) {
            findConnection(i, reqConnectionList[i]);
        }
    }

};

int main(int argc, char *argv[]) {
    string topologyFile;
    string connectionsFile;
    string routingTable;
    string forwardingTable;
    string paths;
    int flag;
    int p;
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-top") {
            topologyFile = argv[i+1];
        }else if (string(argv[i]) == "-conn") {
            connectionsFile = argv[i+1];
        }else if (string(argv[i]) == "-rt") {
            routingTable = argv[i+1];
        }else if (string(argv[i]) == "-ft") {
            forwardingTable = argv[i+1];
        }else if (string(argv[i]) == "-path") {
            paths = argv[i+1];
        }else if (string(argv[i]) == "-flag") {
            if (string(argv[i+1]) == "dist") {
                flag = 0;//dist
            }else {
                flag = 1;//hop
            }
        }else if (string(argv[i]) == "-p") {
            if (string(argv[i+1]) == "0") {
                p = 0;
            }else {
                p = 1;
            }
        }
    }
    // cout << topologyFile << connectionsFile << routingTable << forwardingTable << paths << flag << p << endl;
    Paths solution(flag, p, topologyFile, connectionsFile, routingTable);
    solution.connect();
    solution.printConnectionPaths(paths);
    solution.printforwardingFile(forwardingTable);
    return 0;
}
