import ast
import uuid
import random
import sys


VCID_Arr = []
def get_new_VCID():
    temp_VCID = random.randint(0,9999)
    while(temp_VCID in VCID_Arr):
        temp_VCID = random.randint(0,sys.maxsize)
    # temp_VCID = uuid.uuid4()
    return temp_VCID

def update_forwading_table(routerId,incomingRouterId,vcid1,outgoingRouterId,vcid2,mode):
    with open('forwadingfile.txt', mode) as forwading_file:
        forwading_file.write(f"{routerId} {incomingRouterId} {vcid1} {outgoingRouterId} {vcid2}\n")

def get_avaialble_BW(topoFile,x,y):
    with open (topoFile,'r') as topoloFile:
            topoloFile.readline()
            availBWFlag = False
            availBW = 0

            for topo in topoloFile:
                temp_ = topo.split()
                sNode = int(temp_[0])
                dNode = int(temp_[1])
                aBW  = float(temp_[3])

                if (x == sNode and y == dNode) or (x == dNode and y == sNode):
                    availBWFlag = True
                    availBW = aBW
                    break
    return availBWFlag,availBW

def update_topology_BW(topo_file,n1,n2,updated_BW):
    with open(topo_file,'r') as readTopoFile:
        links = readTopoFile.readlines()

    with open(topo_file,'w') as writeTopoFile:
        for link in links:
            temp = link.split()
            if( (len(temp)== 4 ) and ( (int(temp[0])== n1 and int(temp[1])== n2) or (int(temp[0])== n2 and int(temp[1])== n1) ) )  :
                
                temp[3] = str(updated_BW)
            writeTopoFile.write(' '.join(temp)+'\n')

def write_path_file(pathFile,data,req,accepted):
    with open("pathfile.txt",'w') as pFile:
        pFile.write(f"{req}  {accepted}\n")
        for item in data:
            accepted_connection = f"{item.get('ConnecID', '')} {item.get('src', '')} {item.get('dest', '')} {item.get('path', '')} {item.get('VCIDlist', '')} {item.get('pathCost', '')}\n"
            pFile.write(accepted_connection)


def admission_control_4_connection (topoFile,connectionFile, rTableFile,filePath, connApproach):
    
    with open(connectionFile,'r') as connReq:
        numberOfIncomingRequest = connReq.readline().strip()
        numberOfAcceptedRequest = 0
        connectionId = 0
        dataForAcceptedConnection = []
        
        for connectionRequest in connReq:
            connectionId = connectionId + 1
            temp = connectionRequest.split()
            src = int(temp[0])
            dest = int(temp[1])
            minBW = float(temp[2])
            avgBW = float(temp[3])
            maxBW = float(temp[4])

            print(src," ",dest," ",minBW," ",avgBW," ",maxBW)

            equivalentBW = 0
            if (connApproach == "optimistic"):
                equivalentBW = min(maxBW,(avgBW + 0.35 * (maxBW-minBW)))
            else:
                equivalentBW = maxBW
            
            pathFoundFlag = 0
            # routing table we will search till we get a path
            with open(rTableFile,'r') as rtFile:
                rtFile.readline()
                for path in rtFile:
                    t = path.split()
                    d = int(t[0])
                    p =  ' '.join(t[1:-2])
                    pArr = ast.literal_eval(p)
                    pDelay = float(t[-2])
                    pCost = float(t[-1])
                    #print(d," ",p," ",pDelay," ",pCost," ")
                    VCID_List = []
                    if(dest == d):
                        if(src == pArr[0]):
                            
                            # traversing through obtained path array
                            connectCtr = 0
                            for i in range(len(pArr)-1):
                                x = pArr[i]
                                y = pArr[i+1]

                                availBWFlag,availBW = get_avaialble_BW(topoFile,x,y)
                                if(availBWFlag == True and equivalentBW <= availBW):
                                    connectCtr = connectCtr + 1
                                else:
                                    print("connection is rejected")
                                    break

                            if(connectCtr == (len(pArr)-1)):
                                print("incoming request accepted  ",pArr[0])
                                pathFoundFlag = 1
                                numberOfAcceptedRequest = numberOfAcceptedRequest + 1

                                for j in range(len(pArr)-1):
                                    
                                    x = pArr[j]
                                    y = pArr[j+1]
                                    bwFalg, bwAvaialble = get_avaialble_BW(topoFile,x,y)
                                    updateBW = round((bwAvaialble-equivalentBW),1)
                                    update_topology_BW(topoFile,x,y,updateBW)

                                    mode = ''
                                    if(numberOfAcceptedRequest == 1):
                                        mode ='a'
                                    else:
                                        mode = 'a'
                                    if(j==0):
                                        v2 = get_new_VCID()
                                        update_forwading_table(x,"Nil","Nil",y,v2,mode)
                                        VCID_List.append(v2)
                                    elif (j == len(pArr)-1):
                                        v1 = get_new_VCID()
                                        update_forwading_table(x,pArr[j-1],v1,"Nil","Nil",mode)
                                        VCID_List.append(v1)
                                    else:
                                        v1 = get_new_VCID()
                                        v2 = get_new_VCID()
                                        VCID_List.append(v1)
                                        VCID_List.append(v2)
                                        update_forwading_table(x,pArr[j-1],v1,y,v2,mode)
                                    dataForAcceptedConnection.append({"ConnecID": connectionId-1,
                                                        "src": src,
                                                        "dest": dest,
                                                        "path":pArr,
                                                        "VCIDlist":VCID_List,
                                                        "pathCost":pCost
                                                        })   
                            # break to make sure it only establish connection for one path
                            break
                        
                        
                    
        #print("no of accepted conenction",dataForAcceptedConnection)
        write_path_file(filePath,dataForAcceptedConnection,numberOfIncomingRequest,numberOfAcceptedRequest)




                                


                
