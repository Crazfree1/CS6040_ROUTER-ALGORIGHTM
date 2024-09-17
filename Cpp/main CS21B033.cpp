#include "connection_handler.hpp"
#include "util.hpp"

#include <fstream>
#include <variant>

auto optimistic_condition = [](const Connection &incomingConnection,
                               const EnrichedLink &link) {
  static constexpr auto get_bEquiv = [](const Connection &connection) {
    float bEquiv = connection.maxCapacity;
    bEquiv = std::min(connection.averageCapacity, bEquiv);
    bEquiv = std::min(0.35f * (connection.maxCapacity - connection.minCapacity),
                      bEquiv);
    return bEquiv;
  };
  float bEquivSum = 0;

  for (const Connection &connection : link.connections) {
    bEquivSum += get_bEquiv(connection);
  }
  return (get_bEquiv(incomingConnection) <= link.capacity - bEquivSum);
};

auto pessimistic_condition = [](const Connection &incomingConnection,
                                const EnrichedLink &link) {
  float bMaxSum = 0;

  for (const Connection &connection : link.connections) {
    bMaxSum += connection.maxCapacity;
  }
  return (incomingConnection.maxCapacity <= link.capacity - bMaxSum);
};

auto hop_metric = [](const Link &link) { return 1.f; };

auto delay_metric = [](const Link &link) { return link.propogationDelay; };

int main(int argc, char *argv[]) {
  std::map<std::string, std::string> config;
  for (int i = 1; i < argc; i += 2) {
    std::string arg = argv[i];
    if (arg[0] == '-') {
      arg = std::string(arg.begin() + 1, arg.end());
    }
    config[arg] = argv[i + 1];
  }

  std::string topologyFilePath = config["top"];
  std::string connectionFilePath = config["conn"];
  std::string routingTableFilePath = config["rt"];
  std::string forwardingTableFilePath = config["ft"];
  std::string pathTableFilePath = config["path"];
  std::string flag = config["flag"];
  std::string p = config["p"];

  ConnectionCondition connectionCondition = optimistic_condition;
  LinkDistanceCalculator linkDistanceCalculator = delay_metric;

  if (p == "1")
    connectionCondition = pessimistic_condition;

  if (flag == "hop")
    linkDistanceCalculator = hop_metric;

  TopologyParser topology(topologyFilePath.c_str());
  ConnectionRequestsParser connectionRequests(connectionFilePath.c_str());

  RoutingEngine routingEngine(topology, linkDistanceCalculator);
  ConnectionHandler connectionHandler(routingEngine, connectionCondition,
                                      linkDistanceCalculator);
  connectionHandler.setup_connections(connectionRequests.connections());

  std::ofstream routingTableFileStream = std::ofstream(routingTableFilePath);
  std::ofstream forwardingTableFileStream =
      std::ofstream(forwardingTableFilePath);
  std::ofstream pathTableFileStream = std::ofstream(pathTableFilePath);

  // std::ostream &routingTableFileStream = std::cout;
  // std::ostream &forwardingTableFileStream = std::cout;
  // std::ostream &pathTableFileStream = std::cout;

  routingEngine.print_routing_table(routingTableFileStream);
  connectionHandler.print_forwarding_table(forwardingTableFileStream);
  connectionHandler.print_connections(pathTableFileStream);

  std::cout << "The Blocking Probability is " << connectionHandler.get_blocking_probability() << std::endl;
}
