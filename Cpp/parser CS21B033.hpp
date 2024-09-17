#pragma once

#include "util.hpp"
#include <fstream>
#include <sstream>
#include <vector>

class TopologyParser {
  std::ifstream topologyFile_;
  std::size_t N;
  std::size_t E;
  std::vector<std::vector<Link>> adjList_;

  friend class RoutingEngine;

public:
  TopologyParser(const char *topologyFilePath)
      : topologyFile_(topologyFilePath) {
    if (!topologyFile_.is_open()) {
      throw std::runtime_error("Failed to open Topology file");
    }

    parse_topology();
  }

  void parse_topology() {
    std::string line;

    {
      // parse first line => N E
      std::getline(topologyFile_, line);
      std::istringstream iss(line);
      iss >> N >> E;
    }

    adjList_.resize(N);

    for (std::size_t i = 0; i != E; i++) {
      std::size_t src, dest;
      float linkPropogationDelay, linkCapacity;

      std::getline(topologyFile_, line);
      std::istringstream iss(line);
      iss >> src >> dest >> linkPropogationDelay >> linkCapacity;

      adjList_[src].push_back({src, dest, linkPropogationDelay, linkCapacity});
      adjList_[dest].push_back({dest, src, linkPropogationDelay, linkCapacity});
    }
  }

  ~TopologyParser() { topologyFile_.close(); }
};

class ConnectionRequestsParser {
  std::vector<Connection> connections_;
  std::ifstream connectionFile_;
  std::size_t R_; // number of connection requests

public:
  ConnectionRequestsParser(const char *connectionFilePath)
      : connectionFile_(connectionFilePath) {
    if (!connectionFile_.is_open()) {
      throw std::runtime_error("Failed to open Connections file");
    }

    parse_connection_requests();
  }

  void parse_connection_requests() {
    std::string line;

    {
      // parse first line => R
      std::getline(connectionFile_, line);
      std::istringstream iss(line);
      iss >> R_;
    }

    connections_.resize(R_);

    for (std::size_t i = 0; i != R_; i++) {
      std::size_t src, dest;
      float minCapacity, averageCapacity, maxCapacity;

      std::getline(connectionFile_, line);
      std::istringstream iss(line);
      iss >> src >> dest >> minCapacity >> averageCapacity >> maxCapacity;

      connections_[i] = {src, dest, minCapacity, averageCapacity, maxCapacity};
    }
  }

  const std::vector<Connection> &connections() const { return connections_; }

  ~ConnectionRequestsParser() { connectionFile_.close(); }
};
