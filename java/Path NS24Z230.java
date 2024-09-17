package network;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

public class Path {
    List<Integer> nodes;
    int delay;
    int hops;

    public Path(List<Integer> nodes, int delay) {
        this.nodes = nodes;
        this.delay = delay;
        this.hops = nodes.size() - 1;
    }

    public String getPlainPathString() {
        return nodes.stream()
                .map(String::valueOf)
                .collect(Collectors.joining(" -> "));
    }

    public String getRoutingTableEntry() {
        return nodes.get(0) + ","
                + nodes.get(nodes.size() - 1) + ","
                + getPlainPathString() + ","
                + getDelay() + ","
                + getHops();
    }

    @Override
    public String toString() {
        return getPlainPathString()
                + "; hops: " + hops
                + "; delay: " + delay + ";";
    }

    public List<Integer> getNodes() {
        return nodes;
    }

    public int getDelay() {
        return delay;
    }

    public int getHops() {
        return hops;
    }

    public boolean doesLinkExist(int source, int destination) {
        for (int i = 0; i < nodes.size() - 1; i++) {
            if (nodes.get(i) == source && nodes.get(i + 1) == destination) {
                return true;
            }
        }
        return false;
    }

    public Path reverse() {
        var reversePathNodes = new ArrayList<Integer>();
        for (int i = nodes.size() - 1; i >= 0; i--) {
            reversePathNodes.add(nodes.get(i));
        }
        var reversePath = new Path(reversePathNodes, delay);
        return reversePath;
    }

    public List<Pair> getLinks() {
        var links = new ArrayList<Pair>();
        for (int i = 0; i < nodes.size() - 1; i++) {
            links.add(new Pair(nodes.get(i), nodes.get(i + 1)));
        }
        return links;
    }
}
