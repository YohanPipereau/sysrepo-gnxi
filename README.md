# sysrepo-gnmi

# Description

A C++ server based on [gNMI specification](https://github.com/openconfig/reference/blob/master/rpc/gnmi/gnmi-specification.md) to communicate with [sysrepo](http://www.sysrepo.org/) datastore.

Supported RPCs:

* [x] Capabilities
* [X] Set
* [ ] Get
* [ ] Subscribe

Supported encoding:

* [x] No encoding
* [X] JSON IETF encoding (implies JSON encoding)
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

Why Protobuf/Binary/ASCII encoding are not supported?
=====================================================

Protobuf, binary and ASCII encodings requires gNMI client and server to have a convention regarding the data exchanged. Though, RFC7951 describes JSON encoding for YANG models and JSON is self describing, so both clients and server nows how data should be encoded.

What is no encoding? When should I use it?
==========================================

No encoding is using protobuf types (i.e. basic programming languages types).

It should be used if:

* Your gNMI client support raw types;
* You are making a Get or Subscribe Request;
* You are making a Set request with a XPATH qualifying a YANG leaf;
* You want to exchange lighter messages (typically for telemetry).
