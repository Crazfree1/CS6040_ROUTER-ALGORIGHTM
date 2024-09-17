package network;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Network {
    Map<Integer, List<Edge>> edges = new HashMap<>();

    public Network(List<NetworkGraph> edges) {
        for (NetworkGraph edge : edges) {
            this.edges.putIfAbsent(edge.getNode1(), new ArrayList<>());
            this.edges.putIfAbsent(edge.getNode2(), new ArrayList<>());
            this.edges.get(edge.getNode1()).add(new Edge(edge.getNode2(), edge.getDelay(), edge.getCapacity()));
            this.edges.get(edge.getNode2()).add(new Edge(edge.getNode1(), edge.getDelay(), edge.getCapacity()));
        }
    }

    public Map<Integer, List<Edge>> getEdges() {
        return edges;
    }

    public Edge getEdgeBetweenNodes(int source, int destination) {
        var edges = this.edges.get(source);
        var edge = edges.stream()
                .filter(link -> link.node == destination)
                .findFirst()
                .orElse(null);
        return edge;
    }

    public double getAvailableEdgeCapacity(int firstNode, int secondNode) {
        var edges = this.edges.get(firstNode);
        for (var edge : edges) {
            if (edge.node == secondNode) {
                return edge.availableCapacity;
            }
        }
        return 0;
    }

    public double getAvailableLinkCapacity(Pair link) {
        var firstNode = link.getFirst();
        var secondNode = link.getSecond();
        var capacity = getAvailableEdgeCapacity(firstNode, secondNode);
        return capacity;
    }

    public int getDelayAlongPath(List<Integer> path) {
        int delay = 0;
        for (int i = 0; i < path.size() - 1; i++) {
            var source = path.get(i);
            var destination = path.get(i + 1);
            var link = getEdgeBetweenNodes(source, destination);
            if (link != null) {
                delay += link.getDelay();
            }
        }
        return delay;
    }

}