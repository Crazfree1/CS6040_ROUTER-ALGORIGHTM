#include <vector>
#include <string>
#include <map>
#include <utility>
#include <tuple>
#include <fstream>
#include <sstream>
#include <iostream>
#include <climits>
#include <algorithm>
#include <iomanip>

struct rt_entry {
  int priority;
  int source;
  int dest;
  std::vector<int> path;
  int delay;
  int cost;
public:
  rt_entry(int pr, int s, int d, std::vector<int> p, int de, int c): priority(pr), source(s), dest(d), path(p), delay(de), cost(c) {}
  
  void print(std::ofstream& rtfile) {
    int nocc = 80 - 4*path.size();
    
    rtfile << std::left;
    rtfile << std::setw(8)  << dest;
    for (int v : path)
      rtfile << std::setw(4) << v;
    rtfile << std::setw(nocc) << ' ';
    rtfile << std::setw(8)  << delay;
    rtfile << std::setw(8)  << cost;
    rtfile << '\n';
  }
};

struct conn {
  int conn_id;
  int source;
  int dest;
  std::vector<int> path;
  std::vector<int> vcids;
  int delay;
  int cost;
public:
  conn(int conn_id, int source, int dest, std::vector<int> path, std::vector<int> vcids, int delay, int cost): conn_id(conn_id), source(source), dest(dest), path(path), vcids(vcids), delay(delay), cost(cost) {}
  void print(std::ofstream& pathfile) {
    int nocc = 80 - 4*path.size();
    int noccc = 80 - 4*path.size();

    pathfile << std::left;
    pathfile << std::setw(8) << conn_id;
    pathfile << std::setw(8) << source ;
    pathfile << std::setw(8) << dest   ;
    for (int v : path)
      pathfile << std::setw(4) << v ;
    pathfile << std::setw(nocc) << ' ';
    for (int v : vcids)
      pathfile << std::setw(4) << v ;
    pathfile << std::setw(noccc) << ' ';
    pathfile << std::setw(8) << cost ;
    pathfile << '\n';
  }
};

struct ft_entry {
  int node_id;
  std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> rq;
public:
  ft_entry(int e): node_id(e) {}

  void mark(std::pair<int, int> i, std::pair<int, int> o) {rq.push_back(std::make_pair(i, o));}
  void print(std::ofstream& ftfile) {
    ftfile << std::left;
    for (auto [inc, out] : rq) {
      ftfile << std::setw(8) << node_id ;
      auto [vtx1, vcid1] = inc;
      auto [vtx2, vcid2] = out;
      ftfile << std::setw(8) << vtx1 ;
      ftfile << std::setw(8) << vcid1;
      ftfile << std::setw(8) << vtx2 ;
      ftfile << std::setw(8) << vcid2;
      ftfile << '\n';
    }
  }
};

std::tuple<std::vector<int>, int, int> dijkstra(int** w, int u, int v, int n, bool flag) {
  int* dist = new int[n];
  int* spt  = new int[n];
  bool* vst = new bool[n];
  for (int ijk = 0; ijk < n; ++ijk) {
    dist[ijk] = INT_MAX;
    spt[ijk]  = -1;
    vst[ijk]  = false;
  }
  dist[u] = 0;
  
  for (int i = 0; i < n - 1; ++i) {
    int u, temp{INT_MAX};
    for (int r = 0; r < n; ++r) 
      if (!vst[r] && dist[r] < temp) {
	temp = dist[r];
	u = r;
      }
    vst[u] = true;
    for(int v = 0; v < n; ++v) 
      if (!vst[v] && (w[u][v] != INT_MAX) && (dist[u] != INT_MAX) && (dist[u] + w[u][v] < dist[v])) {
	dist[v] = dist[u] + w[u][v];
	spt[v] = u;
      }
  }
  
  int dest_dist = dist[v];
  std::vector<int> path;
  while (spt[v] != -1) {
    path.push_back(v);
    v = spt[v];
  }
  path.push_back(u);

  std::reverse(path.begin(), path.end());
  
  delete[] dist;
  delete[] spt;
  
  if (flag == 1) return std::make_tuple(path, dest_dist, dest_dist);
  else return std::make_tuple(path, dest_dist, path.size() - 1);
}


//GLOBAL VARS
std::vector<rt_entry> global_rt_table;
std::vector<conn>     global_path_table;
std::vector<ft_entry> global_ft_table;


