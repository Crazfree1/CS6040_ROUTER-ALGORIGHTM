import sys
from routing import build_graph_with_topo_file
from routing import build_routing_table
from connectionFilegenerator import request_generator
from connectionEstablishment import admission_control_4_connection

def main():
    pathMetric          =   ""
    connectionMethod    =   ""
    topoFile            =   sys.argv[2]
    connecFile          =   sys.argv[4]
    routingFile         =   sys.argv[6]
    flag_               =   sys.argv[12]
    filePath            =   sys.argv[8]
    connMethodFlag      =   sys.argv[14]

    #print("topoFile>> ",topoFile)
    #print("connectionFile>> ",connecFile)
    #print("routingFile>> ",routingFile)
    #print("Flag>> ",flag_)
    #print("file_path>> ",filePath)
    #print(connectionMethod)
   
    if (flag_ == "hop"):
        pathMetric = "hop"
    elif(flag_ == "dist"):
        pathMetric = "distance"

    if (connMethodFlag == 0):
        connectionMethod = "optimistic"
    else:
        connectionMethod = "pessimistic"

    G = build_graph_with_topo_file(topoFile)
    r_table = build_routing_table(G,len(G),pathMetric)
    #print(routingFile)

    """with open(routingFile, 'w') as rt_file:
        for src in r_table:
            for dest in r_table[src]:
                path_info = r_table[src][dest]
                rt_file.write(f"Destination: {dest},  1ˢᵗ path: {path_info['Path_1']},  1ˢᵗ path_delay: {path_info['Path_1_delay']}ms,  1ˢᵗ path_cost: {path_info['Path_1_cost']}, ")
                if path_info['Path_2']:
                    rt_file.write(f" 2ⁿᵈ path : {path_info['Path_2']}, 2ⁿᵈ path_delay: {path_info['Path_2_delay']}ms, 2ⁿᵈ path_cost: {path_info['Path_2_cost']}\n")
                else:
                    rt_file.write("Second Path: None, Second Path Delay: None, Second Path Cost: None\n")
   """
    
    with open(routingFile, 'w') as r_file:
        r_file.write("Destination Node, Path(src2dest), Path Delay, Path Cost\n")
        for source, path_metadata in r_table.items():
            for dest, path_info in path_metadata.items():
                #print(f"Destination: {dest}")
                #print(f"Path Info: {path_info}")
                if(len(path_info['Path_1']) > 1):
                    r_file.write(f"{dest}  {path_info['Path_1']}  "f"{path_info['Path_1_delay']}  "f"{path_info['Path_1_cost']}\n")
                if(len(path_info['Path_2']) > 1):
                 r_file.write(f"{dest}  {path_info['Path_2']}  "f" {path_info['Path_2_delay']} "f" {path_info['Path_2_cost']}\n")

    # uncomment the below line if you want to generate the connection request.
    #request_generator(connecFile,100,6,3,6)
    admission_control_4_connection(topoFile,connecFile,routingFile,filePath,connectionMethod)

if __name__ == "__main__":
    main()

    
