cmake_minimum_required(VERSION 3.16)

set(LIBCSP_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/libcsp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SOCKETCAN libsocketcan REQUIRED)

# use the hint from about to find the location of socketcan
find_library(SOCKETCAN_LIBRARY
        NAMES socketcan
	PATHS ${SOCKETCAN_LIBRARY_DIRS}
        )

file(COPY patches/CMakeLists.txt patches/Config.cmake.in patches/csp_autoconfig.h.in DESTINATION ${LIBCSP_SOURCE_DIR})

add_subdirectory(${LIBCSP_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libcsp)
