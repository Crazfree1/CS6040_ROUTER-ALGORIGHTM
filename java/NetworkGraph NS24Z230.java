package network;

public class NetworkGraph {
    private int node1;
    private int node2;
    private int delay;
    private int capacity;

    public NetworkGraph(int node1, int node2, int delay, int capacity) {
        this.node1 = node1;
        this.node2 = node2;
        this.delay = delay;
        this.capacity = capacity;
    }

    public int getNode1() {
        return node1;
    }

    public int getNode2() {
        return node2;
    }

    public int getDelay() {
        return delay;
    }

    public int getCapacity() {
        return capacity;
    }

    // toString method for easy printing
    @Override
    public String toString() {
        return "NetworkGraph{" +
                "node1=" + node1 +
                ", node2=" + node2 +
                ", delay=" + delay +
                ", capacity=" + capacity +
                '}';
    }
}
