package network;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;

public class NetworkGenerator {

    public static List<NetworkGraph> generateTopology(int numNodes, int minEdges, int maxEdges, int minDelay,
            int avgDelay, int maxDelay, int minCapacity, int avgCapacity, int maxCapacity) {
        List<NetworkGraph> edges = new ArrayList<>();
        Set<Integer> connectedNodes = new HashSet<>();
        List<Integer> remainingNodes = new ArrayList<>();
        var random = new Random();
        // Initialize remaining nodes list
        for (int node = 0; node < numNodes; node++) {
            remainingNodes.add(node);
        }

        // Start with the first node
        int currentNode = remainingNodes.remove(random.nextInt(remainingNodes.size()));
        connectedNodes.add(currentNode);

        // Build the minimum spanning tree (MST) to ensure all nodes are connected
        while (!remainingNodes.isEmpty()) {
            int newNode = remainingNodes.remove(random.nextInt(remainingNodes.size()));
            int connectedNode = getRandomElement(connectedNodes, random);

            int delay = generateRandomValue(minDelay, avgDelay, maxDelay, random);
            int capacity = generateRandomValue(minCapacity, avgCapacity, maxCapacity, random);

            edges.add(new NetworkGraph(connectedNode, newNode, delay, capacity));
            connectedNodes.add(newNode);
        }

        // Add additional edges to meet the specified edge constraints
        for (int node = 0; node < numNodes; node++) {
            int numEdges = random.nextInt(maxEdges - minEdges + 1) + minEdges;

            // Already connected edges from the MST
            int existingEdges = 0;
            for (var edge : edges) {
                if ((edge.getNode1() == node) || (edge.getNode2() == node)) {
                    existingEdges++;
                }
            }

            for (int j = existingEdges; j < numEdges; j++) {
                int farEnd = random.nextInt(numNodes);
                if (farEnd == node || edgeExists(edges, node, farEnd))
                    continue; // Avoid self-loops and duplicate edges

                int delay = generateRandomValue(minDelay, avgDelay, maxDelay, random);
                int capacity = generateRandomValue(minCapacity, avgCapacity, maxCapacity, random);

                edges.add(new NetworkGraph(node, farEnd, delay, capacity));
            }
        }

        return edges;
    }

    public static Set<String> generateConnections(int numConnections, int nodes, int minBandwidth, int avgBandwidth,
            int maxBandwidth) {
        Random random = new Random();
        Set<String> connections = new HashSet<>();

        // Generate random connections
        for (int i = 0; i < numConnections; i++) {
            int node1 = random.nextInt(nodes); // Assuming 14 nodes in ARPANET
            int node2 = random.nextInt(nodes);
            while (node2 == node1) {
                node2 = random.nextInt(nodes); // Ensure node2 is different from node1
            }

            int bMin = generateRandomValue(minBandwidth, (minBandwidth + avgBandwidth) / 2, avgBandwidth, random);
            int bAve = generateRandomValue(bMin, (bMin + avgBandwidth) / 2, avgBandwidth, random);
            int bMax = generateRandomValue(bAve, (bAve + maxBandwidth) / 2, maxBandwidth, random);

            connections.add(node1 + " " + node2 + " " + bMin + " " + bAve + " " + bMax);
        }
        return connections;
    }

    private static boolean edgeExists(List<NetworkGraph> edges, int node1, int node2) {
        for (NetworkGraph edge : edges) {
            if ((edge.getNode1() == node1 && edge.getNode2() == node2)
                    || (edge.getNode1() == node2 && edge.getNode2() == node1)) {
                return true;
            }
        }
        return false;
    }

    private static int getRandomElement(Set<Integer> set, Random random) {
        int size = set.size();
        int item = random.nextInt(size);
        int i = 0;
        for (int element : set) {
            if (i == item)
                return element;
            i++;
        }
        return -1; // Should never happen
    }

    static int generateRandomValue(int minValue, int avgValue, int maxValue, Random random) {
        // Using average to create more realistic variation
        var value = minValue + random.nextInt(maxValue - minValue + 1)
                + (int) (random.nextGaussian() * (maxValue - minValue) / 3);
        if ((value >= minValue) && (value < maxValue)) {
            return value;
        }
        return generateRandomValue(minValue, avgValue, maxValue, random);
    }
}