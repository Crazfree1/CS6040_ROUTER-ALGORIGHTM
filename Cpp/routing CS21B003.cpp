#include<bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

string flag;
map<pair<int, int>, vector<int>> shortest_map;
map<pair<int, int>, vector<int>> secShortest_map;
map<int,vector<int>> forwarding_map;
map<int, vector<int>> addedPaths_map;
map<int, vector<int>> addedVICD_map;
map<pair<int, int>, double> capacity_map;
map<pair<int, int>, int> delay_map;
vector<int> vcid_counter;
int p;
int width=30;

void print_pathsfile(int num_con,ofstream& pathsfile){
    pathsfile<<"Number of req connections "<<num_con<<" addmitted connections "<<addedPaths_map.size()<<endl;
    pathsfile<<endl;
    pathsfile<<"------------------------------------------------------------------------------------------"<<endl;
    pathsfile<<"Conn ID"<<setw(width)<<"src node"<<setw(width)<<"dst node"<<setw(width)<<"path cost"<<setw(width)<<"path"<<setw(width)<<"VCID List"<<endl;

    for(int i=0;i<num_con;i++){
        vector<int> final_path;
        vector<int> final_vcid;
        if(addedPaths_map.find(i)!=addedPaths_map.end()){
            pathsfile<<i<<setw(width);
            final_path=addedPaths_map[i];
            final_vcid=addedVICD_map[i];
            pathsfile<<final_path[1]<<setw(width);
            pathsfile<<final_path[final_path.size()-1]<<setw(width);
            pathsfile<<final_path[0]<<setw(width)<<"[ " ;
            for(int j=1;j<final_path.size();j++){
                pathsfile<<final_path[j]<<' ';
            }
            pathsfile<<"] "<<setw(width);
            pathsfile<<" [ ";
            for(int j=1;j<final_vcid.size();j++){
                pathsfile<<final_vcid[j]<<' ';
            }
            pathsfile<<"]"<<endl;
        }

    }

}
void print_FTs(int nodes,ofstream& fts_file){
    for(int i=0;i<nodes;i++){
        fts_file<<"-------Forwarding Table for node "<<i<<" --------------------------------"<<endl;
        fts_file<<"Routers ID"<<setw(width)<<"InNodeID"<<setw(width)<<"VCID"<<setw(width)<<"OutNodeID"<<setw(width)<<"VCID"<<endl;
        for(int c=0;c<forwarding_map[i].size();c++){
            int con_id=forwarding_map[i][c];
            vector<int> tfinal_path=addedPaths_map[con_id];
            vector<int> final_vcid=addedVICD_map[con_id];
            vector<int> final_path;
            for(int m=1;m<tfinal_path.size();m++){
                final_path.push_back(tfinal_path[m]);
            }
            auto it = find(final_path.begin(), final_path.end(), i);
            int index=distance(final_path.begin(), it);
            fts_file<<i<<setw(width);
            if(index-1<0){
                fts_file<<"-1"<<setw(width)<<"-1"<<setw(width);
            }
            else{
                fts_file<<final_path[index-1]<<setw(width)<<final_vcid[index-1]<<setw(width);
            }
            if(index+1>=final_path.size()){
                fts_file<<"-1"<<setw(width)<<"-1"<<endl;
            }
            else{
                fts_file<<final_path[index+1]<<setw(width)<<final_vcid[index+1]<<endl;
            }
        }
    }

}



vector<int> dijkstras_pairwise(vector<vector<int>>adj_matrix,int src,int dst){
    vector<int> path;
    int n= adj_matrix.size();
    int shortest[n];
    bool visited[n];
    int parents[n];
    for(int i=0;i<n;i++){
        shortest[i]=INT_MAX;
        visited[i]=false;
    }
    shortest[src]=0;
    parents[src]=-1;
    for(int i=0;i<n;i++){
        int pre=-1;
        int min=INT_MAX;
        for(int i=0;i<n;i++){
            if(!visited[i] && shortest[i]<min){
                pre=i;
                min=shortest[i];
            }
        }
        if(pre==-1){
            path.push_back(-1);
            return path;
        }
        visited[pre]=true;
        for(int i=0;i<n;i++){
            int dist=adj_matrix[pre][i];
            if (dist > 0 && ((min + dist) <= shortest[i]))  { 
                    parents[i] = pre; 
                    shortest[i] = min + dist; 
                } 
        }
    }
    int node=dst;
    path.push_back(shortest[dst]);
    while(node!=-1){
        path.push_back(node);
        node=parents[node];
    }
    return path;
}

