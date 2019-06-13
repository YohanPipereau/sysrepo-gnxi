cmake_minimum_required(VERSION 3.0)

project(sysrepo-gnxi)

## AVAILABLE OPTIONS FOR CMAKE:
##  DYNAMIC_LINK_GRPC = OFF     , link grpc and protobuf dynamically
##  CPACK_GENERATOR = [DEB,RPM] , package for deb/rpm

set(SYSREPO-GNXI_MAJOR_VERSION 0)
set(SYSREPO-GNXI_MINOR_VERSION 1)
set(SYSREPO-GNXI_MICRO_VERSION 0)

set(SYSREPO-GNXI_VERSION ${SYSREPO-GNXI_MAJOR_VERSION}.${SYSREPO-GNXI_MINOR_VERSION}.${SYSREPO-GNXI_MICRO_VERSION})

#C++11 is required by gNXI
set(CMAKE_CXX_STANDARD 11)

# DEPENDENCIES
##############

# Cmake find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CmakeModules")

find_package(PkgConfig) #official cmake module
find_package(Boost REQUIRED log system) #just boost-log and boost-system libraries

pkg_check_modules(JSONCPP REQUIRED jsoncpp) #official pkgconfig jsoncpp
pkg_check_modules(LIBYANG REQUIRED libyang-cpp)
pkg_check_modules(SYSREPO REQUIRED libSysrepo-cpp=>0.7.7) #PkgConfig cmake module maccro


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
             src/security/authentication.cpp
             src/utils/log.cpp
             src/gnmi/gnmi.cpp
             src/gnmi/capabilities.cpp
             src/gnmi/get.cpp
             src/gnmi/set.cpp
             src/gnmi/subscribe.cpp
             src/gnmi/encode/encode.cpp
             src/gnmi/encode/load_models.cpp
             src/gnmi/encode/runtime.cpp
             src/gnmi/encode/json_ietf.cpp
)

add_executable(gnxi_server ${GNXI_SRC})

#Header file location required to build target
target_include_directories(gnxi_server
    PUBLIC #List of include dirs required to use target binary or library
        ${Boost_INCLUDE_DIRS}
        ${JSONCPP_INCLUDE_DIRS}
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
                      ${JSONCPP_LIBRARIES}
                      ${Boost_LIBRARIES}
                      ${SYSREPO_LIBRARIES}
                      ${LIBYANG_LIBRARIES}
)

# INSTALLATION
##############

install(TARGETS gnxi_server
        RUNTIME DESTINATION bin
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# PACKAGING
###########

SET(CPACK_PACKAGE_VENDOR "FD.io")
SET(CPACK_PACKAGE_CONTACT "ypiperea@cisco.com")
SET(CPACK_PACKAGE_VERSION_MAJOR ${SYSREPO-GNXI_MAJOR_VERSION})
SET(CPACK_PACKAGE_VERSION_MINOR ${SYSREPO-GNXI_MINOR_VERSION})
SET(CPACK_PACKAGE_VERSION_PATCH ${SYSREPO-GNXI_MICRO_VERSION})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A gNMI server for sysrepo YANG datastore.")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "https://github.com/YohanPipereau/sysrepo-gnxi")

#DEBIAN specific : SET(CPACK_GENERATOR "DEB")
if(CPACK_GENERATOR EQUAL "DEB")
    message(STATUS "DEB packaging selected")
    SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "x86_64")
    SET(CPACK_DEBIAN_PACKAGE_SECTION "misc")
    SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${CPACK_SOURCE_PACKAGE_FILE_NAME})
    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-thread, libboost-log, libboost-system, libjsoncpp, libstdc++6 (>= 5.2), zlib1g, libssl, libyang-cpp0.16, sysrepo-cpp")
endif(CPACK_GENERATOR EQUAL "DEB")

#RPM specific : SET(CPACK_GENERATOR "RPM")
if(CPACK_GENERATOR EQUAL "RPM")
    message(STATUS "RPM packaging selected")
    SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
    SET(CPACK_RPM_PACKAGE_URL ${CPACK_SOURCE_PACKAGE_FILE_NAME})
    SET(CPACK_RPM_PACKAGE_REQUIRES "boost, jsoncpp, libstdc++6 (>= 5.2), zlib, openssl-devel")
endif(CPACK_GENERATOR EQUAL "RPM")

INCLUDE(CPack) #run cpack
