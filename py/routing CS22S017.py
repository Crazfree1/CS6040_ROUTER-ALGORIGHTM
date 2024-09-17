#!/bin/bash

import argparse         #To read command line arguments
import numpy as np      #To convert list to array
import math

# #import connProcessing from connectionsProcessing
# import connectionsProcessing

def get_args():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-top',"--top", type=str, default="ARPANET-Topo.txt",
                        help="Topology file which containes details of nodes and connections in network")
    parser.add_argument('-conn',"--conn", type=str, default="connectionsfile.txt",
                        help="Connections file which contains details of 'R' Connection requests, 'R' value will be in first line of this file.")
    parser.add_argument('-rt',"--rt", type=str, default="routingtablefile.txt",
                        help="Routing table file which contains routing table information for all the nodes.")
    parser.add_argument('-ft',"--ft", type=str, default="forwardingfile",
                        help="Forwarding file which contains forwarding table information for all the nodes.")
    parser.add_argument('-path',"--path", type=str, default="pathsfile.txt",
                        help="Name of output file that contails path details for each admitted connections in network")
    parser.add_argument('-flag',"--flag", type=str, default="hop",
                        help="Choose approach to calculate Shortest path (case sensitive): 'hop' for No. of hop count as metric or 'dist' for shortest propogation delay path.")
    parser.add_argument('-p',"--p", type=int, default=0,
                        help="Choose 1 for Pessimistic Approach, 0 for Optimistic Approach.")
    return parser.parse_args()

args = get_args()

#used starting from readFiles
nodeCount = 0
edgeCount = 0
links = []
connRequests = []
#npLinks = np.array([])
#used starting from calcPaths

#calcPath onwards
dictNetGraph = {}   #dictionary variable that holds each node with an edge as key and all its neighboring nodes with link as its value
allPaths = []       #List to store all paths found in graph
allPathsSrcDest = []    #List to store all paths found between src and dest in graph

#routing hop based onwards
minCostPathsHops = []       #List to hold 2 min hop cost paths for all possible Source and Destination
minCostPathsLinks = []       #List to hold 2 min link cost paths for all possible Source and Destination
minCostAtHops = []      #List to hold 2 costs @hops per each srcToDst
minCostAtLinks = []     #List to hold 2 costs @links per each srcToDst
pathDelaysAtHops = []   #List to hold path delay for selected paths at hop based
pathDelaysAtLinks = []   #List to hold path delay for selected paths at link based

#ConnProcessing onwards
connAdmitted = 0
connRej = 0 
linkCapaAvail = []   #To store link capacity after admitting few connections
dictAssignedConn = {}   #Dictionary to hold list of assigned connections- Indexing- 2to5 => 2*n+5 holds all connID admitted.
connIDsAdmitted = []
success = 1
#After connAdmit
fwdFileList = []
pathsFileList = []
VCIDcontroller = 274 #Global stamp
admittedVCID = []

#Function to read the Topology and Connection request files
def readFiles():
    # Open the topology file for reading
    with open(args.top, 'r') as topoFile:
        global nodeCount
        global edgeCount
        global links
        global npLinks
        global npConnRequests
        line = topoFile.readline()
        #fields = line.split('\t')
        fields = line.split()
        nodeCount = fields[0]
        edgeCount = fields[1]
        # for i in range(edgeCount):
        #     line2 = topoFile.readline()
        #     fields2 = line.split('\t')
        #     links.append(fields2) 
        for line in topoFile:
        # Strip any leading/trailing whitespace and split the line into values
            values = line.strip().split()
        
            # Convert values to integers and store in a list
            integers = [int(value) for value in values]
        
            # Append the list of integers to the main data list
            links.append(integers)
        npLinks = np.array(links)

    with open(args.conn, 'r') as connFile:
        line = connFile.readline()
        fields = line.split()
        global numConnReq
        numConnReq = fields[0]
        for line in connFile:
            global connRequests
            values = line.strip().split()
            integers = [int(value) for value in values]
            #integers = [str(value) for value in values]
            connRequests.append(integers)
        npConnRequests = np.array(connRequests)

