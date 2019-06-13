# sysrepo-gnxi

# Description

A C++ server based on [gNMI specification](https://github.com/openconfig/reference/blob/master/rpc/gnmi/gnmi-specification.md) to communicate with [sysrepo](http://www.sysrepo.org/) datastore.

Supported RPCs:

* [x] Capabilities
* [X] Set
* [X] Get
* [X] Subscribe

Supported encoding:

* [ ] No encoding/gNMI native encoding (use `PROTO`)
* [X] JSON IETF encoding  (use `JSON_IETF`)
* [X] JSON encoding (if you ask for `JSON` you will have `JSON_IETF`)
* [ ] ~~Protobuf encoding~~
* [ ] ~~Binary encoding~~
* [ ] ~~ASCII encoding~~

Supported authentication/encryption methods:

* [x] no encryption, no username/password (DEBUGGING ONLY)
* [ ] ~~no encryption, username/password~~
* [ ] ~~TLS/SSL encryption only~~ : server key pair only
* [x] TLS/SSL encryption + Username/password authentication: server key pair only
* [x] TLS/SSL encryption & authentication: server and client key pairs (RECOMMENDED)

# Dependencies

```
sysrepo-gnxi
+-- protobuf (>=3.0) #because of gnmi
+-- jsoncpp #because of get JSON
+-- grpc (cpp) (>=1.18.0) #because of TLS bug to verify client cert
+-- libyang-cpp (>=1.0-r3) #because of feature_enable
+-- sysrepo-cpp (>=0.7.7)
|   +-- libyang
|   +-- ...
```

You can either install dependencies from sources or from the packages.

Install dependencies from source:

1. If `libyang (>=1.0-r3)` is packaged on your distrib use it, else run `scripts/install-libyang.sh` to install the required version of libyang. _you can use an older version and apply commit bf1aa13ba2dfb7b5938ed2345a67de316fc34917 to it_
2. You can run `scripts/install-sysrepo.sh` to install sysrepo. Check [here](https://github.com/sysrepo/sysrepo/blob/master/INSTALL.md) for installation instructions of sysrepo.

By default, grpc and protobuf are linked statically. But you can build it to have them linked dynamically.

# Install

## Install from package:

Install deb and rpm from https://github.com/YohanPipereau/sysrepo-gnxi/releases

## Install from source:

```
mkdir -p build
cd build
cmake -D DYNAMIC_LINK_GRPC=OFF .. # GRPC can be linked dynamically if other applications are using it
make
make install
```

# Build packages

Packages are built with Cpack module for cmake:

## Build DEB package:

```
mkdir -p build
cd build
cmake -D CPACK_GENERATOR="DEB" ..
make package
```

## Build RPM package:

```
mkdir -p build
cd build
cmake -D CPACK_GENERATOR="RPM" ..
make package
```

# Generate PKI (Recommanded)

On CA machine:

```
#Generate a self-signed certificate for CA if you don’t have one
openssl  req -x509 -days  365 -nodes  -newkey  rsa:2048 -keyout  ca.key -out ca.crt
```

On gNXI server:

```
#Generate a private key & certificate request for server
#CN (Common Name) or FQDN is mandatory and must be server domain name
openssl  req -new -days  365 -nodes  -newkey  rsa:2048 -keyout  server.key -out  server.certreq.csr
cat server.certreq.csr | grep "CERTIFICATE REQUEST" #check it is a cert request
```

On CA machine:

```
#CA sign server certificate request:
openssl  x509 -req -days  360 -in server.certreq.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -sha256
```

On client machine:

```
#Generate client key, certificate request, make CA sign it
#FQDN/CN is used for MUTUAL TLS authentication. So CN will be the identiy used for access control
openssl  req -new -days  365 -nodes  -newkey  rsa:2048 -keyout  client.key -out  client.certreq.csr
```

On CA machine:

```
#CA sign client certificate request:
openssl x509 -req -days 360 -in client.certreq.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -sha256
```

Verify the informations:
```
#Check information
openssl x509 -noout -subject -issuer -in server.crt
openssl x509 -noout -subject -issuer -in client.crt

# Verify chain certs
openssl verify -CAfile ca.crt -show_chain client.crt
openssl verify -CAfile ca.crt -show_chain server.crt
```

# Get started

* Server/client in INSECURE mode (no username/password) and no TLS connection
```
gnxi_server -f
gnmi -addr localhost:50051 get /ietf-interfaces:interfaces-state
```

* Server/client with TLS connection for encryption and authentication:
```
gnxi_server -k server.key -c server.crt -l4
gnmi -addr localhost:50051 -certfile=client.crt -keyfile=client.key get /ietf-interfaces:interfaces-state
```

* Server/client with username/password + TLS connection for encryption only:
```
gnxi_server -k server.key -c server.crt --username cisco --password cisco -l4
gnmi -addr localhost:50051 -cafile ca.crt -username cisco -password cisco get /ietf-interfaces:interfaces-state
```

# Clients

Here is a list of gNMI clients, not all of them work because they don't all respect the specification.

## gnmi clients for Capabilities, Set, Get:

* Arista [gnmi](https://github.com/aristanetworks/goarista/tree/master/cmd/gnmi)
* Openconfig [gnmi_cli](https://github.com/openconfig/gnmi)
* Google [gnmi_capabilities](https://github.com/google/gnxi/tree/master/gnmi_capabilities), [gnmi_get](https://github.com/google/gnxi/tree/master/gnmi_get), [gnmi_set](https://github.com/google/gnxi/tree/master/gnmi_set)

## gnmi clients for Subscribe (telemetry):

* InfluxData [Telegraf](https://github.com/influxdata/telegraf)
* Cisco [pipeline-gnmi](https://github.com/cisco-ie/pipeline-gnmi)
* Openconfig [gnmi_collector](https://github.com/openconfig/gnmi/tree/master/cmd/gnmi_collector)
* Nokia [pygnmi](https://github.com/nokia/pygnmi)

Documentation and examples for:

* [gnmi](https://github.com/aristanetworks/goarista/tree/master/cmd/gnmi) is provided [here](doc/gnmi_cli.md)
* [Telegraf-gnmi](https://github.com/influxdata/telegraf) is provided [here](doc/telegraf-gnmi.md)
* [pipeline-gnmi](https://github.com/influxdata/telegraf) is provided [here](doc/pipeline-gnmi.md)

gNMI clients which should work:

* [gnmi](https://github.com/aristanetworks/goarista/tree/master/cmd/gnmi)
* [Telegraf-gnmi](https://github.com/influxdata/telegraf)


# FAQ

## Why does it use libyang rather than a JSON library?

A JSON library would not be enough because JSON is not detailed enough and miss information required to build an XPATH (key of sysrepo datastore).

Typically, in a JSON encoded format containing a list, you don't know which field would be the YANG key of the list element.

Ex: /ietf-interfaces:interfaces/interface contains multiple leaves. "name" is the key but JSON does not give this information.

## What is the problem with JSON and JSON_IETF encodings?

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


## What is the problem using different encoding to store xpath in a database?

The problem is that all encodings do not agree on the type which must be used to store a value.
For example:
`/ietf-interfaces:interfaces/interface[name="eth0"]/statistics/octets` can be stored as a `uint64_t` with no encoding or as a `string` with JSON IETF encoding.

## Why Protobuf/Binary/ASCII encoding are not supported?

Protobuf, binary and ASCII encodings requires gNMI client and server to have a convention regarding the data exchanged.
On the contrary, JSON IETF [RFC7951] and JSON are self describing, so both clients and server nows how data is encoded (Key:Value).

## What is no encoding? When should I use it?

No encoding means your value can be encoded in one of the following type i.e. string, int64, uint64, bool, bytes, float, Decimal64.
This types are all supported types for a YANG leaf node.

If you are using "No Encoding", or "Native Encoding", you must know that it is not part of gNMI specification.
If you use this, you would receive a list of xpath and their value. (Though it is not supported for now).

It should be used if:

* Your gNMI client support raw types;
* You are making a Get or Subscribe Request;
* You are making a Set request with a XPATH qualifying a YANG leaf;
* You want to exchange lighter messages (typically for telemetry).

## Why do I need grpc 1.18.0 ?

This server compiles with grpc 1.12.0 but as reported here https://github.com/grpc/grpc/pull/17500 , if we want to use TLS for authentication and no root certificate is specified on server side, there will be no checking of client certificate . Thus, anyone could access the server without authenticating.

## Why linking statically grpc++ and protobuf by default ?

Because grpc++ is not packaged on Centos and Ubuntu/Debian, and it takes a long time to compile it.
