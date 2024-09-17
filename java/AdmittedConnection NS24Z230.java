package network;

import java.util.List;

public class AdmittedConnection {
    int id;
    ConnectionRequest request;
    Path path;
    List<Integer> vcidList;

    public AdmittedConnection(int id, ConnectionRequest request, Path path, List<Integer> vcidList) {
        this.id = id;
        this.request = request;
        this.path = path;
        this.vcidList = vcidList;
    }

    @Override
    public String toString() {
        return "ID: " + id + ", Path: " + path.toString() +
                ", VCIDs: " + vcidList.toString() +
                ", Request: " + request.toString();
    }
}