#Function to create dictionary for the given Network Graph
def createDictForNet():    
    for i in range(int(edgeCount)):
        if str(npLinks[i][0]) not in dictNetGraph:
            dictNetGraph[str(npLinks[i][0])] = []
        if str(npLinks[i][1]) not in dictNetGraph:
            dictNetGraph[str(npLinks[i][1])] = []
        
        # Since the graph is undirected, add both nodes to each other's adjacency list
        dictNetGraph[str(npLinks[i][0])].append(str(npLinks[i][1]))
        dictNetGraph[str(npLinks[i][1])].append(str(npLinks[i][0]))   
    #print(dictNetGraph)        #Tested, Dict is proper

    #For integer calculations
    # for i in range(int(edgeCount)):
    #     if npLinks[i][0] not in dictNetGraph:
    #         dictNetGraph[npLinks[i][0]] = []
    #     if npLinks[i][1] not in dictNetGraph:
    #         dictNetGraph[npLinks[i][1]] = []
        
    #     # Since the graph is undirected, add both nodes to each other's adjacency list
    #     dictNetGraph[npLinks[i][0]].append(npLinks[i][1])
    #     dictNetGraph[npLinks[i][1]].append(npLinks[i][0])   
    # #print(dictNetGraph)        #Tested, Dict is proper

def dfsAllPathsSrcToDest(sourceNode,destinationNode,path,allPathsSrcDest):
    #Function to define all the paths starting from src node to dest node
    #path = []   #List to hold the current path being explored
    path = path + [sourceNode]
    #print(path)

    if sourceNode == destinationNode: #Src itself is target
        allPathsSrcDest.append(path)
        return
    if sourceNode not in dictNetGraph:  #src not in graph
        return
    for node in dictNetGraph[sourceNode]:
        if node not in path:  # Avoid cycles
            dfsAllPathsSrcToDest(node, destinationNode,path,allPathsSrcDest)
    return allPathsSrcDest

def routingCommon():
    #Approach used: DFS to find all possible paths, then finding best path based on HOP|DIST
    createDictForNet()
    global allPathsSrcDest
    global allPaths

    #Calculating all possible paths for the given network
    for i in range(int(nodeCount)):
        for j in range(int(nodeCount)):
            path = []
            allPathsSrcDest = []
            allPathsSrcDest = dfsAllPathsSrcToDest(str(i), str(j),path,allPathsSrcDest)
            allPaths.append(allPathsSrcDest)
    #print(allPaths[1])     #Tested, proper

#Find Find min Cost(Based on hop) path between given node pairs
def calcCostHop(pathSrcDst,cost):
    #print(pathSrcDst)      #Tested, getting proper  lists
    #print()
    cost.append(len(pathSrcDst))

# def calcDelayOfPath(path1,delay):
#     #print(path1)   #Tested, getting paths
#     #delay = 0
#     for i in range(len(path1) - 1):
#         for j in range(int(edgeCount)):
#             if(int(path1[i]) == npLinks[j][0] and int(path1[i+1] == npLinks[j][1])):
#                 delay += npLinks[j][2]
#     #return delay
#     #print('abcd')

