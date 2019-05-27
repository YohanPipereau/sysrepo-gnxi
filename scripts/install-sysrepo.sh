#!/bin/bash

BR="/tmp"

###########
# SYSREPO #
###########

mkdir -p ${BR}/downloads/ && cd ${BR}/downloads/

wget https://github.com/sysrepo/sysrepo/archive/v0.7.7.tar.gz
tar xvf v0.7.7.tar.gz && cd sysrepo-0.7.7

mkdir -p build && cd build

#Without NACM
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr \
-DGEN_LANGUAGE_BINDINGS=ON -DGEN_CPP_BINDINGS=ON -DGEN_LUA_BINDINGS=OFF \
-DGEN_PYTHON_BINDINGS=OFF -DGEN_JAVA_BINDINGS=OFF -DBUILD_EXAMPLES=OFF \
-DENABLE_TESTS=OFF ..

#With NACM
#cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr \
#-DGEN_LANGUAGE_BINDINGS=ON -DGEN_CPP_BINDINGS=ON -DGEN_LUA_BINDINGS=OFF \
#-DGEN_PYTHON_BINDINGS=OFF -DGEN_JAVA_BINDINGS=OFF -DBUILD_EXAMPLES=OFF \
#-DENABLE_TESTS=OFF -DENABLE_NACM=ON ..

make
make install
