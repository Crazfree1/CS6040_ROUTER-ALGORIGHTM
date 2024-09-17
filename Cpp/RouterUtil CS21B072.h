#ifndef ROUTERUTIL_H
#define ROUTERUTIL_H

#include <vector>
#include <string>

namespace RouterUtil
{
    class ConnectionRequest
    {
    public:
        int myConnectionId;
        int mySource;
        int myDestination;
        double myMinBandwidth;
        double myAvgBandwidth;
        double myMaxBandwidth;

        ConnectionRequest(int aConnectionId, int aSource, int aDestination, double aMinBandwidth, double aAvgBandwidth, double aMaxBandwidth)
            : myConnectionId(aConnectionId), mySource(aSource), myDestination(aDestination), myMinBandwidth(aMinBandwidth), myAvgBandwidth(aAvgBandwidth), myMaxBandwidth(aMaxBandwidth) {}
    };

    class LinkUser
    {
    public:
        int myConnectionId;
        double myEffectiveUsage;

        LinkUser() = default;

        LinkUser(int aConnectionId, double aEffectiveUsage)
            : myConnectionId(aConnectionId), myEffectiveUsage(aEffectiveUsage) {}
    };

    class Link
    {
    public:
        double myDelay;
        double myCapacity;
        int myExists;
        std::vector<LinkUser> myLinkUsers;

        Link() : myExists(0) {
            myLinkUsers = std::vector<LinkUser>();
        }

        Link(double aDelay, double aCapacity)
            : myDelay(aDelay), myCapacity(aCapacity), myExists(1)
        {
            myLinkUsers = std::vector<LinkUser>();
        }
    };

    struct Path
    {
        std::vector<int> nodes;
        double cost;
        double delay;
        bool operator<(const Path &other) const
        {
            return cost > other.cost;
        }
    };

    struct AcceptedConnection
    {
        int connectionId;
        int src;
        int dest;
        Path path;
        std::vector<int> vcidList;
        int pathCost;

        AcceptedConnection(int aConnectionId, int aSrc, int aDest, Path aPath, std::vector<int> aVcidList, int aPathCost)
            : connectionId(aConnectionId), src(aSrc), dest(aDest), path(aPath), vcidList(aVcidList), pathCost(aPathCost) {}
    };

    class UniqueIDGenerator
    {
    public:
        std::vector<std::vector<int>> linkVcids;
        UniqueIDGenerator(int aNodeCount) : linkVcids(aNodeCount, std::vector<int>(aNodeCount, 0)) {}
        int genID(int aNode1, int aNode2)
        {
            linkVcids[aNode1][aNode2]++;
            linkVcids[aNode2][aNode1]++;
            return linkVcids[aNode1][aNode2];
        }
    };

    int genConnectionId();
    bool readTopology(const std::string &aFilename, int &aNodeCount, int &aEdgeCount, std::vector<std::vector<Link>> &aLinks);
    bool readConnections(const std::string &aFilename, int &aConnectionCount, std::vector<ConnectionRequest> &aConnections);
    std::string genPathString(Path aPath);
    std::string genVcidListString(std::vector<int> aVcidList);
    int processConnectionRequest(ConnectionRequest &aConnectionRequest, std::vector<std::vector<Link>> &aLinks, std::vector<std::vector<std::vector<Path>>> &aShortestPaths, int aApproach);
    int getPortNumber(int aRouter, int aBranch, const std::vector<std::vector<Link>> &aLinks);
}

#endif // ROUTERUTIL_H
