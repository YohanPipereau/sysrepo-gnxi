#!/bin/bash

BR="/tmp"

###########
# LIBYANG #
###########

mkdir -p ${BR}/downloads/ && cd ${BR}/downloads/

# We need commit bf1aa13ba2dfb7b5938ed2345a67de316fc34917
git clone https://github.com/CESNET/libyang/
cd libyang

mkdir -p build && cd build

cmake -DCMAKE_BUILD_TYPE:String="Release" -DCMAKE_INSTALL_PREFIX:PATH=/usr \
-DGEN_LANGUAGE_BINDINGS=ON -DGEN_CPP_BINDINGS=ON \
-DGEN_PYTHON_BINDINGS=OFF -DBUILD_EXAMPLES=OFF \
-DENABLE_BUILD_TESTS=OFF ..

make -j$(nproc)
make install
