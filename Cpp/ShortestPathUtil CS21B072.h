#ifndef SHORTESTPATH_H
#define SHORTESTPATH_H

#include "RouterUtil.h"
#include <vector>
using namespace std;

namespace ShortestPathUtil
{

    vector<int> dijkstra(int aHopOrDistance, int src, int dest, const vector<vector<RouterUtil::Link>> &links);

    vector<RouterUtil::Path> yenKShortestPaths(int aHopOrDistance, int src, int dest, int K, const vector<vector<RouterUtil::Link>> &links);

    void getShortestPaths(int aHopOrDistance, const vector<vector<RouterUtil::Link>> &aLinks, int aNodeCount, vector<vector<vector<RouterUtil::Path>>> &aPaths);
}

#endif