#Find min Cost(Based on hop) path between all node pairs 
def routingHopBased():
    currSrcDstPath1 = []
    currSrcDstPath2 = []

    for i in range(int(nodeCount)):
        #minCostPathsHops.append(['Routing table for node ' + str(i)])
        #minCostPathsHops.append(['Destination | Path    |Path Delay |Path Cost  |'])
        for j in range(int(nodeCount)):
            if(i == j):
                # minCostPathsHops.append([str(j) + '|--   |--    |--     |'])
                # minCostPathsHops.append([str(j) + '|--   |--    |--     |'])
                minCostPathsHops.append([str(j)])
                minCostPathsHops.append([str(j)])                
                minCostAtHops.append(0)
                minCostAtHops.append(0)
                pathDelaysAtHops.append(0)
                pathDelaysAtHops.append(0)
            if(i != j):
                cost = []
                #print("src=" + str(i) + "dst=" + str(j))
                for pathSrcDst in allPaths[(i*int(nodeCount)) + j]:
                    calcCostHop(pathSrcDst,cost)
                #print(cost)     #tested, getting proper element count
                npCost = np.array(cost)
                #print(np.argsort(npCost))   #tested, argsort proper integers
                if(len(cost) >= 2):
                    cost1 = len(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]])
                    cost2 = len(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[1]])
                    minCostAtHops.append(cost1)
                    minCostAtHops.append(cost2)
                    delay1 = 0
                    delay2 = 0
                    #for k in range(cost1-1):
                    #calcDelayOfPath(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]],delay1) 
                    #for k in range(cost2-1):                   
                    #calcDelayOfPath(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[1]],delay2)
                    path1 = allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]]
                    path2 = allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[1]]
                    for ii in range(len(path1) - 1):
                        for jj in range(int(edgeCount)):
                            #if((int(path1[ii]) == npLinks[jj][0]) and (int(path1[ii+1] == npLinks[jj][1]))):
                            if(int(path1[ii]) == npLinks[jj][0]):
                                if(int(path1[ii+1]) == npLinks[jj][1]):
                                    #print("I'm here")
                                    delay1 += npLinks[jj][2]
                                    break
                            if(int(path1[ii]) == npLinks[jj][1]):       #As bidirectional links
                                if(int(path1[ii+1]) == npLinks[jj][0]):
                                    delay1 += npLinks[jj][2]
                                    break                            
                    for ii in range(len(path2) - 1):        
                        for jj in range(int(edgeCount)):
                            if(int(path2[ii]) == npLinks[jj][0]):
                                if(int(path2[ii+1]) == npLinks[jj][1]):
                                    delay2 += npLinks[jj][2]
                                    break
                            if(int(path2[ii]) == npLinks[jj][1]):
                                if(int(path2[ii+1]) == npLinks[jj][0]):
                                    delay2 += npLinks[jj][2]
                                    break
                    pathDelaysAtHops.append(int(delay1))
                    pathDelaysAtHops.append(int(delay2))
                    #minCostPathsHops.append([str(j) + '  |' + allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]] +' |' + str(delay1) +' |' + str(cost1)])
                    #minCostPathsHops.append([str(j) + '  |' + allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[1]] +' |' + str(delay2) +' |' + str(cost2)])
                    minCostPathsHops.append(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]])
                    minCostPathsHops.append(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[1]])
                if(len(cost)==1):
                    cost1 = len(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]])
                    cost2 = int(-1)
                    minCostAtHops.append(cost1)
                    minCostAtHops.append(cost2)
                    delay1 = 0
                    delay2 = int(-1)
                    path1 = allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]]
                    for ii in range(len(path1) - 1):
                        for jj in range(int(edgeCount)):
                            if(int(path1[ii]) == npLinks[jj][0]):
                                if(int(path1[ii+1]) == npLinks[jj][1]):
                                    delay1 += npLinks[jj][2]
                                    break
                            if(int(path1[ii]) == npLinks[jj][1]):       #As bidirectional links
                                if(int(path1[ii+1]) == npLinks[jj][0]):
                                    delay1 += npLinks[jj][2]
                                    break   
                    pathDelaysAtHops.append(int(delay1))
                    pathDelaysAtHops.append(int(delay2))
                    minCostPathsHops.append(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]])      
                    minCostPathsHops.append([])   
                if(len(cost)==0):
                    cost1 = int(-1)
                    cost2 = int(-1)
                    minCostAtHops.append(cost1)
                    minCostAtHops.append(cost2)
                    delay1 = int(-1)
                    delay2 = int(-1)
                    pathDelaysAtHops.append(int(delay1))
                    pathDelaysAtHops.append(int(delay2))
                    minCostPathsHops.append([])      
                    minCostPathsHops.append([]) 

    # print(minCostPathsHops)
    # print(minCostAtHops)
    # print(pathDelaysAtHops)   #Tested path,cost,delay at list
    # #Write to output file
    routingFile = 'OutputFiles/routingtablefile.txt'
    mm = 0      #To track index of current element considered
    with open(routingFile, 'w') as routeFile:
        for i in range(int(nodeCount)):
            routeFile.write('\n\n\nRouting table (Hop based) for node ')
            routeFile.write(str(i))
            routeFile.write('\n')
            routeFile.write('|Destination | Path    |Path Delay |Path Cost  |\n')
            for j in range(int(nodeCount)): 
                routeFile.write(str(j))
                routeFile.write('\t|')
                routeFile.write(str(minCostPathsHops[mm]))
                routeFile.write('\t|')
                routeFile.write(str(pathDelaysAtHops[mm]))
                routeFile.write('\t|')
                routeFile.write(str(minCostAtHops[mm]))
                routeFile.write('\n')
                mm += 1
                #For second min path
                routeFile.write(str(j))
                routeFile.write('\t|')
                routeFile.write(str(minCostPathsHops[mm]))
                routeFile.write('\t|')
                routeFile.write(str(pathDelaysAtHops[mm]))
                routeFile.write('\t|')
                routeFile.write(str(minCostAtHops[mm]))
                routeFile.write('\n')
                mm += 1

