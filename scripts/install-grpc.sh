#!/bin/bash

BR="/tmp"
VER="v1.18.0"

##########
# GRPC++ #
##########

mkdir -p ${BR}/downloads/ && cd ${BR}/downloads/

git clone --depth=1 -b ${VER} https://github.com/grpc/grpc
cd grpc && git submodule update --init

#install protobuf
cd third_party/protobuf
./autogen.sh && ./configure && make install

ldconfig

#install grpc
cd ../.. && make && make install
