# sysrepo-gnmi

# Description

A C++ server based on [gNMI specification](https://github.com/openconfig/reference/blob/master/rpc/gnmi/gnmi-specification.md) to communicate with [sysrepo](http://www.sysrepo.org/) datastore.

Supported RPCs:

* [x] Capabilities
* [X] Set
* [X] Get
* [X] Subscribe

Supported encoding:

* [x] No encoding/gNMI native encoding (use `PROTO`)
* [X] JSON IETF encoding  (use `JSON_IETF`)
* [ ] ~~JSON encoding~~
* [ ] ~~Protobuf encoding~~
* [ ] ~~Binary encoding~~
* [ ] ~~ASCII encoding~~

Supported authentication/encryption methods:

* [x] TLS/SSL with username/password
* [x] TLS/SSL without username/password
* [x] username/password, no encryption
* [x] no username/password, no encryption

# Dependencies

```
sysrepo-gnmi
+-- protobuf (>=3.0) #because of gnmi
+-- jsoncpp #because of get JSON
+-- grpc (cpp)
+-- libyang (cpp >=1.0-r3) #because of feature_enable
+-- sysrepo (cpp)
|   +-- libyang
|   +-- ...
```

libyang is a dependency of _sysrepo-gnmi_ but it should be installed with sysrepo anyway. Though, make sure you have installed libyang with C++ library and header files.

Check [here](https://github.com/sysrepo/sysrepo/blob/master/INSTALL.md) for installation instructions of sysrepo.


# Install

You can run scripts/install.sh to install the required version of libyang or use an older version and apply commit bf1aa13ba2dfb7b5938ed2345a67de316fc34917 to it.

```
mkdir -p build
cd build
cmake ..
make
```

# Server

* Launch server without authentification nor encryption : `gnmi_server -f`

# Clients

gnmi clients for Capabilities, Set, Get:
========================================

* Arista [gnmi](https://github.com/aristanetworks/goarista/tree/master/cmd/gnmi)
* Openconfig [gnmi_cli](https://github.com/openconfig/gnmi)
* Google [gnmi_capabilities](https://github.com/google/gnxi/tree/master/gnmi_capabilities), [gnmi_get](https://github.com/google/gnxi/tree/master/gnmi_get), [gnmi_set](https://github.com/google/gnxi/tree/master/gnmi_set)

gnmi clients for Subscribe (telemetry):
=======================================

* InfluxData [Telegraf](https://github.com/influxdata/telegraf)
* Cisco [pipeline-gnmi](https://github.com/cisco-ie/pipeline-gnmi)
* Openconfig [gnmi_collector](https://github.com/openconfig/gnmi/tree/master/cmd/gnmi_collector)
* Nokia [pygnmi](https://github.com/nokia/pygnmi)

# Examples

## Capabilities RPC

```
./gnmi -addr localhost:50051 capabilities
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
./gnmi -addr localhost:50051 update / tmp.json
```

## Get RPC:

```
./gnmi -addr localhost:50051 get /ietf-interfaces:interfaces/ 
./gnmi -addr localhost:50051 get /ietf-interfaces:interfaces/ /openconfig-interfaces:interfaces
```

# FAQ

Why does it use libyang rather than a JSON library?
===================================================

A JSON library would not be enough because JSON is not detailed enough and miss information required to build an XPATH (key of sysrepo datastore).

Typically, in a JSON encoded format containing a list, you don't know which field would be the YANG key of the list element.

Ex: /ietf-interfaces:interfaces/interface contains multiple leaves. "name" is the key but JSON does not give this information.

What is the problem with JSON and JSON_IETF encodings?
======================================================

Your gnmi client for a `get` or `subscribe` rpc must have yang models downloaded if it wants to recognize which is the key from a JSON list.

Ex: Which field is the key for interface list ?

```
{
   "interface" : [
      {
         "admin-status" : "down",
         "name" : "GigabitEthernet0/8/0",
         "oper-status" : "down",
         "phys-address" : "08:00:27:60:b7:12",
         "speed" : "1000000",
         "type" : "iana-if-type:ethernetCsmacd"
      },
      {
         "admin-status" : "down",
         "name" : "local0",
         "oper-status" : "down",
         "phys-address" : "00:00:00:00:00:00",
         "speed" : "0",
         "type" : "iana-if-type:ethernetCsmacd"
      }
   ]
}
```


What is the problem using different encoding to store xpath in a database?
==========================================================================

The problem is that all encodings do not agree on the type which must be used to store a value.
For example:
`/ietf-interfaces:interfaces/interface[name="eth0"]/statistics/octets` can be stored as a `uint64_t` with no encoding or as a `string` with JSON IETF encoding.

Why Protobuf/Binary/ASCII encoding are not supported?
=====================================================

Protobuf, binary and ASCII encodings requires gNMI client and server to have a convention regarding the data exchanged.
On the contrary, JSON IETF [RFC7951] and JSON are self describing, so both clients and server nows how data is encoded (Key:Value).

What is no encoding? When should I use it?
==========================================

No encoding means your value can be encoded in one of the following type i.e. string, int64, uint64, bool, bytes, float, Decimal64.
This types are all supported types for a YANG leaf node.

If you are using "No Encoding", or "Native Encoding", you must know that it is not part of gNMI specification.
I took some liberty and if you use `PROTO` field of gnmi encoding enumeration.
Because it encode yang leaves, the xpath, you will receive are all xpath for YANG leaves.

It should be used if:

* Your gNMI client support raw types;
* You are making a Get or Subscribe Request;
* You are making a Set request with a XPATH qualifying a YANG leaf;
* You want to exchange lighter messages (typically for telemetry).