def calcCostLink(pathSrcDst,cost):
    delay = 0
    for ii in range(len(pathSrcDst) - 1):
        for jj in range(int(edgeCount)):
            if(int(pathSrcDst[ii]) == npLinks[jj][0]):
                if(int(pathSrcDst[ii+1]) == npLinks[jj][1]):
                    delay += npLinks[jj][2]
                    break
            if(int(pathSrcDst[ii]) == npLinks[jj][1]):       #As bidirectional links
                if(int(pathSrcDst[ii+1]) == npLinks[jj][0]):
                    delay += npLinks[jj][2]
                    break   
    cost.append(delay)   


#Find min Cost(Based on prop delay) path between all node pairs 
def routingDistBased():
    for i in range(int(nodeCount)):
        for j in range(int(nodeCount)):
            if(i == j):
                minCostPathsLinks.append([str(j)])
                minCostPathsLinks.append([str(j)])                
                minCostAtLinks.append(0)
                minCostAtLinks.append(0)
                pathDelaysAtLinks.append(0)
                pathDelaysAtLinks.append(0)
            if(i != j):
                cost = []   #Cost is same as delay here
                for pathSrcDst in allPaths[(i*int(nodeCount)) + j]:
                    calcCostLink(pathSrcDst,cost)
                npCost = np.array(cost)
                if(len(cost) >= 2):
                    cost1 = npCost[np.argsort(npCost)[0]]
                    cost2 = npCost[np.argsort(npCost)[1]]
                    minCostAtLinks.append(int(cost1))
                    minCostAtLinks.append(int(cost2))
                    minCostPathsLinks.append(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]])
                    minCostPathsLinks.append(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[1]])
                if(len(cost)==0):
                    cost1 = int(-1)
                    cost2 = int(-1)
                    minCostAtLinks.append(cost1)
                    minCostAtLinks.append(cost2)
                    delay1 = int(-1)
                    delay2 = int(-1)
                    pathDelaysAtLinks.append(int(delay1))
                    pathDelaysAtLinks.append(int(delay2))
                    minCostPathsLinks.append([])      
                    minCostPathsLinks.append([]) 
                if(len(cost)==1):
                    cost1 = npCost[np.argsort(npCost)[0]]
                    cost2 = int(-1)
                    minCostAtLinks.append(cost1)
                    minCostAtLinks.append(cost2)
                    minCostPathsLinks.append(allPaths[(i*int(nodeCount)) + j][np.argsort(npCost)[0]])
                    minCostPathsLinks.append([])
    #print(minCostPathsLinks)
    #print(minCostAtLinks)
    routingFile = 'OutputFiles/' + args.rt
    mm = 0      #To track index of current element considered
    with open(routingFile, 'w') as routeFile:
        for i in range(int(nodeCount)):
            routeFile.write('\n\n\nRouting table (Propogation delay based) for node ')
            routeFile.write(str(i))
            routeFile.write('\n')
            routeFile.write('|Destination | Path    |Path Delay |Path Cost  |\n')
            for j in range(int(nodeCount)): 
                routeFile.write(str(j))
                routeFile.write('\t|')
                routeFile.write(str(minCostPathsLinks[mm]))
                routeFile.write('\t|')
                routeFile.write(str(minCostAtLinks[mm]))
                routeFile.write('\t|')
                routeFile.write(str(minCostAtLinks[mm]))
                routeFile.write('\n')
                mm += 1
                #For second min path
                routeFile.write(str(j))
                routeFile.write('\t|')
                routeFile.write(str(minCostPathsLinks[mm]))
                routeFile.write('\t|')
                routeFile.write(str(minCostAtLinks[mm]))
                routeFile.write('\t|')
                routeFile.write(str(minCostAtLinks[mm]))
                routeFile.write('\n')
                mm += 1


def connPessi(pathToFollow,i):
    global npLinkCapaAvail
    global success
    npPath = np.array(pathToFollow)
    length = len(npPath)
    for j in range(length-1):     
        tInd = int(npPath[j])* int(nodeCount) + int(npPath[j+1])
        if(int(npConnRequests[i][4]) > (npLinkCapaAvail[int(tInd)])):
            success = 0
            break
    if(success):
        for j in range(length-1):    
            tInd = int(npPath[j])* int(nodeCount) + int(npPath[j+1])
            #print(tInd)
            npLinkCapaAvail[int(tInd)] -= float((npConnRequests[i][4]))
        

