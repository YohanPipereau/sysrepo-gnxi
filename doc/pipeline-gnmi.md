# pipeline-gnmi

pipeline-gnmi support `Subscribe` RPC. They use `PROTO` to require a list of YANG leaves XPAtH and their associated values (which is not the real PROTO encoding as defined in gNMI specification).

pipeline-gnmi does not support JSON yet as of version 2.0.

Here is an example configuration of pipeline-gnmi:

```
#########################
# Configure data source #
#########################

[vpp-instance1]
stage = xport_input
type = gnmi
server = localhost:50051

# Sensor Path to subscribe to. No configuration on the device necessary
# Appending an @ with a parameter specifies subscription type:
#   @x where x is a positive number indicates a fixed interval, e.g. @10 -> every 10 seconds
#   @change indicates only changes should be reported
#   omitting @ and parameter will do a target-specific subscriptions (not universally supported)
#
path1 = /ietf-interfaces:interfaces-state@2
#path2 = /interfaces/interface/state@change

# Whitelist the actual sensor values we are interested in (1 per line) and drop the rest.
# This replaces metrics-based filtering for gNMI input - which is not implemented.
# Note: Specifying one or more selectors will drop all other sensor values and is applied for all paths.
#select1 = Cisco-IOS-XR-infra-statsd-oper:infra-statistics/interfaces/interface/latest/generic-counters/packets-sent
#select2 = Cisco-IOS-XR-infra-statsd-oper:infra-statistics/interfaces/interface/latest/generic-counters/packets-received

# Suppress redundant messages (minimum hearbeat interval)
# If set and 0 or positive, redundant messages should be suppressed by the server
# If greater than 0, the number of seconds after which a measurement should be sent, even if no change has occured
#heartbeat_interval = 0

#tls = false
#username = cisco
#password = ...

##################
# Export metrics #
##################

[metrics]
stage=xport_output
type=metrics
output=influx
influx=http://localhost:8086

#It is not used but need to be passed:
file=/etc/pipeline/metrics.json
database=pipeline_telem
#username=admin
#password=admin

dump=influxdump.txt
```

# Run pipeline-gnmi

`./pipeline -config pipeline.conf`
