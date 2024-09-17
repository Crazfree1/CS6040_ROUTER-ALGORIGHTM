package network;

public class ConnectionRequest {
    int src;
    int dest;
    double bMin;
    double bAve;
    double bMax;

    public ConnectionRequest(int src, int dest, double bMin, double bAve, double bMax) {
        this.src = src;
        this.dest = dest;
        this.bMin = bMin;
        this.bAve = bAve;
        this.bMax = bMax;
    }

    @Override
    public String toString() {
        return src + " -> " + dest + " : [" + bMin + ", " + bAve + ", " + bMax + "]";
    }
}