def connOpti(pathToFollow,i):
    global npLinkCapaAvail
    global success
    npPath = np.array(pathToFollow)
    #print(npLinkCapaAvail)
    #print(npPath)
    length = len(npPath)
    if(npConnRequests[i][3]+(0.35*(npConnRequests[i][4]-npConnRequests[i][2])) > npConnRequests[i][4]): 
        bEqui = float(npConnRequests[i][4])
    else:
        bEqui = npConnRequests[i][3]+(0.35*(npConnRequests[i][4]-npConnRequests[i][2]))
    # #print(bEqui)    #bEqui is float, but if>2, decline anyway
    # bEqui = math.ceil(bEqui)
    # #print(bEqui)
    # for j in range(length-1):    
    #     #print(nodeCount)  
    #     tInd = int(npPath[j])* int(nodeCount) + int(npPath[j+1])
    #     #print(tInd)
    #     if(int(bEqui) > (npLinkCapaAvail[int(tInd)])):
    #         success = 0
    #         break
    #print(bEqui)    #bEqui is float, but if>2, decline anyway
    #bEqui = math.ceil(bEqui)
    #print(bEqui)
    for j in range(length-1):    
        #print(nodeCount)  
        tInd = int(npPath[j])* int(nodeCount) + int(npPath[j+1])
        #print(tInd)
        if((bEqui) > (npLinkCapaAvail[int(tInd)])):
            #print('hi')
            success = 0
            break   
    
    if(success):
        #Reduce link Capacity as we admitted conn
        for j in range(length-1):    
            tInd = int(npPath[j])* int(nodeCount) + int(npPath[j+1])
            #print(tInd)
            npLinkCapaAvail[int(tInd)] -= bEqui
        #print(npLinkCapaAvail)

