#include <bits/stdc++.h>
#include "RouterUtil.h"
#include "ShortestPathUtil.h"
using namespace std;
int DEBUG = 0;

int main(int argc, char *argv[])
{
    string myTopoFile = argv[2];
    string myConnectionsFile = argv[4];
    string myRoutingTableFile = argv[6];
    string myForwardingFile = argv[8];
    string myPathsFile = argv[10];
    int myHopOrDist = strcmp(argv[12], "hop") == 0 ? 0 : 1;
    int myApproach = atoi(argv[14]);

    int myNodeCount, myEdgeCount;
    ifstream file(myTopoFile);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << myTopoFile << endl;
        return 1;
    }
    file >> myNodeCount >> myEdgeCount;
    vector<vector<RouterUtil::Link>> myLinks(myNodeCount, vector<RouterUtil::Link>(myNodeCount, RouterUtil::Link()));
    if (RouterUtil::readTopology(myTopoFile, myNodeCount, myEdgeCount, myLinks))
    {
        if (DEBUG)
        {
            cout << "Topology read successfully" << endl;
        }
    }
    else
    {
        cerr << "Error: Failed to read topology" << endl;
    }
    int myConnectionCount;
    vector<RouterUtil::ConnectionRequest> myConnections;
    if (RouterUtil::readConnections(myConnectionsFile, myConnectionCount, myConnections))
    {
        if (DEBUG)
        {
            cout << "Connections read successfully" << endl;
        }
    }
    else
    {
        cerr << "Error: Failed to read connections" << endl;
    }
    vector<vector<vector<RouterUtil::Path>>> shortestPathsHops(myNodeCount, vector<vector<RouterUtil::Path>>(myNodeCount, vector<RouterUtil::Path>(2)));
    ShortestPathUtil::getShortestPaths(0, myLinks, myNodeCount, shortestPathsHops);
    vector<vector<vector<RouterUtil::Path>>> shortestPathsDistance(myNodeCount, vector<vector<RouterUtil::Path>>(myNodeCount, vector<RouterUtil::Path>(2)));
    ShortestPathUtil::getShortestPaths(1, myLinks, myNodeCount, shortestPathsDistance);

    // publish routing table
    ofstream myRoutingTable(myRoutingTableFile);
    if (DEBUG)
        cout << "Generating routing Table" << endl;
    for (int i = 0; i < myNodeCount; i++)
    {
        myRoutingTable << "Router " << i << endl;
        for (int j = 0; j < myNodeCount; j++)
        {
            if (i == j)
                continue;
            // delay | cost
            // hops
            myRoutingTable << j << setw(10) << " " << RouterUtil::genPathString(shortestPathsHops[i][j][0]) << setw(10) << shortestPathsHops[i][j][0].delay << setw(10) << shortestPathsHops[i][j][0].cost << endl;
            myRoutingTable << j << setw(10) << " " << RouterUtil::genPathString(shortestPathsHops[i][j][1]) << setw(10) << shortestPathsHops[i][j][1].delay << setw(10) << shortestPathsHops[i][j][1].cost << endl;
            // distances
            myRoutingTable << j << setw(10) << " " << RouterUtil::genPathString(shortestPathsDistance[i][j][0]) << setw(10) << shortestPathsDistance[i][j][0].delay << setw(10) << shortestPathsDistance[i][j][0].cost << endl;
            myRoutingTable << j << setw(10) << " " << RouterUtil::genPathString(shortestPathsDistance[i][j][1]) << setw(10) << shortestPathsDistance[i][j][1].delay << setw(10) << shortestPathsDistance[i][j][1].cost << endl;
        }
    }
    ofstream myForwardingTable(myForwardingFile);
    RouterUtil::UniqueIDGenerator vcidGenerator(myNodeCount);
    int numAcceptedConnections = 0;
    vector<RouterUtil::AcceptedConnection> acceptedConnections;
    for (auto &aConnectionRequest : myConnections)
    {
        int possible;
        // 0-fail, 1-1st shortest path, 2-2nd shortest path
        if (myHopOrDist == 0)
            possible = RouterUtil::processConnectionRequest(aConnectionRequest, myLinks, shortestPathsHops, myApproach);
        else
            possible = RouterUtil::processConnectionRequest(aConnectionRequest, myLinks, shortestPathsDistance, myApproach);
        // set up virtual circuit
        if (possible)
        {
            numAcceptedConnections++;
            cout << "Setting up virtual circuit for connection " << aConnectionRequest.myConnectionId << endl;
            RouterUtil::Path myPath;
            if (myHopOrDist == 0)
            {
                myPath = shortestPathsHops[aConnectionRequest.mySource][aConnectionRequest.myDestination][possible - 1];
            }
            else
            {
                myPath = shortestPathsDistance[aConnectionRequest.mySource][aConnectionRequest.myDestination][possible - 1];
            }
            vector<int> vcidList;
            int mySrc = aConnectionRequest.mySource;
            int curVcid = vcidGenerator.genID(mySrc, myPath.nodes[1]);
            vcidList.push_back(curVcid);
            for (int i = 1; i < myPath.nodes.size() - 1; i++)
            {
                int myNext = myPath.nodes[i];
                int inputPortNo = RouterUtil::getPortNumber(myNext, mySrc, myLinks);
                int outputPortNo = RouterUtil::getPortNumber(myNext, myPath.nodes[i + 1], myLinks);
                int outVcid = vcidGenerator.genID(myNext, myPath.nodes[i + 1]);
                myForwardingTable << myNext << setw(10) << inputPortNo << setw(10) << curVcid << setw(10) << outputPortNo << setw(10) << outVcid << endl;
                curVcid = outVcid;
                vcidList.push_back(outVcid);
                mySrc = myNext;
            }
            RouterUtil::AcceptedConnection myAcceptedConnection(aConnectionRequest.myConnectionId, aConnectionRequest.mySource, aConnectionRequest.myDestination, myPath, vcidList, myPath.cost);
            acceptedConnections.push_back(myAcceptedConnection);
        }
    }
    ofstream myPaths(myPathsFile);
    myPaths << myConnections.size() << " " << numAcceptedConnections << endl;
    for (auto &connection : acceptedConnections)
    {
        myPaths << connection.connectionId << setw(10) << " " << connection.src << setw(10) << " " << connection.dest << setw(10) << " " << RouterUtil::genPathString(connection.path) << setw(10) << " " << RouterUtil::genVcidListString(connection.vcidList) << setw(10) << " " << connection.pathCost << endl;
    }
}