int main(int argc, char** argv) {
  std::vector<std::string> params;
  params.assign(argv + 1, argv + argc);

  bool flag, p;
  
  if (params[10] == "-flag") {
    if (params[11] == "hop") flag = 0;
    else if (params[11] == "dist") flag = 1;
    else throw std::invalid_argument(params[11]);
  }
  else throw std::invalid_argument(params[10]);

  if (params[12] == "-p") {
    if (params[13] == "0") p = 0;
    else if (params[13] == "1") p = 1;
    else throw std::invalid_argument(params[13]);
  }
  else throw std::invalid_argument(params[12]);

  //PROCESS TOPOFILE
  std::ifstream topo;
  if (params[0] == "-top") {
    topo.open(params[1]);
    if (!topo) {
      std::cout << "FAILED to open topologyfile.\n";
      return 1;
    }
  }
  else throw std::invalid_argument(params[0]);
  
  int n, m;
  topo >> n >> m;
  int M{m};

  int** wt = new int*[n];
  for (int i = 0; i < n; ++i) 
    wt[i] = new int[n];
  int** cp = new int*[n];
  for (int i = 0; i < n; ++i) 
    cp[i] = new int[n];
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      wt[i][j] = INT_MAX;
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      cp[i][j] = 0;
  
  while(M--) {
    int source, dest;
    int temp_wt, temp_cp;
    topo >> source >> dest >> temp_wt >> temp_cp;
    wt[source][dest] = wt[dest][source] = temp_wt;
    cp[source][dest] = cp[dest][source] = 1024*temp_cp;
  }
  topo.close();

  
  //PROCESSING
  for (int u = 0; u < n; ++u) {
    for (int v= u + 1; v < n; ++v) {
      
      std::tuple<std::vector<int>, int, int> tp_1 = dijkstra(wt, u, v, n, flag);
      std::vector<int> sp = std::get<0>(tp_1);
      int delay           = std::get<1>(tp_1);
      int cost            = std::get<2>(tp_1);

      std::vector<int> ssp;
      int sdelay;
      int scost{INT_MAX};
      
      for (int i = 0; i < sp.size() - 1; ++i) {
	int u_rm = sp.at(i);
	int v_rm = sp.at(i+1);
	
	int** wt_new = new int*[n];
	for (int i = 0; i < n; ++i) 
	  wt_new[i] = new int[n];
	for (int i =0; i < n; ++i)
	  for (int j = 0; j < n; ++j)
	    wt_new[i][j] = wt[i][j];

	wt_new[u_rm][v_rm] = wt_new[v_rm][u_rm] = INT_MAX;
	std::tuple<std::vector<int>, int, int> tp_2 = dijkstra(wt_new, u, v, n, flag);
	std::vector<int> sfsp = std::get<0>(tp_2);
	int sfdelay           = std::get<1>(tp_2);
	int sfcost            = std::get<2>(tp_2);

	if (sfcost < scost) {
	  scost = sfcost;
	  sdelay = sfdelay;
	  ssp = sfsp;
	}

	for (int k = 0; k < n; ++k) {
	  delete[] wt_new[k];
	}
	delete[] wt_new;
      }
      
      rt_entry et1(1, u, v, sp, delay, cost);
      rt_entry et2(2, u, v, ssp, sdelay, scost);
      std::reverse(sp.begin(), sp.end());
      std::reverse(ssp.begin(), ssp.end());
      rt_entry et3(1, v, u, sp, delay, cost);
      rt_entry et4(2, v, u, ssp, sdelay, scost);

      global_rt_table.push_back(et1);
      global_rt_table.push_back(et2);
      global_rt_table.push_back(et3);
      global_rt_table.push_back(et4);
    }
  }

  for (int i = 0; i < n; ++i) 
    delete[] wt[i];
  delete[] wt;
  

  std::ofstream rt;
  if (params[4] == "-rt") {  
    rt.open(params[5]);
    if (!rt) {
      std::cout << "FAILED to open routingtablefile.\n";
      return 1;
    }
  }
  else throw std::invalid_argument(params[4]);

  for (rt_entry entry : global_rt_table)
    entry.print(rt);
  rt.close();
  

  //PROCESS CONNFILE AND SETUP CONNS
  std::ifstream Conn;
  if (params[2] == "-conn") {  
    Conn.open(params[3]);
    if (!Conn) {
      std::cout << "FAILED to open connectionfile.\n";
      return 1;
    }
  }
  else throw std::invalid_argument(params[2]);
  
  int conn_admitted{0}, conn_addressed{0};
  static int id = 1000;
  int src, dst, b_1, b_2, b_3;

  for (int k = 0; k < n; ++k)
    global_ft_table.emplace_back(k);


  while (Conn >> src >> dst >> b_1 >> b_2 >> b_3) { 
    for (rt_entry entry : global_rt_table) {
      if (entry.source == src && entry.dest == dst && entry.priority == 1) {
	
	int b_eq;
	if (p == 0) {
	  int v_1 = b_3;
	  int v_2 = b_2 + 0.35*(b_3 - b_1);
	  b_eq =  std::min(v_1,v_2);
	}
	else b_eq = b_3;
	
	bool check{true};
	for (int i = 0; i < entry.path.size() - 1; ++i) {
	  int u = entry.path.at(i);
	  int v = entry.path.at(i+1);
	  if (cp[u][v] < b_eq)
	    check = false;
	}
	
	if (check) {
	  std::vector<int> VIDS;
	  for(int i = 0; i < entry.path.size() - 1; ++i) {
	    int u = entry.path.at(i);
	    int v = entry.path.at(i+1);
	    cp[u][v] -= b_eq;
            cp[v][u] -= b_eq;
	    VIDS.push_back(id++);
	  }
	  ///////////////////////////////////////////////////////////////////////////
	  conn conn_entry(id++, src, dst, entry.path, VIDS, entry.delay, entry.cost);
	  global_path_table.push_back(conn_entry);
	  ///////////////////////////////////////////////////////////////////////////
	  //first
	  int vtx_i_f = entry.path.at(0);
	  int vtx_j_f = entry.path.at(1);
	  global_ft_table.at(vtx_i_f).mark(std::make_pair(-1,-1), std::make_pair(vtx_j_f, VIDS.at(0)));
	  //
	  for (int i = 1; i < entry.path.size() - 1; ++i) {
	    int vtx_h = entry.path.at(i-1);
	    int vtx_i = entry.path.at(i);
	    int vtx_j = entry.path.at(i+1);
	    global_ft_table.at(vtx_i).mark(std::make_pair(vtx_h, VIDS.at(i-1)), std::make_pair(vtx_j, VIDS.at(i)));
	  }
	  //last
	  int rank = entry.path.size() - 1;
	  int vtx_h_l = entry.path.at(rank-1);
	  int vtx_i_l = entry.path.at(rank);
	  global_ft_table.at(vtx_i_l).mark(std::make_pair(vtx_h_l, VIDS.at(rank-1)), std::make_pair(-1,-1));
	  ///////////////////////////////////////////////////////////////////////////
	  ++conn_admitted;
          ++conn_addressed;
	}
      }
      else if (entry.source == src && entry.dest == dst && entry.priority == 2) {
	int b_eq;
	if (p == 0) {
	  int v_1 = b_3;
	  int v_2 = b_2 + 0.35*(b_3 - b_1);
	  b_eq =  std::min(v_1,v_2);
	}
	else b_eq = b_3;
	bool check{true};
	for (int i = 0; i < entry.path.size() - 1; ++i) {
	  int u = entry.path.at(i);
	  int v = entry.path.at(i+1);
	  if (cp[u][v] < b_eq)
	    check = false;
	}
	if (check) {
	  std::vector<int> VIDS;
	  for(int i = 0; i < entry.path.size() - 1; ++i) {
	    int u = entry.path.at(i);
	    int v = entry.path.at(i+1);
	    cp[u][v] -= b_eq;
	    VIDS.push_back(id++);
	  }
	  ///////////////////////////////////////////////////////////////////////////
	  conn conn_entry(id++, src, dst, entry.path, VIDS, entry.delay, entry.cost);
	  global_path_table.push_back(conn_entry);
	  ///////////////////////////////////////////////////////////////////////////
	  //first
	  int vtx_i_f = entry.path.at(0);
	  int vtx_j_f = entry.path.at(1);
	  global_ft_table.at(vtx_i_f).mark(std::make_pair(-1,-1), std::make_pair(vtx_j_f, VIDS.at(0)));
	  //
	  for (int i = 1; i < entry.path.size() - 1; ++i) {
	    int vtx_h = entry.path.at(i-1);
	    int vtx_i = entry.path.at(i);
	    int vtx_j = entry.path.at(i+1);
	    global_ft_table.at(vtx_i).mark(std::make_pair(vtx_h, VIDS.at(i-1)), std::make_pair(vtx_j, VIDS.at(i)));
	  }
	  //last
	  int rank = entry.path.size() - 1;
	  int vtx_h_l = entry.path.at(rank-1);
	  int vtx_i_l = entry.path.at(rank);
	  global_ft_table.at(vtx_i_l).mark(std::make_pair(vtx_h_l, VIDS.at(rank-1)), std::make_pair(-1,-1));
	  ///////////////////////////////////////////////////////////////////////////
	  ++conn_admitted, ++conn_addressed;
	}
      }
      else ++conn_addressed;
    }
  }
  Conn.close();
  for(int k = 0; k < n; ++k) 
    delete[] cp[k];
  delete cp;
  
  std::ofstream path;
  if (params[8] == "-path") {  
    path.open(params[9]);
    if (!path) {
      std::cout << "FAILED to open pathfile.\n";
      return 1;
    }
  }
  else throw std::invalid_argument(params[8]);

  path << std::setw(8) << conn_addressed/global_rt_table.size() ;
  path << std::setw(8) << conn_admitted ;
  path << '\n';
  for (conn entry : global_path_table) 
    entry.print(path);
  path.close();

  std::ofstream ft;
  if (params[6] == "-ft") {  
    ft.open(params[7]);
    if (!ft) {
      std::cout << "FAILED to open forwardingtablefile.\n";
      return 1;
    }
  }
  else throw std::invalid_argument(params[6]);
  
  for (ft_entry entry : global_ft_table)
    entry.print(ft);
  ft.close();
  /***********************************************************************************************/

  return 0;
}
