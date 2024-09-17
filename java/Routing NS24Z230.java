import java.util.logging.Level;
import java.util.logging.Logger;

import io.ConnectionsFileParser;
import io.OutputProcessor;
import io.TopologyFileParser;
import network.BFSPathFinder;
import network.ConnectionController;
import network.DijkstraPathFinder;
import network.Network;
import network.PathFinder;
import network.Router;
import network.RoutingMetric;

public class Routing {
    private static Logger logger = Logger.getLogger(Routing.class.getName());

    public static void main(String[] args) {
        logger.setLevel(Level.FINE);
        var config = new Configuration().importFromCLIArgs(args);
        var networkGraphs = new TopologyFileParser(config.getTopologyFile()).readNetworkGraphs();
        var connectionRequests = new ConnectionsFileParser(config.getConnectionsFile())
                .toConnectionRequests();

        var network = new Network(networkGraphs);
        var pathFinder = buildPathFinder(config.getMetric(), network);
        var router = new Router(network, pathFinder);
        var connectionsController = new ConnectionController(network, router, connectionRequests, config.getMode());

        var routingTable = router.getRoutingTable();
        var forwardingTable = connectionsController.getForwardingTable();
        var totalConnectionRequests = connectionsController.getTotalConnectionRequests();
        var connections = connectionsController.getConnections();
        var paths = connectionsController.getPaths(config.getMetric());
        new OutputProcessor(config.getRoutingTableFile(), config.getForwardingTableFile(), config.getPathsFile())
                .writeRoutingTableFile(routingTable)
                .writeForwardingTableFile(forwardingTable)
                .writePathsFile(totalConnectionRequests, connections, paths);
    }

    private static PathFinder buildPathFinder(RoutingMetric metric, Network network) {
        if (metric == RoutingMetric.DISTANCE) {
            return new DijkstraPathFinder(network);
        }
        return new BFSPathFinder(network);
    }
}
