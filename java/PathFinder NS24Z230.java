package network;

import java.util.List;
import java.util.Set;
import java.util.HashSet;
import java.util.logging.Logger;

public abstract class PathFinder {
    private Logger logger = Logger.getLogger(DijkstraPathFinder.class.getName());

    abstract List<List<Integer>> findPathsStrategy(int start, int end);

    public List<List<Integer>> findPaths(int start, int end) {
        var resultPaths = findPathsStrategy(start, end);
        sanitizePaths(resultPaths);
        logger.info("Paths found");
        logger.info(resultPaths.toString());
        return resultPaths;
    }

    void sanitizePaths(List<List<Integer>> resultPaths) {
        resultPaths.removeIf(path -> {
            Set<Integer> uniqueElements = new HashSet<>(path);
            return uniqueElements.size() != path.size();
        });
        resultPaths.removeIf(path -> {
            return path.size() < 2;
        });
        if (resultPaths.size() > 1
                && resultPaths.get(0).equals(resultPaths.get(1))) {
            resultPaths.remove(1);
        }
    }
}
