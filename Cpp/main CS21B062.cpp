#include <bits/stdc++.h>

using namespace std;

string topologyFile, connectionsFile, routingTableFile, forwardingFile, pathsFile;
string flag;
int p;
int R;

int VCIDlimit = 100000;
int admittedConnections = 0;


int getArgumentID(char* arg) {
    arg[strcspn(arg, "\n")] = 0;
    if(strcmp(arg, "-top") == 0) return 1;
    if(strcmp(arg, "-conn") == 0) return 2;
    if(strcmp(arg, "-rt") == 0) return 3;
    if(strcmp(arg, "-ft") == 0) return 4;
    if(strcmp(arg, "-path") == 0) return 5;
    if(strcmp(arg, "-flag") == 0) return 6;
    if(strcmp(arg, "-p") == 0) return 7;
    return -1;
}

int setVariables(char* argv[]) {
    for(int ii = 1; ii < 15; ii += 2) {
        int id = getArgumentID(argv[ii]);

        if(id == -1) {
            cout << "invalid arguments" << endl;
            return 1;
        }
        switch (id) {
            case 1: topologyFile = argv[ii+1]; break;
            case 2: connectionsFile = argv[ii+1]; break;
            case 3: routingTableFile = argv[ii+1]; break;
            case 4: forwardingFile = argv[ii+1]; break;
            case 5: pathsFile = argv[ii+1]; break;
            case 6: flag = argv[ii+1]; break;
            case 7: p = atoi(argv[ii+1]); break;
            default: break;
        }
    }
    return 0;
}

class Link {

    public:
        int node1, node2;
        int delay, capacity;
        int cost;
        double linkCapacityUsed;
        set<int> VCIDs;
        int lastVCID;

        Link(int node1, int node2, int delay, int capacity) {
            this->node1 = node1;
            this->node2 = node2;
            this->delay = delay;
            this->capacity = capacity;
            if(flag == "hop") {
                this->cost = 1;
            } else {
                this->cost = delay;
            }
            this->linkCapacityUsed = (double)0;
            this->lastVCID = 0;
            this->VCIDs.clear();
        }

};

int nodeCount, edgeCount;

class Node {
    public:
        int nodeID;
        map<int, Link*> links;
        vector<vector<int>> paths1;
        vector<int>path1Cost;
        vector<vector<int>> paths2;
        vector<int> path2Cost;
        vector<vector<int>> forwardingTable;
    
    Node(int nodeID) {
        this->nodeID = nodeID;
        this->paths1.resize(nodeCount);
        this->paths2.resize(nodeCount);
        this->path2Cost.resize(nodeCount);
        this->path2Cost.assign(nodeCount, INT_MAX);
        this->path1Cost.resize(nodeCount);
        this->path1Cost.assign(nodeCount, INT_MAX);
    }
    
};

vector<Node*> nodes;


class Connection {
    public:
        int connectionID;
        int src, dst;
        double bmin, bavg, bmax;
        double breq;
        vector<int> path;
        vector<int> VCIDList;
        int cost;

    Connection(int id, int src, int dst, double bmin, double bavg, double bmax) {
        this->connectionID = id; 
        this->src = src;
        this->dst = dst;
        this->bmin = (double)bmin;
        this->bavg = (double)bavg;
        this->bmax = (double)bmax;
        if(p) {
            breq = this->bmax;
        } else {
            double cons = 0.35;
            breq = min(this->bmax,(this->bavg)+(cons*((this->bmax)-(this->bmin))));
        }
    }
};

vector<Connection*> connections;

void getPath(int src, int dst, vector<int> & prev, int type = 1, int cost = INT_MAX) {
    if(dst == src) return;
    int pres = dst;
    vector<int> path;

    while(prev[pres] != -1) {
        path.push_back(pres);
        pres = prev[pres];
    }

    if(pres == src) path.push_back(pres);

    reverse(path.begin(), path.end());

    if(type == 1) {
        nodes[src]->paths1[dst] = path;
        nodes[src]->path1Cost[dst] = cost;
    } else {
        if(cost < nodes[src]->path2Cost[dst]) {
            nodes[src]->paths2[dst] = path;
            nodes[src]->path2Cost[dst] = cost;
        }
    }
}

