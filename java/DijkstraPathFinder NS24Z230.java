package network;

import java.util.*;

public class DijkstraPathFinder extends PathFinder {

    private Network network;

    public DijkstraPathFinder(Network network) {
        this.network = network;
    }

    List<List<Integer>> findPathsStrategy(int start, int end) {
        // First shortest path
        PriorityQueue<int[]> pq = new PriorityQueue<>(Comparator.comparingInt(a -> a[1]));
        Map<Integer, Integer> distances = new HashMap<>();
        Map<Integer, Integer> previous = new HashMap<>();
        Set<Integer> visited = new HashSet<>();

        for (int node : network.getEdges().keySet()) {
            distances.put(node, Integer.MAX_VALUE);
            previous.put(node, -1);
        }

        distances.put(start, 0);
        pq.add(new int[] { start, 0 });

        while (!pq.isEmpty()) {
            int[] current = pq.poll();
            int currentNode = current[0];
            if (currentNode == end)
                break;

            if (!visited.add(currentNode))
                continue;

            for (Edge edge : network.getEdges().get(currentNode)) {
                int neighbor = edge.getNode();
                int weight = edge.getDelay();

                int newDist = distances.get(currentNode) + weight;
                if (newDist < distances.get(neighbor)) {
                    distances.put(neighbor, newDist);
                    previous.put(neighbor, currentNode);
                    pq.add(new int[] { neighbor, newDist });
                }
            }
        }

        // Reconstruct first path
        List<Integer> firstPath = new ArrayList<>();
        for (Integer at = end; at != -1; at = previous.get(at)) {
            firstPath.add(at);
        }
        Collections.reverse(firstPath);

        // Apply penalties and find the second path
        pq.clear();
        distances.clear();
        previous.clear();
        visited.clear();

        for (int node : network.getEdges().keySet()) {
            distances.put(node, Integer.MAX_VALUE);
            previous.put(node, -1);
        }

        distances.put(start, 0);
        pq.add(new int[] { start, 0 });

        while (!pq.isEmpty()) {
            int[] current = pq.poll();
            int currentNode = current[0];
            if (currentNode == end)
                break;

            if (!visited.add(currentNode))
                continue;

            for (Edge edge : network.getEdges().get(currentNode)) {
                int neighbor = edge.getNode();
                int weight = edge.getDelay();

                // Apply a large penalty to the first path
                if (doesLinkExistInPath(firstPath, currentNode, neighbor)) {
                    weight += 1000;
                }

                int newDist = distances.get(currentNode) + weight;
                if (newDist < distances.get(neighbor)) {
                    distances.put(neighbor, newDist);
                    previous.put(neighbor, currentNode);
                    pq.add(new int[] { neighbor, newDist });
                }
            }
        }

        // Reconstruct second path
        List<Integer> secondPath = new ArrayList<>();
        for (Integer at = end; at != -1; at = previous.get(at)) {
            secondPath.add(at);
        }
        Collections.reverse(secondPath);

        List<List<Integer>> paths = new ArrayList<>();
        paths.add(firstPath);
        paths.add(secondPath);

        return paths;
    }

    private boolean doesLinkExistInPath(List<Integer> path, int source, int destination) {
        for (int i = 0; i < path.size() - 1; i++) {
            if (path.get(i) == source && path.get(i + 1) == destination) {
                return true;
            }
        }
        return false;
    }
}
