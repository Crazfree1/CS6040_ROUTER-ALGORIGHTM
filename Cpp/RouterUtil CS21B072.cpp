#include "RouterUtil.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

namespace RouterUtil
{
    static int CONNECTION_ID = 0;

    int genConnectionId()
    {
        return CONNECTION_ID++;
    }

    bool readTopology(const string &aFilename, int &aNodeCount, int &aEdgeCount, vector<vector<Link>> &aLinks)
    {
        ifstream file(aFilename);
        if (!file.is_open())
        {
            cerr << "Error: Could not open file " << aFilename << endl;
            return false;
        }
        file >> aNodeCount >> aEdgeCount;
        if (file.fail())
        {
            cerr << "Error: Failed to read node and edge counts" << endl;
            return false;
        }
        int myNode1, myNode2;
        double myDelay, myCapacity;
        while (file >> myNode1 >> myNode2 >> myDelay >> myCapacity)
        {
            aLinks[myNode1][myNode2] = Link(myDelay, myCapacity);
            aLinks[myNode2][myNode1] = Link(myDelay, myCapacity);
        }
        if (file.fail() && !file.eof())
        {
            cerr << "Error: Failed to read edge info" << endl;
            return false;
        }
        return true;
    }

    bool readConnections(const string &aFilename, int &aConnectionCount, vector<ConnectionRequest> &aConnections)
    {
        ifstream file(aFilename);
        if (!file.is_open())
        {
            cerr << "Error: Could not open file " << aFilename << endl;
            return false;
        }
        file >> aConnectionCount;
        if (file.fail())
        {
            cerr << "Error: Failed to read connection count" << endl;
            return false;
        }
        int mySource, myDestination;
        double myMinBandwidth, myAvgBandwidth, myMaxBandwidth;
        while (file >> mySource >> myDestination >> myMinBandwidth >> myAvgBandwidth >> myMaxBandwidth)
        {
            aConnections.emplace_back(genConnectionId(), mySource, myDestination, myMinBandwidth, myAvgBandwidth, myMaxBandwidth);
        }
        if (file.fail() && !file.eof())
        {
            cerr << "Error: Failed to read connection info" << endl;
            return false;
        }
        return true;
    }

    string genPathString(Path aPath)
    {
        string myPath = to_string(aPath.nodes[0]);
        int myNext = aPath.nodes[0];

        for (auto &node : aPath.nodes)
        {
            if (node == aPath.nodes[0])
                continue;
            myPath += " -> " + to_string(node);
            myNext = node;
        }
        return myPath;
    }

    string genVcidListString(vector<int> aVcidList)
    {
        string myVcidList = to_string(aVcidList[0]);
        for (int i = 1; i < aVcidList.size(); i++)
        {
            myVcidList += " -> " + to_string(aVcidList[i]);
        }
        return myVcidList;
    }

    int processConnectionRequest(ConnectionRequest &aConnectionRequest, vector<vector<Link>> &aLinks, vector<vector<vector<Path>>> &aShortestPaths, int aApproach)
    {
        int mySrc = aConnectionRequest.mySource;
        int myDest = aConnectionRequest.myDestination;
        double finalBandwidth;
        if (aApproach == 0)
        {
            finalBandwidth = min(aConnectionRequest.myMaxBandwidth, aConnectionRequest.myAvgBandwidth + 0.35 * (aConnectionRequest.myMaxBandwidth - aConnectionRequest.myMinBandwidth));
        }
        else
        {
            finalBandwidth = aConnectionRequest.myMaxBandwidth;
        }

        vector<pair<int, int>> myPath;
        int fails = 0;
        for (Path &path : aShortestPaths[mySrc][myDest])
        {
            // 2 paths
            vector<int> nodes = path.nodes;
            int myStart = mySrc;
            bool worked = true;
            for (int i = 1; i < nodes.size(); i++)
            {
                int myNext = nodes[i];
                double sum = 0;
                for (LinkUser &linkUser : aLinks[myStart][myNext].myLinkUsers)
                {
                    sum += linkUser.myEffectiveUsage;
                }
                if (finalBandwidth <= aLinks[myStart][myNext].myCapacity - sum)
                {
                    aLinks[myStart][myNext].myLinkUsers.emplace_back(aConnectionRequest.myConnectionId, finalBandwidth);
                    myPath.emplace_back(myStart, myNext);
                    myStart = myNext;
                }
                else
                {
                    fails++;
                    worked = false;
                    break;
                }
            }
            if (worked)
            {
                cout << "Connection " << aConnectionRequest.myConnectionId << " successfully established in " << fails + 1 << " attempts" << endl;
                return fails + 1;
            }
            else
            {
                for (auto &link : myPath)
                {
                    aLinks[link.first][link.second].myLinkUsers.pop_back();
                }
                myPath.clear();
                if (fails == 2)
                {
                    cout << "Connection " << aConnectionRequest.myConnectionId << " failed" << endl;
                    return 0;
                }
            }
        }
        return 0;
    }

    int getPortNumber(int aRouter, int aBranch, const vector<vector<Link>> &aLinks)
    {
        int curPort = 0;
        for (int i = 0; i < aLinks[aRouter].size(); i++)
        {
            if (!aLinks[aRouter][i].myExists)
                continue;
            if (i == aBranch)
            {
                return curPort;
            }
            else
            {
                curPort++;
            }
        }
        return -1;
    }
}
