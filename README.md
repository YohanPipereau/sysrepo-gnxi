# sysrepo-gnmi

# Description

A C++ server based on [gNMI specification](https://github.com/openconfig/reference/blob/master/rpc/gnmi/gnmi-specification.md) to communicate with [sysrepo](http://www.sysrepo.org/) datastore.

Supported RPCs:

* [x] Capabilities
* [ ] Set
* [ ] Get
* [ ] Subscribe

Supported encoding:

* [x] No encoding
* [ ] JSON IETF encoding (implies JSON encoding)
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
+-- sysrepo
|   +-- libyang
|   +-- ...
```

libyang is a dependency of _sysrepo-gnmi_ but it should be installed with sysrepo anyway. Though, make sure you have installed libyang with C++ library and header files.

Check [here](https://github.com/sysrepo/sysrepo/blob/master/INSTALL.md) for installation instructions of sysrepo.


# Install

```
mkdir -p build
cd build
cmake ..
make
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