def connProcessing():
    global linkCapaAvail
    global npLinkCapaAvail
    global connAdmitted
    global connRej
    global connIDsAdmitted
    global VCIDcontroller
    global fwdFileList
    global pathsFileList
    global admittedVCID
    global success

    #Create array to hold avail capacity of all nodes.
    npLinkCapaAvail = np.zeros((int(nodeCount)*int(nodeCount)))
    for i in range(int(edgeCount)):
        npLinkCapaAvail[(npLinks[i][0] * int(nodeCount)) + npLinks[i][1]] = float(npLinks[i][3])
        npLinkCapaAvail[(npLinks[i][1] * int(nodeCount)) + npLinks[i][0]] = float(npLinks[i][3])
        #print(npLinkCapaAvail[i][3])
    #     ints = [npLinks[i][0],npLinks[i][1],npLinks[i][3]]    #Ignored, Using np array now.
    #     linkCapaAvail.append(ints)
    # for i in range(int(edgeCount)):
    #     ints = [npLinks[i][1],npLinks[i][0],npLinks[i][3]]
    #     linkCapaAvail.append(ints)
    # npLinkCapaAvail = np.array(linkCapaAvail)
    
    #print(npLinkCapaAvail) #Tested, as bidirectional; 2 entry for each edge

    if(args.flag == 'hop'):
        for i in range(int(numConnReq)):
            #global success
            #Try if shortest cost path is available for requested connection.
            pathToFollow = minCostPathsHops[(2*npConnRequests[i][0]*int(nodeCount))+(2*npConnRequests[i][1])]
            #print(pathToFollow)
            success = 1
            if (args.p==1):
                connPessi(pathToFollow,i)
            else:
                connOpti(pathToFollow,i)
            if(not success):
                #print('Hey' + str(success))
                pathToFollow = minCostPathsHops[(2*npConnRequests[i][0]*int(nodeCount))+(2*npConnRequests[i][1])+1]
                success = 1
                if (args.p==1):
                    connPessi(pathToFollow,i)
                else:
                    connOpti(pathToFollow,i)
                #print('Hi' + str(success))
                if(not success):
                    connRej += 1
            if(success):
                    connAdmitted += 1
                    connIDsAdmitted.append(i)
                    #print(pathToFollow)
                    reqCountVC = len(pathToFollow) - 1
                    vcList = []
                    for kkk in range(reqCountVC):
                        vcList.append(VCIDcontroller)
                        VCIDcontroller +=1
                    #vcList = tArray.tolist()
                    admittedVCID.append(vcList)
                    temp = [i, npConnRequests[i][0], npConnRequests[i][1], pathToFollow, vcList, reqCountVC+1]
                    pathsFileList.append(temp)

                    #Update Forwarding table of nodes
                    npTemp = np.array(pathToFollow)
                    npT = np.array(vcList)
                    for jk in range(reqCountVC-1):
                        # if(jk == 0):
                        #     continue
                        te = [npTemp[jk+1],npTemp[jk],npT[jk],npTemp[jk+2],npT[jk+1]]
                        fwdFileList.append(te)
                    
                    

                    
    else:
        for i in range(int(numConnReq)):
            #global success
            #Try if shortest cost path is available for requested connection.
            pathToFollow = minCostPathsLinks[(2*npConnRequests[i][0]*int(nodeCount))+(2*npConnRequests[i][1])]
            success = 1
            if (args.p==1):
                connPessi(pathToFollow,i)
            else:
                connOpti(pathToFollow,i)
            if(not success):
                pathToFollow = minCostPathsLinks[(2*npConnRequests[i][0]*int(nodeCount))+(2*npConnRequests[i][1])+1]
                if (args.p==1):
                    connPessi(pathToFollow,i)
                else:
                    connOpti(pathToFollow,i)
                if(not success):
                    connRej += 1
            if(success):
                    connAdmitted += 1
                    connIDsAdmitted.append(i)
                    reqCountVC = len(pathToFollow) - 1
                    vcList = []
                    for kkk in range(reqCountVC):
                        vcList.append(VCIDcontroller)
                        VCIDcontroller +=1
                    admittedVCID.append(vcList)
                    cost1  = 0
                    #calcCostLink(pathToFollow,cost1)
                    delay = 0
                    for ii in range(len(pathToFollow) - 1):
                        for jj in range(int(edgeCount)):
                            if(int(pathToFollow[ii]) == npLinks[jj][0]):
                                if(int(pathToFollow[ii+1]) == npLinks[jj][1]):
                                    delay += npLinks[jj][2]
                                    break
                            if(int(pathToFollow[ii]) == npLinks[jj][1]):       #As bidirectional links
                                if(int(pathToFollow[ii+1]) == npLinks[jj][0]):
                                    delay += npLinks[jj][2]
                                    break 
                    cost1 = delay
                    temp = [i, npConnRequests[i][0], npConnRequests[i][1], pathToFollow, vcList, cost1]
                    pathsFileList.append(temp)
                    #Update Forwarding table of nodes
                    npTemp = np.array(pathToFollow)
                    npT = np.array(vcList)
                    for jk in range(reqCountVC-1):
                        # if(jk == 0):
                        #     continue
                        te = [npTemp[jk+1],npTemp[jk],npT[jk],npTemp[jk+2],npT[jk+1]]
                        fwdFileList.append(te)

    #print(pathsFileList)
    #print(npLinkCapaAvail)
    #create each nodes forwarding Table

    #Write data to paths file
    pathsFile = 'OutputFiles/' + args.path
    with open(pathsFile, 'w') as pFile:
        pFile.write('|Conn. ID | Source    |Dest |Path  |VC ID list |PathCost\n')
        for i in range(len(pathsFileList)):
            for j in range(6):
                pFile.write(str(pathsFileList[i][j]))
                pFile.write('\t')
            pFile.write('\n')
    #Sort based on first field
    sortedFwd = sorted(fwdFileList, key=lambda x: x[0])
    fwdFile = 'OutputFiles/' + args.ft
    #print(sortedFwd)
    with open(fwdFile, 'w') as fFile:       
        for i in range(len(sortedFwd)):
            if(i == 0):
                fFile.write('Forward Table of node ' + str(sortedFwd[i][0]) + '\n')
                fFile.write('|ThisRouter ID | Node ID of IncomingPort   |VC ID |Node ID of OutgoingPort  |VC ID \n')
            else:
                if(sortedFwd[i-1][0] != sortedFwd[i][0]):
                    fFile.write('Forward Table of node ' + str(sortedFwd[i][0]) + '\n')
                    fFile.write('|ThisRouter ID | Node ID of IncomingPort   |VC ID |Node ID of OutgoingPort  |VC ID \n')
            for j in range(5):
                fFile.write(str(sortedFwd[i][j]))
                fFile.write('\t')
            fFile.write('\n')
    print('Connections Admitted = ' + str(connAdmitted))
    print('Connections Rejected = ' + str(connRej))


def main():
    readFiles()
    # print(npLinks)
    # print()
    # print(npConnRequests)

    routingCommon()
    if(args.flag == 'hop'):
        routingHopBased()
    else:
        routingDistBased()

    connProcessing()


if __name__ == '__main__':
  main()

