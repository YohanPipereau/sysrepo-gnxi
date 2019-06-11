# DEPENDENCIES
##############

# Require at least Protobuf 3 for gNMI
find_package(Protobuf 3.0.0 REQUIRED) #official cmake module
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# gRPC 1.12.0 will compile but to prevent a security bug, use 1.18.0
# We leverage grpc pkg-config files in /usr/local/lib/pkgconfig
find_package(PkgConfig REQUIRED)
pkg_check_modules(grpc++ REQUIRED grpc++>=1.18.0)

# GENERATION OF SOURCE FILES
############################

# Official gRPCConfig.cmake is installed by gRPC only if compiled with cmake
# We just use FindGRPC.cmake to have PROTOBUF_GENERATE_GRPC_CPP function
find_package(GRPC)

get_filename_component(gnmi_proto "gnmi.proto" ABSOLUTE)
get_filename_component(gnmi_ext_proto "gnmi_ext.proto" ABSOLUTE)

# Official generator
protobuf_generate_cpp(gnmi_proto_srcs gnmi_proto_hdrs ${gnmi_proto})
protobuf_generate_cpp(gnmi_ext_proto_srcs gnmi_ext_proto_hdrs ${gnmi_ext_proto})

# Custom generator: Official `protobuf_generate_cpp` can't use grpc plugins
PROTOBUF_GENERATE_GRPC_CPP(gnmi_grpc_srcs gnmi_grpc_hdrs ${gnmi_proto})
PROTOBUF_GENERATE_GRPC_CPP(gnmi_ext_grpc_srcs gnmi_ext_grpc_hdrs ${gnmi_ext_proto})

# LINK OPTIONS FOR GNMI-SERVER
##############################

#Create a new library named gnmi
add_library(gnmi ${gnmi_grpc_srcs} ${gnmi_ext_grpc_srcs} ${gnmi_proto_srcs} ${gnmi_ext_proto_srcs})

set(DYNAMIC_LINK_GRPC OFF)

if(DYNAMIC_LINK_GRPC)
    message(STATUS "DYNAMICALLY LINK gRPC")
    #Link our new gnmi library with grpc and protobuf libraries
    target_link_libraries(gnmi ${grpc++_LIBRARIES} ${PROTOBUF_LIBRARIES})
else()
    message(STATUS "STATICALLY LINK gRPC")
    find_library(GRPCPP_STATIC_LIB NAMES libgrpc++.a PATHS ${grpc++_LIBRARY_DIRS})
    if(NOT GRPCPP_STATIC_LIB)
        message(FATAL_ERROR "static libraries for grpc++ not found")
    endif()
    find_library(GRPC_STATIC_LIB NAMES libgrpc.a PATHS ${grpc++_LIBRARY_DIRS})
    if(NOT GRPC_STATIC_LIB)
        message(FATAL_ERROR "static libraries for grpc++ not found")
    endif()
    find_library(PROTOBUF_STATIC_LIB NAMES libprotobuf.a PATHS ${grpc++_LIBRARY_DIRS})
    if(NOT PROTOBUF_STATIC_LIB)
        message(FATAL_ERROR "static libraries for protobuf not found")
    endif()
    pkg_check_modules(zlib REQUIRED zlib)
    pkg_check_modules(openssl REQUIRED openssl)

    #Link our new gnmi library with grpc, protobuf libraries and their dependencies
    target_link_libraries(gnmi
        ${GRPCPP_STATIC_LIB} ${GRPC_STATIC_LIB}
        ${PROTOBUF_STATIC_LIB}
        ${openssl_LIBRARIES} ${zlib_LIBRARIES}
    )
endif(DYNAMIC_LINK_GRPC)
