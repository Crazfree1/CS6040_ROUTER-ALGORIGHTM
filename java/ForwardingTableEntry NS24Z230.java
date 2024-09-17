package network;

public class ForwardingTableEntry {
    int node;
    int previous;
    int incomingVCID;
    int next;
    int outgoingVCID;

    ForwardingTableEntry(int node, int previous, int incomingVCID, int next, int outgoingVCID) {
        this.node = node;
        this.previous = previous;
        this.incomingVCID = incomingVCID;
        this.next = next;
        this.outgoingVCID = outgoingVCID;
    }

    public int getNode() {
        return node;
    }

    public int getPrevious() {
        return previous;
    }

    public int getIncomingVCID() {
        return incomingVCID;
    }

    public int getNext() {
        return next;
    }

    public int getOutgoingVCID() {
        return outgoingVCID;
    }

    @Override
    public String toString() {
        return node + "," + previous + "," + incomingVCID
                + "," + next + "," + outgoingVCID;
    }
}