void dijikstra(int src, int dst = -1, int excludex = -1, int excludey = -1) {

    vector<int> dist(nodeCount, INT_MAX);
    vector<int> prev(nodeCount, -1);
    vector<bool> visited(nodeCount, false);

    dist[src] = 0;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.push({0, src});

    while(!pq.empty()) {
        int u = pq.top().second;
        pq.pop();
        if(visited[u]) continue;
        visited[u] = true;

        for(auto link : nodes[u]->links) {
            int v = link.first;
            int cost = link.second->cost;

            if((u == excludex && v == excludey) || (u == excludey && v == excludex)) continue;

            if(dist[v] > dist[u] + cost) {
                dist[v] = dist[u] + cost;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    // cout << "Shortest Path from " << src << " : " << endl;
    // for(int ii = 0; ii < nodeCount; ii++) {
    //     cout << ii << " : " << dist[ii] << " || ";
    // }
    // cout << endl;

    if(excludex == -1 && excludey == -1) {
        for(int ii = 0; ii < nodeCount; ii++) {
            if(ii == src) continue;
            getPath(src, ii, prev, 1, dist[ii]);
        }
    } else {
        getPath(src,dst, prev, 2, dist[dst]);
    }

}

void routingTablePrinter(int src, int dst, vector<int> & path, ofstream & rfile, int csize) {

    if(src == dst) {
        rfile << setw(10) << dst << " | " << setw(csize*6) << " - " << " | " << setw(10) << 0 << " | " << setw(10) << 0 << endl;
    }
    else if(path.size() == 0) {
        rfile << setw(10) << dst << " | " << setw(csize*6) <<  " - " << " | " << setw(10) << "INF" << " | " << setw(10) <<  "INF" << endl;
    } else { 
        int pdelay = 0;
        int pcost = 0;
        for(int kk = 1; kk < (int)path.size(); kk++) {
            pdelay += nodes[path[kk-1]]->links[path[kk]]->delay;
            pcost += nodes[path[kk-1]]->links[path[kk]]->cost;
        }
        rfile << setw(10) << dst << " | " ;

        for(int ii = 0; ii < csize; ii++) {
            if(ii >= (int)path.size()) {
                rfile << "      ";
                continue;
            }
            rfile << " ->" << setw(3) << path[ii];
        }
        rfile << " | ";

        rfile << setw(10) <<  pdelay << " | " << setw(10) <<  pcost << endl;
    }

}

void getShortestPaths() {

    for(int ii = 0; ii < nodeCount; ii++) {
        dijikstra(ii);
    }

    for(int ii = 0; ii < nodeCount; ii++) {
        for(int jj = 0; jj < nodeCount; jj++) {
            if(ii == jj) continue;
            for(int kk = 1; kk < (int)nodes[ii]->paths1[jj].size(); kk++) {
                dijikstra(ii, jj, nodes[ii]->paths1[jj][kk-1], nodes[ii]->paths1[jj][kk]);
            }
        }
    }

    ofstream rfile(routingTableFile);


    for(int ii = 0; ii < nodeCount; ii++) {

        int sz = 0;
        for(int jj =  0; jj < nodeCount; jj++) {
            sz = max(sz, (int)nodes[ii]->paths1[jj].size());
            sz = max(sz, (int)nodes[ii]->paths2[jj].size());
        }


        rfile << "routing table for Node " << ii << " : " << endl;
        rfile << setw(10) << "dst Node" << " | ";
        rfile << setw(sz*6) << "path" << " | " ;
        rfile << setw(10) << "path delay" << " | ";
        rfile << setw(10) << "path cost" << endl;
        for(int jj = 0; jj < 45+sz*6; jj++) rfile << "-";
        rfile << endl;

        for(int jj = 0; jj < nodeCount; jj++) {
            routingTablePrinter(ii, jj, nodes[ii]->paths1[jj], rfile, sz);
            routingTablePrinter(ii, jj, nodes[ii]->paths2[jj], rfile, sz);
        }

        rfile << endl;
    }

}

bool checkPath(vector<int> & path, double req) {
    if(path.size() == 0 || path.size() == 1) return false;
    for(int ii = 1; ii < (int)path.size(); ii++) {
        Link *link = nodes[path[ii-1]]->links[path[ii]];
        if(link->linkCapacityUsed + req > (double)link->capacity) return false;
    }

    for(int ii = 1; ii < (int)path.size(); ii++) {
        Link *link = nodes[path[ii-1]]->links[path[ii]];
        link->linkCapacityUsed += req;
    }

    return true;
}

void getVCDList(Connection* connection) {
    vector<int> path = connection->path;
    vector<int> VClist;
    
    int siz =  path.size();

    if(siz <= 1) return;

    for(int ii = 1; ii < siz;  ii++) {

        Link * link = nodes[path[ii-1]]->links[path[ii]];
        int vc = link->lastVCID;

        bool flag = true;

        for(int jj = 0; jj < VCIDlimit; jj++) {
            int ind = (jj+vc)%VCIDlimit;
            if(link->VCIDs.find(ind) == link->VCIDs.end()) {
                vc = ind;
                link->lastVCID = vc+1;
                link->VCIDs.insert(vc);
                flag = false;
                break;
            }
        }
       
        if(flag) {
            cout << "Cannot find a required VCID" << endl;
            return;
        }
        VClist.push_back(vc);
    }
    connection->VCIDList = VClist;
}


void setupConnection(Connection* connection) {

    
    int src = connection-> src;
    int dst = connection->dst;

    // cout << "setting up connection for " << src << " -> " <<  dst << endl;


    if(checkPath(nodes[src]->paths1[dst], connection->breq)) {
        connection->path = nodes[src]->paths1[dst];
        connection->cost = nodes[src]->path1Cost[dst];
    } else if(checkPath(nodes[src]->paths2[dst], connection->breq)) {
        connection->path = nodes[src]->paths2[dst];
        connection->cost = nodes[src]->path2Cost[dst];
    } else {
        connection->path = vector<int>();
        connection->cost = -1;
        return;
    }

    getVCDList(connection);

    if(connection->VCIDList.size() == 0 || connection->path.size() <= 1 || connection->path.size() != connection->VCIDList.size() + 1) {
        connection->path = vector<int> ();
        connection->cost = -1;
        cout << "Not able to find VCIDs" << endl;
        return;
    }

    admittedConnections++;

    vector<int> fentry(4,-1);
    fentry[2] = connection->path[1];
    fentry[3] = connection->VCIDList[0];
    nodes[connection->path[0]]->forwardingTable.push_back(fentry);
    
    for(int ii = 1; ii < (int) connection->VCIDList.size(); ii++) {
        fentry[0] = connection->path[ii-1];
        fentry[1] = connection->VCIDList[ii-1];
        fentry[2] = connection->path[ii+1];
        fentry[3] = connection->VCIDList[ii];
        nodes[connection->path[ii]]->forwardingTable.push_back(fentry);
    }

    fentry.assign(4,-1);
    fentry[0] = connection->path[connection->path.size()-2];
    fentry[1] = connection->VCIDList.back();
    nodes[connection->path.back()]->forwardingTable.push_back(fentry);
   
}

void initiateConnections() {
    ifstream cfile(connectionsFile);

    cout << "reading from: " << connectionsFile << endl;

    if(!cfile.is_open()) {
        cout << "Error opening file" << endl;
        return;
    }

    cfile >> R;

    int src, dst;
    double bmin, bavg, bmax;

    for(int ii = 0; ii < R; ii++) {
        cfile >> src >> dst >> bmin >> bavg >> bmax;

        // cout << "Connection " << ii << " : " << src << " -> " << dst << " : " << bmin << " " << bavg << " " << bmax << endl;

        Connection *connection = new Connection(ii, src, dst, bmin, bavg, bmax);
        connections.push_back(connection);

        setupConnection(connection);
    }
}

void printPath(vector<int> &path, ofstream & pfile, int size) {

    for(int ii = 0; ii < size; ii++) {
        if(ii >= (int)path.size()) {
            pfile << "      ";
            continue;
        }
        pfile << " ->" << setw(3) << path[ii];
    }
}

void getPathsfile() {
    ofstream pfile(pathsFile);

    if(!pfile.is_open()) {
        pfile << "Error opening file: " << pathsFile  << endl;
        return;
    }

    pfile << "Requested Connections: " << R << " | Admitted Connections: " << admittedConnections << endl;
    pfile << "\n\n";  
    
    int psize =0, vsize = 0;
    for(int ii = 0; ii < R; ii++) {
        Connection* connection = connections[ii];
        if(connection->path.size() == 0 && connection->cost == -1) continue;
        psize = max(psize, (int)connection->path.size());
        vsize = max(vsize, (int)connection->VCIDList.size());
    }

    pfile <<setw(10) <<  "Conn. ID" << " | " << setw(10) << "Source" << " | " << setw(10) << "Dest" << " | "  << setw(psize*6) << "Path" << " | " << setw(vsize*6) << "VC ID List" << " | "<< setw(10) << "PathCost" << endl;
    for(int ii = 0; ii < 55 + (psize+vsize)*6; ii++) pfile << "-";
    pfile << endl;

    for(int ii = 0; ii < R; ii++) {
        Connection* connection = connections[ii];
        if(connection->path.size() == 0 && connection->cost == -1) continue;
        pfile << setw(10) <<  ii << " | " << setw(10) << connection->src << " | " << setw(10) <<  connection->dst << " | " ;
        printPath(connection->path, pfile, psize);
        pfile << " | ";
        printPath(connection->VCIDList, pfile, vsize);
        pfile << " | " << setw(10) << connection->cost << endl;
    }    

    cout << "Number of connections with path and VCID: " << admittedConnections << endl;
}

void getForwardingTable() {
    ofstream ffile(forwardingFile);

    if(!ffile.is_open()) {
        ffile << "Error opening file: " << forwardingFile << endl;
        return;
    }


    ffile <<setw(10) << "Router ID" << " | " <<  setw(8) << "In Node" << " | " << setw(8) << "VC ID" << " | " << setw(8) << "Out Node" << " | " << setw(6) << "VC ID" << endl;
    for(int ii = 0; ii < 55; ii++) ffile << "-"; ffile << endl;

    for(int ii = 0; ii < nodeCount; ii++) {
        for(int jj = 0; jj < (int)nodes[ii]->forwardingTable.size(); jj++) {
            ffile << setw(10) << ii ;
            for(int kk = 0; kk < 4; kk++) {
               ffile << " | " <<  setw(8) << nodes[ii]->forwardingTable[jj][kk] ;
            }
            ffile << endl;
        }
    }
}

int main(int argc, char * argv[]) {
    
    if(argc != 15) {
        cout << "invalid arguments" << endl;
        return 1;
    }

    setVariables(argv);

    cout << "Reading from " << topologyFile << endl;

    ifstream file(topologyFile);

    if(!file.is_open()) {
        cout << "Error opening file" << endl;
        return 1;
    }

    file >> nodeCount >> edgeCount;

    cout << "Node Count: " << nodeCount << endl;
    cout << "Edge Count: " << edgeCount << endl;

    for(int ii = 0; ii < nodeCount; ii++) {
        Node *node = new Node(ii);
        nodes.push_back(node);
    }

    int node1, node2, delay, capacity;

    for(int ii = 0; ii < edgeCount; ii++) {
        file >> node1 >> node2 >> delay >> capacity;

        Link *link1 = new Link(node1, node2, delay, capacity);
        Link *link2 = new Link(node2, node1, delay, capacity);

        nodes[node1]->links[node2] = link1;
        nodes[node2]->links[node1] = link2;

    }

    file.close();

    getShortestPaths();

    initiateConnections();

    getPathsfile();

    getForwardingTable();

    return 0;

}
