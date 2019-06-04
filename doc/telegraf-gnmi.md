# Telegraf with gNMI plugin

Telegraf supports `Subscribe` RPC.

Install and run `influxdb` & `chronograf`. Then open your browser to have `chronograf` web page.

## Create your telegraf configuration:

```
[[inputs.cisco_telemetry_gnmi]]
  ## List of device addresses to collect telemetry from
  addresses = ["localhost:50051"]

  ## Authentication details. Username and password are must if device expects
  ## authentication. Client ID must be unique when connecting from multiple instances
  ## of telegraf to the same device
  username = "cisco"
  password = "cisco"

  ## x509 Certificate to use with TLS connection. If it is not provided, an insecure
  ## channel will be opened with server
  # tls = true
  # tls_ca = "/cert.pem"

  ## Encoding json, json_ietf, proto
  # encoding = "json_ietf"
  # encoding = "json"
  encoding = "json_ietf"

  [[inputs.cisco_telemetry_gnmi.subscription]]
    path = "/ietf-interfaces:interfaces-state"

    # Subscription mode (one of: "target_defined", "sample", "on_change") and interval
    subscription_mode = "sample"
    sample_interval = "4s"

    ## Suppress redundant transmissions when measured values are unchanged
    # suppress_redundant = false

    ## If suppression is enabled, send updates at least every X seconds anyway
    # heartbeat_interval = "60s"

[[outputs.file]]
  files = ["stdout"]

[[outputs.influxdb]]
  url = "http://localhost:8086"
  database = "telemetry"
  precision = "s"
```

## Run telegraf

```
telegraf --config telegraf.conf
```

