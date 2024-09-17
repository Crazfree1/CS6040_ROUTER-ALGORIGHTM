package network;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Router {
    private Logger logger = Logger.getLogger(Router.class.getName());
    Network network;
    Map<String, List<Path>> routes = new HashMap<String, List<Path>>();
    PathFinder pathFinder;
    BFSPathFinder bfsPathFinder;

    public Router(Network network, PathFinder pathFinder) {
        this.network = network;
        this.pathFinder = pathFinder;
        generateRoutingTable();
    }

    public Set<Integer> getNodes() {
        return network.getEdges().keySet();
    }

    public List<String> getRoutingTable() {
        var routingTable = new ArrayList<String>();
        var pairs = Pair.findUniquePairs(getNodes());
        for (var pair : pairs) {
            var forwardPaths = routes.get(computeRouteHash(pair.getFirst(), pair.getSecond()));
            var reversePaths = routes.get(computeRouteHash(pair.getSecond(), pair.getFirst()));
            var paths = Stream.concat(forwardPaths.stream(),
                    reversePaths.stream()).collect(Collectors.toList());
            for (Path path : paths) {
                routingTable.add(path.getRoutingTableEntry());
            }
        }
        return routingTable;
    }

    public List<Path> getPathsBetween(int source, int destination) {
        return routes.get(computeRouteHash(destination, source));
    }

    String computeRouteHash(int destination, int source) {
        return destination + "-" + source;
    }

    List<Path> findShortestPaths(int source, int destination) {
        List<List<Integer>> paths = pathFinder.findPaths(source, destination);
        List<Path> pathObjects = new ArrayList<>();
        for (var path : paths) {
            logger.info(path.toString());
            int delay = network.getDelayAlongPath(path);
            var pathObj = new Path(path, delay);
            pathObjects.add(pathObj);
        }
        return pathObjects;
    }

    void generateRoutingTable() {
        logger.info("Generating Routing Table...");
        var pairs = Pair.findUniquePairs(getNodes());
        for (var pair : pairs) {
            logger.info(pair.toString());
            var source = pair.getFirst();
            var destination = pair.getSecond();
            var paths = findShortestPaths(source, destination);
            var reversePaths = new ArrayList<Path>();
            for (var path : paths) {
                reversePaths.add(path.reverse());
            }
            if (paths.size() > 0) {
                routes.put(computeRouteHash(destination, source), paths);
                routes.put(computeRouteHash(source, destination), reversePaths);
            }
        }
        logger.info("Routing Table Generated");
    }
}