void secDijkstras(vector<vector<int>>adj_matrix,int src){
    int preV=-1;
    int preS=-1;
    int preD=-1;
    int n=adj_matrix.size();
    vector<int> path;
    for(int i=0;i<n;i++){
        if(i!=src){
            int newDist=INT_MAX;
            path=shortest_map[make_pair(src,i)];
            for(int j=path.size()-1;j>=2;j--){
                int s=path[j];
                int d=path[j-1];
                if(preV!=-1){
                    adj_matrix[preS][preD]=preV;
                    adj_matrix[preD][preS]=preV;
                }
                preV=adj_matrix[s][d];
                preS=s;
                preD=d;
                adj_matrix[s][d]=0;
                adj_matrix[d][s]=0;
                vector<int> new_path=dijkstras_pairwise(adj_matrix,src,i);
                if(new_path[0]<newDist && new_path[0]!=-1){
                    newDist=new_path[0];
                    secShortest_map[make_pair(src,i)]=new_path;
                }
            }    
        }
    }
}

void dijkstras_toall(vector<vector<int>>adj_matrix,int src){
    int n = adj_matrix.size();
    vector<int> shortest(n);
    vector<bool> added(n);
    for (int vertexIndex = 0; vertexIndex < n;
         vertexIndex++) {
        shortest[vertexIndex] = INT_MAX;
        added[vertexIndex] = false;
    }
    shortest[src]=0;
    vector<int> parents(n);
    parents[src]=-1;
    for(int i=1;i<n;i++){
        int nearV=-1;
        int shorDist=INT_MAX;
        for(int j=0;j<n;j++){
            if(!added[j] && shortest[j]<shorDist){
                nearV=j;
                shorDist=shortest[j];
            }
        }
        added[nearV]=true;
        for(int j=0;j<n;j++){
            int dist = adj_matrix[nearV][j];
            if(dist!=0 && (shorDist+dist)<shortest[j]){
                parents[j]=nearV;
                shortest[j]=shorDist+dist;
            }
        }
    }
    vector<int> path;
    
    for(int i=0;i<n;i++){
        if(i!=src){
            path.clear();
            path.push_back(shortest[i]);
            int node=i;
            while(node!=-1){
                path.push_back(node);
                node=parents[node];
            }
        shortest_map[make_pair(src,i)]=path;
        }
    }
}



void print_routing(map<pair<int, int>, vector<int>> myMap,ofstream& rtFile,int nodes){
    for(int i=0;i<nodes;i++){
        rtFile<<"------------Source node is "<<i<<" below is its routing table--------------"<<endl;
        rtFile<<endl;
        vector<int> path;
        rtFile<<"dst node           "<<"path delay          "<<"path cost          "<<"path            "<<endl;
        for(int j=0;j<nodes;j++){
            if(i!=j){
                path=myMap[make_pair(i,j)];
                rtFile<<j<<"                        ";
                if(flag=="hop"){
                    rtFile<<path[0]<<"                      "<<path.size()-2<<"               ";
                }
                else{
                    rtFile<<path[0]<<"                     "<<path[0]<<"               ";
                }
                for(int v=path.size()-1;v>=1;v--){
                    rtFile<<path[v]<<' ';
                }
                rtFile<<endl;
            }
        }
        rtFile<<endl;
    }
}
class Connection{

    public:
    int id;
    int src;
    int dst;
    int b_min;
    int b_avg;
    int b_max;

    Connection(int id,int src,int dst,int b_min,int b_avg,int b_max)
    : 
    id(id),src(src),dst(dst),b_min(b_min),b_avg(b_avg),b_max(b_max){
    }

    double b_equiv(){
        if(b_max*1.0>b_avg+0.35*(b_max-b_min)){
            return b_avg+0.35*(b_max-b_min);
        }
        else{
            return b_max*1.0;
        }
        
    }
    void display() const{
        //cout<<src<<' '<<dst<<' '<<b_min<<' '<<b_avg<<' '<<b_max<<endl;
    }

    void handle() {
        double b_matters;
        display();
        if(p==0){
            b_matters=b_equiv();
        }
        else{
            b_matters=b_max*1.0;
        }
        int s=src;
        int d=dst;
        vector<int> short_path=shortest_map[make_pair(s,d)];
        vector<int> sec_short_path=secShortest_map[make_pair(s,d)];


        int admit=0;
        for(int j=1;j<short_path.size()-1;j++){
            int d1=short_path[j];
            int s1=short_path[j+1];
            if(capacity_map[make_pair(s1,d1)]<b_matters){
                admit=1;
                // return;
            }
        }
        if(admit==0){
            for(int j=1;j<short_path.size()-1;j++){
            int d1=short_path[j];
            int s1=short_path[j+1];
            capacity_map[make_pair(s1,d1)]=capacity_map[make_pair(s1,d1)]-b_matters;
            
            
            
        }
        vector<int> final_path;
        vector<int> final_vcid;
        final_path.push_back(short_path[0]);
        
        for(int j=short_path.size()-1;j>=1;j--){
            final_path.push_back(short_path[j]);
            if(forwarding_map.find(short_path[j])!=forwarding_map.end()){
                forwarding_map[short_path[j]].push_back(id);
            }
            else{
                vector<int> temp;
                temp.push_back(id);
                forwarding_map[short_path[j]]=temp;
            }
            final_vcid.push_back(vcid_counter[short_path[j]]);
            vcid_counter[short_path[j]]++;
        }
        addedPaths_map[id]=final_path;
        addedVICD_map[id]=final_vcid;
        }
        else{
            for(int j=1;j<sec_short_path.size()-1;j++){
            int d1=sec_short_path[j];
            int s1=sec_short_path[j+1];
            if(capacity_map[make_pair(s1,d1)]<b_matters){
                admit=2;
                
            }
        }
        if(admit==1){
            for(int j=1;j<sec_short_path.size()-1;j++){
            int d1=sec_short_path[j];
            int s1=sec_short_path[j+1];
            capacity_map[make_pair(s1,d1)]=capacity_map[make_pair(s1,d1)]-b_matters;
        }
        vector<int> final_path;
        vector<int> final_vcid;
        final_path.push_back(sec_short_path[0]);
        
        for(int j=sec_short_path.size()-1;j>=1;j--){
            final_path.push_back(sec_short_path[j]);
            if(forwarding_map.find(sec_short_path[j])!=forwarding_map.end()){
                forwarding_map[sec_short_path[j]].push_back(id);
            }
            else{
                vector<int> temp;
                temp.push_back(id);
                forwarding_map[sec_short_path[j]]=temp;
            }
            final_vcid.push_back(vcid_counter[sec_short_path[j]]);
            vcid_counter[sec_short_path[j]]++;
        }
        addedPaths_map[id]=final_path;
        addedVICD_map[id]=final_vcid;
        }
        }
    }
};

