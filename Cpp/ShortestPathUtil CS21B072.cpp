#include "ShortestPathUtil.h"
#include "RouterUtil.h"
#include <bits/stdc++.h>

using namespace std;

namespace ShortestPathUtil
{
    vector<int> dijkstra(int aHopOrDistance, int src, int dest, const vector<vector<RouterUtil::Link>> &links)
    {
        int n = links.size();
        vector<double> dist(n, INT_MAX);
        vector<int> parent(n, -1);
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;

        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty())
        {
            auto [d, u] = pq.top();
            pq.pop();

            if (u == dest)
                break;

            for (int v = 0; v < n; ++v)
            {
                if (links[u][v].myExists && dist[v] > d + ((aHopOrDistance == 1) ? links[u][v].myDelay : 1))
                {
                    dist[v] = d + ((aHopOrDistance == 1) ? links[u][v].myDelay : 1);
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        vector<int> path;
        for (int at = dest; at != -1; at = parent[at])
            path.push_back(at);
        reverse(path.begin(), path.end());

        if (path.size() == 1 && path[0] != src)
            path.clear();

        return path;
    }

    vector<RouterUtil::Path> yenKShortestPaths(int aHopOrDistance, int src, int dest, int K, const vector<vector<RouterUtil::Link>> &links)
    {
        vector<RouterUtil::Path> A;
        priority_queue<RouterUtil::Path> B;

        vector<int> firstPath = dijkstra(aHopOrDistance, src, dest, links);
        if (!firstPath.empty())
        {
            double cost = 0;
            double delay = 0;
            for (int i = 0; i < firstPath.size() - 1; ++i)
            {
                cost += ((aHopOrDistance == 1) ? links[firstPath[i]][firstPath[i + 1]].myDelay : 1);
                delay += links[firstPath[i]][firstPath[i + 1]].myDelay;
            }
            A.push_back({firstPath, cost, delay});
        }

        for (int k = 1; k < K; ++k)
        {
            const RouterUtil::Path &prevPath = A[k - 1];
            int n = prevPath.nodes.size();

            for (int i = 0; i < n - 1; ++i)
            {
                vector<int> spurPath;
                vector<vector<RouterUtil::Link>> tempLinks = links;

                int spurNode = prevPath.nodes[i];
                vector<int> rootPath(prevPath.nodes.begin(), prevPath.nodes.begin() + i + 1);

                for (const RouterUtil::Path &path : A)
                {
                    vector<int> segment(path.nodes.begin(), path.nodes.begin() + i + 1);
                    if (segment == rootPath)
                    {
                        tempLinks[path.nodes[i]][path.nodes[i + 1]].myExists = 0;
                    }
                }

                spurPath = dijkstra(aHopOrDistance, spurNode, dest, tempLinks);

                if (!spurPath.empty())
                {
                    spurPath.insert(spurPath.begin(), rootPath.begin(), rootPath.end() - 1);
                    double cost = 0;
                    double delay = 0;
                    for (int j = 0; j < spurPath.size() - 1; ++j)
                    {
                        cost += ((aHopOrDistance == 1) ? links[spurPath[j]][spurPath[j + 1]].myDelay : 1);
                        delay += links[spurPath[j]][spurPath[j + 1]].myDelay;
                    }

                    B.push({spurPath, cost, delay});
                }
            }

            if (B.empty())
                break;

            A.push_back(B.top());
            B.pop();
        }

        return A;
    }

    void getShortestPaths(int aHopOrDistance, const vector<vector<RouterUtil::Link>> &aLinks, int aNodeCount, vector<vector<vector<RouterUtil::Path>>> &aPaths)
    {
        for (int i = 0; i < aNodeCount; i++)
        {
            for (int j = 0; j < aNodeCount; j++)
            {
                if (i == j)
                    continue;
                vector<RouterUtil::Path> shortestPaths = ShortestPathUtil::yenKShortestPaths(aHopOrDistance, i, j, 2, aLinks);
                aPaths[i][j] = shortestPaths;
            }
        }
    }
}
