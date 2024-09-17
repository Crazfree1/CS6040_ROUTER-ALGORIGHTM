#include <bits/stdc++.h>
using namespace std;
#define MAX 1e9
int global_count =0;
struct Edge {
    int node1;
    int node2;
    int propagation_delay;
    int capacity;
    int cost;
};
struct ShortestPath {
    int src;
    int dest;
    int cost;
    int delay;
    vector<int> path;
};
struct Connection {
    int id;
    int src;
    int dest;
    vector<int>path;
    vector<int>vcid_list;
    int cost;
};

int counter(){
    global_count +=1;
    return global_count;
}
pair<vector<int>,vector<int>> dijkstras(int src,int nodeCount,vector<vector<int>>adjacency_matrix) {
    vector<int>dist(nodeCount,MAX);
    vector<int>prev_hop(nodeCount,-1);
    vector<int>mark(nodeCount,0);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    dist[src]=0;
    mark[src]=1;
    pq.push(make_pair(0,src));
    while(!pq.empty())
    {
        pair<int,int> temp = pq.top();
        int weight = temp.first;
        int node = temp.second;
        pq.pop();
        mark[node] =1;
        for(int i=0;i<nodeCount;i++)
        {
            if(mark[i]!=1&&dist[i]>weight+adjacency_matrix[node][i]) {
                dist[i]=weight+adjacency_matrix[node][i];
                pq.push(make_pair(dist[i],i));
                prev_hop[i]=node;  
            }
        }
    }  
    return make_pair(dist,prev_hop);  
}
vector<ShortestPath> FirstShortestPathFromASource(int src,int nodeCount,vector<int>cost,vector<int>prev_hop,vector<vector<int>>delay_matrix) {
    vector<ShortestPath>paths;
    for (int i=0;i<nodeCount;i++)
    {
        ShortestPath spath;
        vector<int> path;
        spath.src = src;
        spath.dest = i;
        spath.cost = cost[i];
        int delay = 0;
        int node =i;
        while(node != src)
        {
            delay +=delay_matrix[prev_hop[node]][node];
            path.push_back(node);
            node = prev_hop[node];
        }
        path.push_back(src);
        spath.delay = delay;
        reverse(path.begin(), path.end());
        spath.path = path;
        paths.push_back(spath);
    }
    return paths;
}
vector<ShortestPath> SecondShortestPathFromASource(int src,int nodeCount,vector<ShortestPath>FirstShortestPath,vector<vector<int>>cost_matrix,vector<vector<int>>delay_matrix ){
    vector<int> cost(nodeCount) ,prev_hop(nodeCount);
    vector<ShortestPath>SSP(nodeCount);
    for(int i =0;i<nodeCount;i++){
        int min=MAX;
        if(i==src){continue;}
        for(int j=0;j< FirstShortestPath[i].path.size()-1;j++)
        {
            int prev=cost_matrix[FirstShortestPath[i].path[j]][FirstShortestPath[i].path[j+1]];
            cost_matrix[FirstShortestPath[i].path[j]][FirstShortestPath[i].path[j+1]]=MAX;
            pair<vector<int>,vector<int>> temp = dijkstras(src,nodeCount,cost_matrix);
            if(temp.first[i]<min)
            {
                cost = temp.first;
                prev_hop= temp.second;
                min=temp.first[i];
            }
            cost_matrix[FirstShortestPath[i].path[j]][FirstShortestPath[i].path[j+1]]=prev;
        }
        SSP[i] = FirstShortestPathFromASource(src,nodeCount,cost,prev_hop,delay_matrix)[i];
    }
    return SSP;
}
int validConnection(double required_cap,vector<int>FirstShortestPath,vector<int>SecondShortestPath,vector<vector<double>>avail_capacity_matrix) {
    int flag=0;
    // cout<<"ho"<<endl;
    for (int i =0;i<FirstShortestPath.size()-1;i++) {
        if(avail_capacity_matrix[FirstShortestPath[i]][FirstShortestPath[i+1]]<required_cap) {
            flag =1;
            // cout<<"avail "<<avail_capacity_matrix[FirstShortestPath[i]][FirstShortestPath[i+1]]<<" req "<<required_cap<<endl;
            // cout<<"LINK "<<FirstShortestPath[i]<<" "<<FirstShortestPath[i+1]<<endl;
        }
        // cout << avail_capacity_matrix[FirstShortestPath[i]][FirstShortestPath[i+1]]<<" ";
    }
    // cout<<endl;
    if(flag == 0) {
        // cout<<"return1\n";
        return 1;
    }
    // cout<<"Checking SecondSP\n";
    for (int i =0;i<SecondShortestPath.size()-1;i++) {
        if(avail_capacity_matrix[SecondShortestPath[i]][SecondShortestPath[i+1]]<required_cap) {
            flag =2;
            // cout<<" 2hi"<<endl;
            // cout<<"avail "<<avail_capacity_matrix[SecondShortestPath[i]][SecondShortestPath[i+1]]<<" req "<<required_cap<<endl;
            // cout<<"LINK "<<SecondShortestPath[i]<<" "<<SecondShortestPath[i+1]<<endl;
        }

    }
    if(flag ==1) {
        return 2;
    }
    return -1;
}
Connection setup_connection(int id,int src,int dest,double required_cap,int cost,vector<int>path,vector<vector<double>>&avail_capacity_matrix,vector<vector<int>>&vcid_matrix) {
    Connection c;
    vector<int>vcid_list;
    c.id =id;
    c.src = src;
    c.dest = dest;
    c.cost = cost;
    c.path = path;
    for(int i =0;i<path.size()-1;i++) {
        avail_capacity_matrix[path[i]][path[i+1]] -=required_cap;
        // avail_capacity_matrix[path[i+1]][path[i]] -=required_cap;
        vcid_list.push_back(vcid_matrix[path[i]][path[i+1]]);
        vcid_matrix[path[i]][path[i+1]] +=1;
        // vcid_matrix[path[i+1]][path[i]] +=1;
    }
    c.vcid_list = vcid_list;
    // cout<<"Established Connection :"<<id<<endl;
    return c;

}
int main(int argc, char* argv[]) {
    string topologyFile, connectionsFile, routingTableFile, forwardingFile,hopOrDist,pathsFile;
    int p_flag = -1;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-top" && i + 1 < argc) {
            topologyFile = argv[++i];
        } else if (arg == "-conn" && i + 1 < argc) {
            connectionsFile = argv[++i];
        } else if (arg == "-rt" && i + 1 < argc) {
            routingTableFile = argv[++i];
        } else if (arg == "-ft" && i + 1 < argc) {
            forwardingFile = argv[++i];
        } else if (arg == "-flag" && i + 1 < argc) {
            hopOrDist = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            p_flag = atoi(argv[++i]); 
        } else if (arg == "-path" && i + 1 < argc) {
            pathsFile = argv[++i];
        }
        else {
            cerr << "Unknown argument: " << arg << endl;
            return 1;
        }
    }
    ifstream file(topologyFile);

    if (!file.is_open()) {
        cerr << "Error opening file: " << topologyFile << endl;
        return 1;
    }
    int nodeCount, edgeCount;
    file >> nodeCount >> edgeCount;

    vector<Edge> edges;
    for (int i = 0; i < edgeCount; ++i) {
        Edge edge;
        file >> edge.node1 >> edge.node2 >> edge.propagation_delay >> edge.capacity;
        if(hopOrDist == "dist")
        {
            edge.cost = edge.propagation_delay; 
        }
        else if (hopOrDist == "hop")
        {
            edge.cost = 1;
        }
        edges.push_back(edge);
    }

    file.close();
    vector<vector<int>>delay_matrix(nodeCount,vector<int>(nodeCount,MAX));
    vector<vector<int>>cost_matrix(nodeCount,vector<int>(nodeCount,MAX));
    // vector<vector<int>>capacity_matrix(nodeCount,vector<int>(nodeCount,0));
    vector<vector<double>>avail_capacity_matrix(nodeCount,vector<double>(nodeCount,0));
    vector<vector<int>>vcid_matrix(nodeCount,vector<int>(nodeCount,0));
    for (const auto& edge : edges) {
            delay_matrix[edge.node1][edge.node2]=edge.propagation_delay;
            delay_matrix[edge.node2][edge.node1]=edge.propagation_delay;
            cost_matrix[edge.node1][edge.node2]=edge.cost;
            cost_matrix[edge.node2][edge.node1]=edge.cost;
            // capacity_matrix[edge.node1][edge.node2]=edge.capacity;
            // capacity_matrix[edge.node2][edge.node1]=edge.capacity;
            avail_capacity_matrix[edge.node1][edge.node2]=double(edge.capacity);
            avail_capacity_matrix[edge.node2][edge.node1]=double(edge.capacity);
        }  
    for (int i = 0 ; i< nodeCount ; i++) {
        delay_matrix[i][i]=0;
        cost_matrix[i][i]=0;
        }
    vector<vector<ShortestPath>>FirstShortestPath(nodeCount);
    for(int i = 0;i<nodeCount;i++) {  
        pair<vector<int>,vector<int>> temp = dijkstras(i,nodeCount,cost_matrix);
        vector<int> cost = temp.first;
        vector<int>prev_hop = temp.second;
        FirstShortestPath[i] = FirstShortestPathFromASource(i,nodeCount,cost,prev_hop,delay_matrix);
    }
    vector<ShortestPath>SecondShortestPath[nodeCount];
    for(int i =0;i<nodeCount;i++){
        SecondShortestPath[i] = SecondShortestPathFromASource(i,nodeCount,FirstShortestPath[i],cost_matrix,delay_matrix);

    }
    ofstream outfile(routingTableFile);
    int width = 15;
    if (outfile.is_open()) {
        for(int i=0;i<nodeCount;i++) {
            outfile << "Source Node: " << i<<endl;
            outfile << std::left<<setw(width)<<"Destination"<<setw(50)<<"Path"<<setw(15)<<"PathDelay"<<setw(10)<<"PathCost"<<endl;
            for(int j =0;j<nodeCount;j++){
                if(i==j){continue;}
                stringstream fResult;
                for (size_t ii = 0; ii < FirstShortestPath[i][j].path.size(); ++ii) {
                    fResult << FirstShortestPath[i][j].path[ii];
                    if (ii != FirstShortestPath[i][j].path.size() - 1) {
                        fResult << "->";
                    }
                }
                outfile << std::left<<setw(width)<< FirstShortestPath[i][j].dest<<setw(50)<< fResult.str()<<setw(width)<<FirstShortestPath[i][j].delay <<setw(width)<< FirstShortestPath[i][j].cost<<endl;
                stringstream sResult;
                for (size_t ii = 0; ii < SecondShortestPath[i][j].path.size(); ++ii) {
                    sResult << SecondShortestPath[i][j].path[ii];
                    if (ii != SecondShortestPath[i][j].path.size() - 1) {
                        sResult << "->";
                    }
                }
                outfile << std::left<<setw(width)<< SecondShortestPath[i][j].dest<<setw(50)<< sResult.str()<<setw(width)<<SecondShortestPath[i][j].delay <<setw(width)<< SecondShortestPath[i][j].cost<<endl;
            }
        }
        outfile.close();
    } else {
        cout << "Unable to open the file." << endl;
    }
    ifstream cfile(connectionsFile);

    if (!cfile.is_open()) {
        cerr << "Error opening file: " << connectionsFile << endl;
        return 1;
    }
    int connectionCount;
    cfile >> connectionCount;
    // cout<<"connection Count: "<<connectionCount<<endl;
    vector<Connection> ConnectionTable;
    int id;
    for (id=0;id<connectionCount;id++) {
        // int id = ConnectionTable.size();
        int src,dest;
        int b_min,b_avg,b_max;
        cfile >> src >> dest >> b_min >> b_avg >>b_max;
        // printf ("%d %d %.2f %.2f %.2f\n",src,dest,b_min,b_avg,b_max);
        double required_cap;
        if(p_flag==1) {
            required_cap = double(b_max);
        }
        else if (p_flag ==0) {
            double expression = b_avg + 0.35 * (b_max - b_min);
            double result = min(double(b_max), expression);
            required_cap = round(result * 100) / 100;
        }
        else {
            cerr<<"invalid p flag: Accepts either 0 or 1.\n";
        }
        // cout<<"required_Cap: "<<required_cap<<endl;
        // printf("Source %d dest %d\n",src,dest);
        // cout<<"Connection ID: "<<id<<endl;
        // cout<<"id "<<id<<endl;
        // if(id == 180){
        //     cout<<"src "<<src<<" dest "<<dest<<endl;
        //     cout <<FirstShortestPath[src][dest].path.size()<<endl;
        //     cout <<SecondShortestPath[src][dest].path.size()<<endl;
        // }
        switch (validConnection(required_cap,FirstShortestPath[src][dest].path,SecondShortestPath[src][dest].path,avail_capacity_matrix)){
            case 1:
                // cout<<"firstSSP \n";
                ConnectionTable.push_back(setup_connection(id,src,dest,required_cap,FirstShortestPath[src][dest].cost,FirstShortestPath[src][dest].path,avail_capacity_matrix,vcid_matrix));
                break;
            case 2:
                // cout<<"secondSSP\n";
                ConnectionTable.push_back(setup_connection(id,src,dest,required_cap,SecondShortestPath[src][dest].cost,SecondShortestPath[src][dest].path,avail_capacity_matrix,vcid_matrix));
                break;
        }
        // cout<<"for endl\n";
        // break;  
    }
    // cout<<"id "<<id<<endl;
    cfile.close();
    ofstream coutfile(pathsFile);
    if (!coutfile.is_open()) {
        cerr << "Error opening file: " << pathsFile<< endl;
        return 1;
    }
    else {
        // cout<<"hi print\n";
        coutfile << connectionCount << "," << ConnectionTable.size() << endl;
        coutfile << std::left<<setw(width)<<"Conn.ID"<<setw(15)<<"Source"<<setw(20)<<"Destination"<<setw(30)<<"Path"<<setw(20)<<"VCID list"<<"Path Cost"<<endl;
        // cout<<ConnectionTable.size()<<endl;
        for(int i=0;i<ConnectionTable.size();i++) {
            stringstream pResult,vResult;
            // pResult<<" empty";
            if(ConnectionTable[i].vcid_list.size()==0){
                    pResult<<" -";
                }
            for (size_t ii = 0; ii < ConnectionTable[i].path.size(); ++ii) {
                pResult << ConnectionTable[i].path[ii];
                if (ii != ConnectionTable[i].path.size() -1) {
                    pResult << "->";
                }
            }
            // vResult<<"empty";
            if(ConnectionTable[i].vcid_list.size()==0){
                    vResult<<" -";
                }
            for (size_t ii = 0; ii < ConnectionTable[i].vcid_list.size(); ++ii) {
                // vResult<<" empty";
                vResult << ConnectionTable[i].vcid_list[ii];
                if (ii != ConnectionTable[i].vcid_list.size() -1) {
                    vResult << ", ";
                }
            }
            coutfile << std::left<<setw(15)<<ConnectionTable[i].id<<setw(15)<<ConnectionTable[i].src<<setw(20)<<ConnectionTable[i].dest<<setw(30)<<pResult.str()<<setw(25)<<vResult.str()<<ConnectionTable[i].cost<<endl;;
        }
        coutfile.close();
    }
    ofstream foutfile(forwardingFile);
    if (!foutfile.is_open()) {
        cerr << "Error opening file: " << forwardingFile<< endl;
        return 1;
    }
    else {
        foutfile << std::left<<setw(10)<<"Router.ID"<<setw(15)<<"Inc.PortID"<<setw(10)<<"VC ID"<<setw(13)<<"Out.PortID"<<setw(15)<<"VC ID"<<endl;
        for(int i=0;i<ConnectionTable.size();i++){
            vector<int> path = ConnectionTable[i].path;
            vector<int> vcid_list = ConnectionTable[i].vcid_list;
            if(path.size()>0) {
                foutfile << "\t"<<setw(10)<<path[0] <<setw(13) <<"-"<< setw(13)<<"-"<<setw(10)<<path[1]<<setw(10)<<vcid_list[0]<<endl;
                for (int i=1;i<path.size()-1;i++) {
                    foutfile << "\t" << setw(10)<<path[i] <<setw(13) <<path[i-1]<< setw(13)<<vcid_list[i-1]<<setw(10)<<path[i+1]<<setw(10)<<vcid_list[i]<<endl;
                }
                int last = path.size() -1;
                foutfile << "\t" << setw(10)<<path[last]<<setw(13) <<path[last -1]<< setw(13)<<vcid_list[last-1]<<setw(10)<<"-"<<setw(10)<<"-"<<endl;
            }
           
        }

    }
    return 0;
}
