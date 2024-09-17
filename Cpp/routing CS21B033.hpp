#pragma once

#include "parser.hpp"
#include "util.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <numeric> // std::accumulate in numeric and not algorithm
#include <utility>

class RoutingEngine {
  // topology
  std::size_t N;
  std::size_t E;
  std::vector<std::vector<Link>> adjList;

  LinkDistanceCalculator linkDistanceCalculator;

  struct RoutingInformation {
    std::vector<Link> path;
    float pathDelay;
  };

  // map from (src, dest) to (shortestPathInformation,
  // secondShortestPathInformation)
  std::map<std::pair<std::size_t, std::size_t>,
           std::pair<RoutingInformation, RoutingInformation>>
      routingInformationMap;

  std::vector<Link> compute_shortest_path(std::size_t src, std::size_t dest) {
    // Use Dijkstra's algorithm
    std::vector<float> dist(N, std::numeric_limits<float>::max());
    std::vector<std::size_t> parent(N, std::size_t(-1));
    std::vector<bool> visited(N, false);

    dist[src] = 0;

    min_heap<std::pair<float, std::size_t>> pq;

    pq.push({0, src});

    while (!pq.empty()) {
      auto [d, u] = pq.top();
      pq.pop();

      if (visited[u])
        continue;
      visited[u] = true;

      for (auto &link : adjList[u])
        if (dist[link.dest] > dist[u] + linkDistanceCalculator(link)) {
          dist[link.dest] = dist[u] + linkDistanceCalculator(link);
          parent[link.dest] = u;
          pq.push({dist[link.dest], link.dest});
        }
    }

    std::vector<Link> path;
    for (std::size_t at = dest; at != std::size_t(-1); at = parent[at]) {
      if (parent[at] != std::size_t(-1))
        path.push_back({parent[at], at, 0, 0});
    }

    std::reverse(path.begin(), path.end());
    return path;
  }

  std::vector<Link>
  compute_second_shortest_path(std::size_t src, std::size_t dest,
                               std::vector<Link> &shortestPath) {

    float secondShortestPathLength = std::numeric_limits<float>::max();
    std::vector<Link> secondShortestPath;
    for (auto &link : shortestPath) {
      std::size_t u = link.src;
      std::size_t v = link.dest;
      //   we remove (u, v) from the graph and try to run dijkstra's algorithm

      struct RAII {
        Link link;
        RoutingEngine *routingEngine;

        RAII(std::size_t u, std::size_t v, RoutingEngine *routingEngine)
            : routingEngine(routingEngine) {
          auto it =
              std::find_if(routingEngine->adjList[u].begin(),
                           routingEngine->adjList[u].end(),
                           [v](const Link &link) { return link.dest == v; });
          link = *it;
          routingEngine->adjList[u].erase(it);
        }
        ~RAII() { routingEngine->adjList[link.src].push_back(link); }
      };

      auto l = [this, raii = RAII(u, v, this)]() { return; };

      std::vector<Link> path = compute_shortest_path(src, dest);
      if (path.size() == 0)
        continue;

      float pathLength = get_path_cost(path);
      if (pathLength < secondShortestPathLength) {
        secondShortestPath = std::move(path);
        secondShortestPathLength = pathLength;
      }
    }
    return secondShortestPath;
  }

  std::pair<std::vector<Link>, std::vector<Link>>
  compute_shortest_2_paths(std::size_t src, std::size_t dest) {
    auto shortestPath = compute_shortest_path(src, dest);
    auto secondShortestPath =
        compute_second_shortest_path(src, dest, shortestPath);
    return {shortestPath, secondShortestPath};
  }

  // all pairs shortest path
  void APSP() {
    for (std::size_t src = 0; src < adjList.size(); src++)
      for (std::size_t dest = 0; dest < adjList.size(); dest++) {

        auto [shortestPath, secondShortestPath] =
            compute_shortest_2_paths(src, dest);

        auto foldBinOp = [this](float accumulator, const Link &link) mutable {
          return accumulator + this->linkDistanceCalculator(link);
        };
        float shortestPathDelay = std::accumulate(
            shortestPath.begin(), shortestPath.end(), 0.f, foldBinOp);
        float secondShortestPathDelay =
            std::accumulate(secondShortestPath.begin(),
                            secondShortestPath.end(), 0.f, foldBinOp);

        routingInformationMap[{src, dest}] = {
            {shortestPath, shortestPathDelay},
            {secondShortestPath, secondShortestPathDelay}};
      }
  }

  friend class ConnectionHandler;

public:
  RoutingEngine(const TopologyParser &topology_,
                LinkDistanceCalculator &linkDistanceCalculator_)
      : linkDistanceCalculator(
            std::forward<LinkDistanceCalculator>(linkDistanceCalculator_)) {
    N = topology_.N;
    E = topology_.E;
    adjList = topology_.adjList_;

    APSP();
  }

  float get_path_cost(const std::vector<Link> &path) {
    float pathCost = 0;
    for (auto &link : path) {
      pathCost += linkDistanceCalculator(link);
    }
    return pathCost;
  }

  // clang-format off
  //(Destination Node) | (Path (from Source to Dest)) | (Path Delay) | (Path Cost)
  // clang-format on
  void print_routing_table(std::ostream &os) {
    std::vector<std::vector<std::string>> routingTableString;

    os << "Printing Routing Table" << std::endl;
    os << "Destination | Path | Path Delay | Path Cost" << std::endl;

    for (std::size_t src = 0; src < adjList.size(); src++) {
      routingTableString.push_back({"Source: " + std::to_string(src)});
      for (std::size_t dest = 0; dest < adjList.size(); dest++) {
        if (src == dest)
          continue;

        auto [shortestPath, secondShortestPath] =
            get_shortest_2_paths(src, dest);

        std::vector<std::string> row;

        if (shortestPath.path.size() != 0) {
          row.emplace_back(std::to_string(dest));
          row.emplace_back(util::build_path_sring(shortestPath.path));
          row.emplace_back(std::to_string(shortestPath.pathDelay));
          row.emplace_back(std::to_string(get_path_cost(shortestPath.path)));
          routingTableString.emplace_back(std::move(row));
        }

        row.clear();

        if (secondShortestPath.path.size() != 0) {
          row.emplace_back(std::to_string(dest));
          row.emplace_back(util::build_path_sring(secondShortestPath.path));
          row.emplace_back(std::to_string(secondShortestPath.pathDelay));
          row.emplace_back(
              std::to_string(get_path_cost(secondShortestPath.path)));
          routingTableString.emplace_back(std::move(row));
        }
      }
    }

    util::print_table(routingTableString, os);
  }

  void print_forwarding_file() {
    std::vector<std::vector<std::string>> routingTableString;
  }

  std::pair<RoutingInformation, RoutingInformation>
  get_shortest_2_paths(std::size_t src, std::size_t dst) {
    auto routingInformationPair = routingInformationMap[{src, dst}];
    return {routingInformationPair.first, routingInformationPair.second};
  }
};
