package network;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.logging.Logger;
import java.util.stream.Collectors;

public class ConnectionController {
    int connections = 0;
    Network network;
    Router router;
    AdmissionStrategy mode;
    List<ConnectionRequest> connectionRequests;
    List<ConnectionRequest> rejectedRequests = new ArrayList<>();
    List<AdmittedConnection> admittedConnections = new ArrayList<>();
    List<ForwardingTableEntry> forwardingTable = new ArrayList<ForwardingTableEntry>();
    private Logger logger = Logger.getLogger(ConnectionController.class.getName());

    public ConnectionController(Network network, Router router, List<ConnectionRequest> connectionRequests,
            AdmissionStrategy mode) {
        this.network = network;
        this.router = router;
        this.connectionRequests = connectionRequests;
        this.mode = mode;
        processConnectionRequests();
    }

    public List<String> getForwardingTable() {
        return forwardingTable.stream().sorted(Comparator.comparingInt(ForwardingTableEntry::getNode)
                .thenComparingInt(ForwardingTableEntry::getPrevious).thenComparingInt(ForwardingTableEntry::getNext))
                .map(te -> te.toString()).collect(Collectors.toList());
    }

    double getEffectiveBandwidth(ConnectionRequest connectionRequest) {
        System.out.println("mode" + mode);
        System.out.println(connectionRequest);
        var effectiveBandwidth = Math.min(connectionRequest.bMax,
                (connectionRequest.bAve + (0.35 * (connectionRequest.bMax - connectionRequest.bMin))));
        System.out.println("effectiveBandwidth" + effectiveBandwidth);
        if (mode == AdmissionStrategy.OPTIMISTIC) {
            return effectiveBandwidth;
        }
        System.out.println("maxBandwidth" + connectionRequest.bMax);
        return connectionRequest.bMax;
    }

    boolean checkIfAdmissionPossible(Path path, Double effectiveBandwidth) throws Exception {
        logger.info(path.toString());
        var links = path.getLinks();
        for (var link : links) {
            Double availableCapacity = this.network.getAvailableLinkCapacity(link);
            logger.info(link.toString());
            logger.info("availableCapacity: ");
            logger.info(availableCapacity.toString());
            logger.info("effectiveBandwidth: ");
            logger.info(effectiveBandwidth.toString());
            if (effectiveBandwidth > availableCapacity) {
                return false;
            }
        }
        return true;
    }

    List<Integer> admitConnection(Path path, double bandwidth) {
        var links = path.getLinks();
        List<Integer> vcids = new ArrayList<Integer>();
        logger.info(links.toString());
        Integer lastLinkVCID = -1;
        Integer previousNode = -1;
        for (int i = 0; i < links.size(); i++) {
            var link = links.get(i);
            var currentNode = link.getFirst();
            var nextNode = link.getSecond();
            var edge = network.getEdgeBetweenNodes(currentNode, nextNode);
            if (edge != null) {
                edge.allocateCapacity(bandwidth);
                var vcid = edge.getNextVCID();
                if (i > 0 & previousNode > -1 & lastLinkVCID > -1) {
                    var forwardingTableEntry = new ForwardingTableEntry(currentNode, previousNode, lastLinkVCID,
                            nextNode, vcid);
                    forwardingTable.add(forwardingTableEntry);
                    logger.info(forwardingTableEntry.toString());
                    vcids.add(vcid);
                }
                lastLinkVCID = vcid;
                previousNode = currentNode;
            }
        }
        return vcids;
    }

    Path getAdmissiblePath(Path firstPath, Path secondPath, double effectiveBandwidth) throws Exception {
        var isAdmissionPossible = checkIfAdmissionPossible(firstPath, effectiveBandwidth);
        if (isAdmissionPossible) {
            // Admit
            logger.info("Admit on first path");
            return firstPath;
        }
        isAdmissionPossible = checkIfAdmissionPossible(secondPath, effectiveBandwidth);
        if (isAdmissionPossible) {
            // Admit
            logger.info("Admit on second path");
            return secondPath;
        }
        throw new Exception();
    }

    public boolean processConnectionRequest(ConnectionRequest connectionRequest) {
        logger.info(connectionRequest.toString());
        var effectiveBandwidth = getEffectiveBandwidth(connectionRequest);
        // Get reverse path since we need to allocate resources from destination to
        // source
        var source = connectionRequest.src;
        var destination = connectionRequest.dest;
        var reversePaths = router.getPathsBetween(source, destination);
        try {
            var firstPath = reversePaths.get(0);
            var secondPath = reversePaths.get(1);
            var admissiblePath = getAdmissiblePath(firstPath, secondPath, effectiveBandwidth);
            var vcids = admitConnection(admissiblePath, effectiveBandwidth);
            connections++;
            var forwardPath = admissiblePath.reverse();
            admittedConnections.add(new AdmittedConnection(connections, connectionRequest, forwardPath, vcids));
            logger.info(forwardPath.toString());
            return true;
        } catch (Exception e) {
            logger.info("Reject connection request");
            rejectedRequests.add(connectionRequest);
            return false;
        }
    }

    void processConnectionRequests() {
        for (var connectionRequest : connectionRequests) {
            processConnectionRequest(connectionRequest);
        }
    }

    public int getTotalConnectionRequests() {
        return connectionRequests.size();
    }

    public int getConnections() {
        return connections;
    }

    public int getRejectedConnections() {
        return rejectedRequests.size();
    }

    public List<String> getPaths(RoutingMetric metric) {
        var paths = new ArrayList<String>();
        for (var connection : admittedConnections) {
            String row = connection.id + "," + connection.request.src + "," + connection.request.dest + ","
                    + connection.path.getPlainPathString() + "," + connection.vcidList + ","
                    + (metric == RoutingMetric.DISTANCE ? connection.path.delay : connection.path.hops);
            paths.add(row);
        }
        return paths;
    }
}
