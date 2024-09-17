import java.util.logging.Logger;

import io.IOCommon;
import network.AdmissionStrategy;
import network.RoutingMetric;

public class Configuration {
    private String topologyFile;
    private String connectionsFile;
    private String routingTableFile;
    private String forwardingTableFile;
    private String pathsFile;
    private RoutingMetric metric;
    private AdmissionStrategy mode;

    private Logger logger = Logger.getLogger(Configuration.class.getName());

    public String getTopologyFile() {
        return topologyFile;
    }

    public String getConnectionsFile() {
        return connectionsFile;
    }

    public String getRoutingTableFile() {
        return routingTableFile;
    }

    public String getForwardingTableFile() {
        return forwardingTableFile;
    }

    public String getPathsFile() {
        return pathsFile;
    }

    public RoutingMetric getMetric() {
        return metric;
    }

    public AdmissionStrategy getMode() {
        return mode;
    }

    @Override
    public String toString() {
        return "Topology File: " + topologyFile + "; " +
                "Connections File: " + connectionsFile + "; " +
                "Routing Table File: " + routingTableFile + "; " +
                "Forwarding File: " + forwardingTableFile + "; " +
                "Paths File: " + pathsFile + "; " +
                "Metric: " + metric + "; " +
                "Mode: " + mode;
    }

    public Configuration importFromCLIArgs(String[] args) {
        logger.info("Parsing arguments");
        this.topologyFile = IOCommon.parseArguments(args, "top");
        this.connectionsFile = IOCommon.parseArguments(args, "conn");
        this.routingTableFile = IOCommon.parseArguments(args, "rt");
        this.forwardingTableFile = IOCommon.parseArguments(args, "ft");
        this.pathsFile = IOCommon.parseArguments(args, "path");
        String metricStr = IOCommon.parseArguments(args, "flag");
        this.metric = metricStr.equals("hop") ? RoutingMetric.HOP : RoutingMetric.DISTANCE;
        String modeStr = IOCommon.parseArguments(args, "p");
        this.mode = modeStr.equals("0") ? AdmissionStrategy.OPTIMISTIC : AdmissionStrategy.PESSIMISTIC;
        logger.info(toString());
        logger.info("Arguments parsed successfully");
        return this;
    }
}