int main(int argc, char* argv[]){
    if (argc < 15) { 
        cerr << "Insufficient arguments provided!" << std::endl;
        return 1;
    }
    string topologyFile;
    string connectionsFile;
    string routingTableFile;
    string forwardingFile;
    string pathsFile;

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
        } else if (arg == "-path" && i + 1 < argc) {
            pathsFile = argv[++i];
        } else if (arg == "-flag" && i + 1 < argc) {
            flag = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            p = std::stoi(argv[++i]);
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }
    
    ifstream inputFile(topologyFile+".txt");
    string line;
    getline(inputFile,line);
    vector<int> numbers;
    int number;
    istringstream iss(line);
    while (iss >> number) {
         numbers.push_back(number);
    }
    int nodes=numbers[0];
    int edges=numbers[1];
    int src_top;
    int dst_top;
    int delay_top;
    int capacity_top;
    
    

    for(int i=0;i<edges;i++) {
        getline(inputFile, line);
        istringstream iss(line);
        vector<int> numbers;
        int number;
        while (iss >> number) {
         numbers.push_back(number);
        
        }
        src_top=numbers[0];
        dst_top=numbers[1];
        delay_top=numbers[2];
        capacity_top=numbers[3];
        if(flag=="hop"){
            capacity_map[make_pair(dst_top,src_top)]=capacity_top;
            capacity_map[make_pair(src_top,dst_top)]=capacity_top;
            delay_map[make_pair(dst_top,src_top)]=1;
            delay_map[make_pair(src_top,dst_top)]=1;
        }
        else{
            
            capacity_map[make_pair(dst_top,src_top)]=capacity_top;
            capacity_map[make_pair(src_top,dst_top)]=capacity_top;
            delay_map[make_pair(dst_top,src_top)]=delay_top;
            delay_map[make_pair(src_top,dst_top)]=delay_top;
            
        }
    }
    
    vector<vector<int>> adj_matrix(nodes,vector<int>(nodes, 0));
    for(auto it = delay_map.begin(); it != delay_map.end(); ++it) {
        std::pair<int, int> key = it->first;
        int value = it->second;
        adj_matrix[key.first][key.second]=value;
        adj_matrix[key.second][key.first]=value;
    }
    
    
    for(int v=0;v<nodes;v++){
        dijkstras_toall(adj_matrix,v);
        secDijkstras(adj_matrix,v);
    }

    ofstream rtFile(routingTableFile+".txt");
    print_routing(shortest_map,rtFile,nodes);
    rtFile<<"----------------------------------------SECOND SHORTEST PATHS------------------------------------------------------------"<<endl;
    print_routing(secShortest_map,rtFile,nodes);

    ifstream conFile(connectionsFile+".txt");
    getline(conFile,line);
    vector<int> Numbers;
    istringstream iss1(line);
    while (iss1 >> number) {
         Numbers.push_back(number);
    }
    int num_con=Numbers[0];
    vector<Connection> connections;
    while (getline(conFile, line)) {
        istringstream iss1(line);
        vector<int> numbers;
        int number;
        while (iss1 >> number) {
         numbers.push_back(number);
        }
        int ID=connections.size();
        connections.push_back(Connection(ID,numbers[0],numbers[1],numbers[2],numbers[3],numbers[4]));

    }

    for(int i=0;i<nodes;i++){
        vcid_counter.push_back(0);
    }

    for(int i=0;i<connections.size();i++){
        connections[i].handle();
        }
    ofstream pFile(pathsFile+".txt");
    print_pathsfile(num_con,pFile);
    ofstream fts_file(forwardingFile+".txt");
    print_FTs(nodes,fts_file);
    return 0;

}



