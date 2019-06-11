# Examples

## Capabilities RPC

```
gnmi -addr localhost:50051 capabilities
```

## Set RPC

Create a JSON file named tmp.json :

```
{
    "ietf-interfaces:interfaces": {
        "interface": [
            {
                "name": "GigabitEthernet0/8/0",
                "type": "iana-if-type:ethernetCsmacd",
                "enabled": false
            }
        ]
    }
}
```

```
gnmi -addr localhost:50051 update / tmp.json
```

## Get RPC:

```
gnmi -addr localhost:50051 get origin=ietf-interfaces /interfaces
gnmi -addr localhost:50051 get /ietf-interfaces:interfaces/ 
gnmi -addr localhost:50051 get /ietf-interfaces:interfaces/ /openconfig-interfaces:interfaces
```

