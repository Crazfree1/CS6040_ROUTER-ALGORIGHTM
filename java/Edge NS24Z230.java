package network;

public class Edge {
    int node;
    int delay;
    double capacity;
    double availableCapacity;
    int nextVCID = 0;

    public int getNode() {
        return node;
    }

    public void setDelay(int delay) {
        this.delay = delay;
    }

    public int getDelay() {
        return delay;
    }

    public double getCapacity() {
        return capacity;
    }

    public double availableCapacity() {
        return availableCapacity;
    }

    public int getNextVCID() {
        return nextVCID;
    }

    public Edge(int node, int delay, double capacity) {
        this.node = node;
        this.delay = delay;
        this.capacity = capacity;
        this.availableCapacity = capacity;
    }

    public void allocateCapacity(double bandwidth) {
        this.availableCapacity -= bandwidth;
        this.nextVCID++;
    }

    public Edge clone() {
        return new Edge(node, delay, availableCapacity);
    }
}