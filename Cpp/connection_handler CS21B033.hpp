#pragma once

#include "parser.hpp"
#include "routing.hpp"
#include "util.hpp"
#include <ostream>

class ConnectionHandler {
  enum ApproachType { OPTIMISTIC, PESSIMISTIC };

  RoutingEngine &routingEngine_;
  ConnectionCondition connectionCondition;
  LinkDistanceCalculator linkDistanceCaclulator;

  std::vector<EnrichedConnection> connectionInformation;
  std::map<std::pair<std::size_t, std::size_t>, EnrichedLink> enrichedLinkMap;
  std::vector<EnrichedConnection> connections;

  std::size_t totalConnectionsRequested = 0;
  std::size_t totalConnectionsSetup = 0;

  static constexpr auto vcIdGenerator = []() {
    static std::size_t vcId = 0;
    return vcId++;
  };

  bool setup_connection_on_path(const Connection &connection,
                                const std::vector<Link> &path) {
    std::size_t pathLinkIterator = 0;
    for (; pathLinkIterator != path.size(); pathLinkIterator++) {
      EnrichedLink &enrichedLink = enrichedLinkMap.at(std::make_pair(
          path[pathLinkIterator].src, path[pathLinkIterator].dest));

      if (!connectionCondition(connection, enrichedLink))
        break;

      enrichedLink.add_connection(connection);
    }

    if (pathLinkIterator == path.size())
      return true;

    if (pathLinkIterator == 0)
      return false;

    do {
      --pathLinkIterator;
      EnrichedLink &enrichedLink = enrichedLinkMap.at(
          {path[pathLinkIterator].src, path[pathLinkIterator].dest});
      enrichedLink.rollback_last_connection();
    } while (pathLinkIterator != 0);

    return false;
  }

  // returns false if connection can not be setup
  bool setup_connection(const Connection &connection) {
    totalConnectionsRequested++;
    auto [shortestPathInformation, secondShortestPathInformation] =
        routingEngine_.get_shortest_2_paths(connection.src, connection.dest);

    auto &shortestPath = shortestPathInformation.path;
    auto &secondShortestPath = secondShortestPathInformation.path;

    if (shortestPath.size() != 0 &&
        setup_connection_on_path(connection, shortestPath)) {
      connections.emplace_back(connection, std::move(shortestPath),
                               vcIdGenerator());
      return true;
    } else if (secondShortestPath.size() != 0 &&
               setup_connection_on_path(connection, secondShortestPath)) {
      connections.emplace_back(connection, std::move(secondShortestPath),
                               vcIdGenerator());
      return true;
    }

    return false;
  }

public:
  void setup_connections(const std::vector<Connection> &connections) {
    for (auto &connection : connections) {
      if (!setup_connection(connection))
        std::cout << "Connection " << connection.src << " -> "
                  << connection.dest << " can not be setup" << std::endl;
      else
        totalConnectionsSetup++;
    }
  }

  float get_blocking_probability() {
    return 1.0f - (float)totalConnectionsSetup / totalConnectionsRequested;
  }

  ConnectionHandler(RoutingEngine &routingEngine,
                    ConnectionCondition &connectionCondition,
                    LinkDistanceCalculator &linkDistanceCaclulator)
      : routingEngine_(routingEngine), connectionCondition(connectionCondition),
        linkDistanceCaclulator(linkDistanceCaclulator) {
    for (auto &links : routingEngine_.adjList)
      for (auto &link : links)
        enrichedLinkMap.emplace(std::make_pair(
            std::make_pair(link.src, link.dest), EnrichedLink(link)));
  }

  // This Router’s ID | Node ID of Incoming Port | VC ID | Node ID of Outgoing
  // Port | VC ID |
  void print_forwarding_table(std::ostream &os) {
    std::vector<std::vector<std::string>> table;

    os << "Printing Forwarding Table\n";
    os << "(This Router’s ID) | (Node ID of Incoming Port) | (VC ID) | (Node "
          "ID of Outgoing Port) | (VC ID)\n";

    for (auto &connection : connections) {
      // TOOD : what to do for first and last node
      std::size_t i = 0;
      {
        std::vector<std::string> row;

        row.emplace_back(std::to_string(connection.path[i].src));
        row.emplace_back("-");
        row.emplace_back(std::to_string(connection.vcId));
        row.emplace_back(std::to_string(connection.path[i].dest));
        row.emplace_back(std::to_string(connection.vcId));

        table.emplace_back(std::move(row));
        i++;
      }
      for (; i < connection.path.size(); i++) {
        std::vector<std::string> row;

        row.emplace_back(std::to_string(connection.path[i].src));
        row.emplace_back(std::to_string(connection.path[i - 1].src));
        row.emplace_back(std::to_string(connection.vcId));
        row.emplace_back(std::to_string(connection.path[i].dest));
        row.emplace_back(std::to_string(connection.vcId));

        table.emplace_back(std::move(row));
      }
    }

    util::print_table(table, os);
  }

  // Conn. ID Source Dest Path VC ID List PathCos
  void print_connections(std::ostream &os) {
    std::vector<std::vector<std::string>> table;
    os << "Printing Connections\n";
    os << "(Conn. ID) | (Source) | (Dest) | (Path VC ID List) | (PathCos)\n";

    for (std::size_t i = 0; i < connections.size(); i++) {
      std::vector<std::string> row;
      row.emplace_back(std::to_string(i));
      row.emplace_back(std::to_string(connections[i].src));
      row.emplace_back(std::to_string(connections[i].dest));
      row.emplace_back(util::build_path_sring(connections[i].path));
      row.emplace_back(
          util::build_vc_ids(std::vector<int>(connections[i].path.size(), i)));
      row.emplace_back(
          std::to_string(routingEngine_.get_path_cost(connections[i].path)));

      table.emplace_back(std::move(row));
    }

    util::print_table(table, os);
  }
};
