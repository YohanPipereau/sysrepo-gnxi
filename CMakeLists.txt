cmake_minimum_required(VERSION 2.8)
project(sysrepo-gnmi)

#C++11 is required by gNMI
set(CMAKE_CXX_STANDARD 11)

# DEPENDENCIES
##############

# Cmake find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CmakeModules")

include_directories(${LIBYANG_INCLUDE_DIRS})

# dependencies - libyang
find_package(PkgConfig) #official cmake module
find_package(JSONCPP REQUIRED) #use FindJSONCPP.cmake
#find_package(LibYANG REQUIRED)
pkg_check_modules(LIBYANG REQUIRED libyang-cpp)
pkg_check_modules(SYSREPO REQUIRED libSysrepo-cpp) #PkgConfig cmake module maccro

if(LIBJSONCPP_FOUND)
    message ("LIBJSONCPP found")
else()
    message (FATAL_ERROR "Cannot find LIBJSONCPP")
endif()

# DEPENDENCIES & COMPILATION OF GNMI PROTO
##########################################

# set compiler and linker flags
set(RIGOROUS_C_FLAGS "-Wlogical-op -Wold-style-cast")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11 -g ${RIGOROUS_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -std=c++11 -DNDEBUG -O2 ${RIGOROUS_C_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -std=c++11 -g -O0 ${RIGOROUS_C_FLAGS}")

add_subdirectory(proto)

# COMPILER & LINKER
###################

# Generate a compile_commands.json with compile options
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(GNMI_SRC src/main.cpp
             src/capabilities.cpp
             src/get.cpp
             src/set.cpp
             src/subscribe.cpp
             src/security/security.cpp
             src/encode/encode.cpp
             src/encode/encode.cpp
             src/encode/json_ietf.cpp
)

add_executable(gnmi_server ${GNMI_SRC})

target_include_directories(gnmi_server PRIVATE ${PROTOBUF_INCLUDE_DIR}
                             ${CMAKE_CURRENT_BINARY_DIR}/proto
                             ${LIBJSONCPP_INCLUDE_DIR}
                             ${SYSREPO_INCLUDE_DIRS})

# link gnmi_server executable with grpc, jsoncpp, sysrepo libraries
target_link_libraries(gnmi_server gnmi
                      ${LIBJSONCPP_LIBRARY}
                      ${SYSREPO_LIBRARIES}
                      ${LIBYANG_LIBRARIES})
