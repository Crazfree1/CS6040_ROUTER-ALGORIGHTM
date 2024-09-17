package network;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Set;

public class BFSPathFinder extends PathFinder {
    Network network;

    public BFSPathFinder(Network network) {
        this.network = network;
    }

    public List<List<Integer>> findPathsStrategy(int start, int end) {
        Queue<List<Integer>> queue = new LinkedList<>();
        Set<Integer> visited = new HashSet<>();
        List<List<Integer>> resultPaths = new ArrayList<>();

        queue.add(Collections.singletonList(start));

        while (!queue.isEmpty() && resultPaths.size() < 2) {
            List<Integer> path = queue.poll();
            int currentNode = path.get(path.size() - 1);

            if (currentNode == end) {
                resultPaths.add(new ArrayList<>(path));
                if (resultPaths.size() == 1) {
                    visited.clear();
                    continue;
                } else {
                    break;
                }
            }

            if (!visited.add(currentNode)) {
                continue;
            }

            for (Edge edge : network.getEdges().get(currentNode)) {
                int neighbor = edge.getNode();

                List<Integer> newPath = new ArrayList<>(path);
                newPath.add(neighbor);
                queue.add(newPath);
            }
        }
        return resultPaths;
    }

}
