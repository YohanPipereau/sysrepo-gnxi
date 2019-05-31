cmake_minimum_required(VERSION 2.8)
project(sysrepo-gnxi)

#C++11 is required by gNXI
set(CMAKE_CXX_STANDARD 11)

# DEPENDENCIES
##############

# Cmake find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CmakeModules")

# dependencies - libyang
find_package(PkgConfig) #official cmake module
find_package(jsoncpp REQUIRED) #use official jsoncppConfig cmake
find_package(Boost REQUIRED log system)
pkg_check_modules(LIBYANG REQUIRED libyang-cpp)
pkg_check_modules(SYSREPO REQUIRED libSysrepo-cpp) #PkgConfig cmake module maccro


# DEPENDENCIES & COMPILATION OF GNXI PROTO
##########################################

# set compiler and linker flags
set(RIGOROUS_C_FLAGS "-Wlogical-op -Wold-style-cast")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11 -g ${RIGOROUS_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -std=c++11 -DNDEBUG -O2 ${RIGOROUS_C_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -std=c++11 -g -O0 ${RIGOROUS_C_FLAGS}")

#Use Boost with Dynamic libraries rather than static linkage
add_definitions(-D BOOST_LOG_DYN_LINK)

# SUBDIR
########

add_subdirectory(proto)

# COMPILER & LINKER
###################

# Generate a compile_commands.json with compile options
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(GNXI_SRC src/main.cpp
             src/security/security.cpp
             src/utils/log.cpp
             src/gnmi/capabilities.cpp
             src/gnmi/get.cpp
             src/gnmi/set.cpp
             src/gnmi/subscribe.cpp
             src/gnmi/encode/encode.cpp
             src/gnmi/encode/encode.cpp
             src/gnmi/encode/json_ietf.cpp
)

add_executable(gnxi_server ${GNXI_SRC})

#Header file location required to build target
target_include_directories(gnxi_server
    PUBLIC #List of include dirs required to use target binary or library
        ${Boost_INCLUDE_DIRS}
        ${LIBJSONCPP_INCLUDE_DIR}
        ${LIBYANG_INCLUDE_DIRS}
        ${SYSREPO_INCLUDE_DIRS}
        ${PROTOBUF_INCLUDE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}     #include "build" directory tree for "build/proto"
        ${CMAKE_CURRENT_SOURCE_DIR}/src #include "src" tree for <utils/> <security/>
)

#Directory path to look for libraries
link_directories(${Boost_LIBRARY_DIRS})

# link gnxi_server executable with grpc, jsoncpp, sysrepo libraries
target_link_libraries(gnxi_server gnmi
                      ${LIBJSONCPP_LIBRARY}
                      ${Boost_LIBRARIES}
                      ${SYSREPO_LIBRARIES}
                      ${LIBYANG_LIBRARIES}
                      )
