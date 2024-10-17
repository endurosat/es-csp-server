cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0048 NEW)

project(csp VERSION 1.6 LANGUAGES C)
set(CSP_VERSION 1)
set(CSP_RELEASE 1.6)
enable_language(C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -std=gnu11 -Wall -Wextra   -fanalyzer")
set(CMAKE_C_FLAGS_RELEASE "-std=gnu11 -Wall -Wextra -fanalyzer")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(PkgConfig REQUIRED)
pkg_get_variable(SYSTEMD_UNIT_DIR systemd systemdsystemunitdir)

# If no build type was specified, set it to Release.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
            "Choose the type of build, options are: None Debug Release."
            FORCE)
endif(NOT CMAKE_BUILD_TYPE)

include(${CMAKE_SOURCE_DIR}/cmake/install_paths.cmake)
if(USE_STATIC_ANALYSIS)
	include(${CMAKE_SOURCE_DIR}/cmake/analysis.cmake)
endif(USE_STATIC_ANALYSIS)

# Projectwise paths
set(PROJECT_PREFIX ${PROJECT_NAME})
set(INSTALL_RUNTIME_DIR ${CMAKE_INSTALL_BINDIR})
set(INSTALL_CONFIG_DIR  ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME})
set(INSTALL_LIBRARY_DIR ${CMAKE_INSTALL_LIBDIR})
set(INSTALL_ARCHIVE_DIR ${CMAKE_INSTALL_LIBDIR})
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_PREFIX})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(INSTALL_SYSTEMD_SERVICE ${CMAKE_INSTALL_PREFIX}/${SYSTEMD_UNIT_DIR})
else(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(INSTALL_SYSTEMD_SERVICE ${SYSTEMD_UNIT_DIR})
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include(TestBigEndian)
test_big_endian(CSP_ENDIANNESS)
if(CSP_ENDIANNESS)
set(CSP_LITTLE_ENDIAN 0)
set(CSP_BIG_ENDIAN 1)
else()
set(CSP_LITTLE_ENDIAN 1)
set(CSP_BIG_ENDIAN 0)
endif()
set(CSP_DEBUG 1)
set(CSP_DEFAULT_ROUTE 0)
set(CSP_HAVE_LIBSOCKETCAN 1)
set(CSP_USE_EXTERNAL_DEBUG 0)
set(CSP_USE_CRC32 1)
set(CSP_HAVE_STDIO 1)
set(CSP_ENABLE_CSP_PRINT 1)
set(CSP_PRINT_STDIO 0)
set(CSP_LOG_LEVEL_DEBUG 1)
set(CSP_QFIFO_LEN 15 CACHE STRING "Length of incoming queue for router task.")
set(CSP_PORT_MAX_BIND 16 CACHE STRING "Length of incoming queue for router task")
set(CSP_CONN_RXQUEUE_LEN 16 CACHE STRING "Number of packets in connection queue")
set(CSP_CONN_MAX 8 CACHE STRING "Number of new connections on socket queue")
set(CSP_BUFFER_SIZE 256 CACHE STRING "Bytes in each packet buffer")
set(CSP_BUFFER_COUNT 15 CACHE STRING "Number of total packet buffers")
set(CSP_RDP_MAX_WINDOW 5 CACHE STRING "Max window size for RDP")
set(CSP_RTABLE_SIZE 10 CACHE STRING "Number of elements in routing table")

# Options

option(CSP_USE_RDP "Reliable Datagram Protocol" On)
option(CSP_USE_HMAC "Hash-based message authentication code" OFF)
option(CSP_USE_PROMISC "Promiscious mode" OFF)
option(CSP_USE_DEDUP "Packet deduplication" OFF)

option(enable-python3-bindings "Build Python3 binding" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	  set(CSP_POSIX 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Zephyr")
	  set(CSP_ZEPHYR 1)
endif()

option(CSP_BUILD_SHARED "Whether or not to build the shared object" ON)
option(CSP_BUILD_STATIC "Whether or not to build the static archive" ON)

if (NOT CSP_BUILD_SHARED AND NOT CSP_BUILD_STATIC)
	  message(FATAL_ERROR "Neither static nor shared library build enabled")
endif()


include(CheckIncludeFiles)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(arpa/inet.h HAVE_ARPA_INET_H)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
    CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
	include(FindThreads)
	find_package(PkgConfig REQUIRED)

	pkg_search_module(LIBZMQ libzmq)
	pkg_search_module(LIBSOCKETCAN libsocketcan)

	# use the hint from about to find the location of socketcan
	find_library(LIBSOCKETCAN_LIBRARY
		NAMES socketcan
		PATHS ${LIBSOCKETCAN_LIBRARY_DIRS}
		)

endif()

#file(REAL_PATH include csp_inc)
set(csp_inc "${CMAKE_CURRENT_SOURCE_DIR}/include")
list(APPEND csp_inc ${CMAKE_CURRENT_BINARY_DIR})
#target_include_directories(csp PUBLIC ${csp_inc})

if(CSP_POSIX)
	  set(CSP_C_ARGS -Wshadow -Wcast-align -Wwrite-strings -Wno-unused-parameter)
elseif(CSP_ZEPHYR)
	  set(CSP_C_ARGS -Wwrite-strings -Wno-unused-parameter)
endif()

add_library(csp-objects OBJECT)
set_property(TARGET csp-objects PROPERTY POSITION_INDEPENDENT_CODE ON)
#target_compile_options(csp-objects PUBLIC ${CSP_C_ARGS})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(CSP_POSIX 1)
	target_sources(csp-objects PUBLIC
	  src/arch/posix/csp_clock.c
	  src/arch/posix/csp_malloc.c
	  src/arch/posix/csp_queue.c
	  src/arch/posix/csp_semaphore.c
	  src/arch/posix/csp_system.c
	  src/arch/csp_system.c
	  src/arch/posix/csp_thread.c
	  src/arch/posix/csp_time.c
	  src/arch/csp_time.c
	  src/arch/posix/pthread_queue.c
	  )
	

elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeRTOS")
	set(CSP_FREERTOS 1)
	# TODO:
elseif(CMAKE_SYSTEM_NAME STREQUAL "Zephyr")
	set(CSP_ZEPHYR 1)
	target_sources(csp-objects PUBLIC
	  src/arch/zephyr/csp_clock.c
	  src/arch/zephyr/csp_hooks.c
	  src/arch/zephyr/csp_queue.c
	  src/arch/zephyr/csp_semaphore.c
	  src/arch/zephyr/csp_time.c
	  src/arch/zephyr/csp_zephyr_init.c
	)
else()
	message(FATAL_ERROR "invalid system ${CMAKE_SYSTEM_NAME}")
endif()
target_sources(csp-objects PUBLIC
  	src/csp_bridge.c
  	src/csp_buffer.c
  	src/csp_conn.c
  	src/csp_crc32.c
  	src/csp_debug.c
  	src/csp_dedup.c
	src/csp_endian.c
  	src/csp_hex_dump.c
  	src/csp_id.c
  	src/csp_iflist.c
  	src/csp_init.c
  	src/csp_io.c
  	src/csp_port.c
  	src/csp_promisc.c
  	src/csp_qfifo.c
  	src/csp_route.c
	src/rtable/csp_rtable_cidr.c
	src/rtable/csp_rtable.c
  	src/csp_service_handler.c
  	src/csp_services.c
  	src/csp_sfp.c
  	src/transport/csp_udp.c
  	src/transport/csp_rdp.c
	#	src/csp_rdp_queue.c
  	src/csp_sfp.c
  	src/crypto/csp_hmac.c
  	src/crypto/csp_sha1.c
  )
#if (CSP_HAVE_STDIO)
#	  target_sources(csp-objects PUBLIC src/csp_rtable_stdio.c)
#endif()

target_sources(csp-objects PUBLIC src/drivers/usart/usart_kiss.c)

if(LIBSOCKETCAN_FOUND)
	  message(STATUS "LIBSOCKETCAN has been found!")
	  add_library(driver_can OBJECT src/drivers/can/can_socketcan.c)
	  add_library(driver_can_shared OBJECT src/drivers/can/can_socketcan.c)
	  target_include_directories(driver_can PUBLIC
	    ${csp_inc}
	    ${LIBSOCKETCAN_INCLUDE_DIRS})
	  target_link_libraries(driver_can PUBLIC
	    ${LIBSOCKETCAN_LIBRARIES})
	  target_include_directories(driver_can_shared PUBLIC
	    ${csp_inc}
	    ${LIBSOCKETCAN_INCLUDE_DIRS})
	  target_link_libraries(driver_can_shared PUBLIC
	    ${LIBSOCKETCAN_LIBRARIES})
	  set_property(TARGET driver_can_shared
			    PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_sources(csp-objects PUBLIC src/drivers/usart/usart_linux.c)
endif()

target_sources(csp-objects PUBLIC
	  src/interfaces/csp_if_lo.c
	  src/interfaces/csp_if_kiss.c
	  #src/interfaces/csp_if_i2c.c
	  #src/interfaces/csp_if_tun.c
)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Zephyr")
  target_sources(csp-objects PUBLIC
	    src/interfaces/csp_if_can.c
	    src/interfaces/csp_if_can_pbuf.c
  )
endif()

if(LIBZMQ_FOUND)
	  add_library(if_zmq OBJECT src/interfaces/csp_if_zmqhub.c)
	  target_include_directories(if_zmq PUBLIC
	    ${csp_inc}
	    ${LIBZMQ_INCLUDE_DIRS})
	  target_link_libraries(if_zmq PUBLIC ${LIBZMQ_LIBRARIES})
	  target_link_libraries(csp-objects PUBLIC if_zmq)
	  if(BUILD_SHARED_LIBS)
	    set_property(TARGET if_zmq PROPERTY POSITION_INDEPENDENT_CODE ON)
	  endif()
endif()

if(HAVE_SYS_SOCKET_H AND HAVE_ARPA_INET_H)
	  target_sources(csp-objects PUBLIC src/interfaces/csp_if_udp.c)
endif()

# public headers
target_include_directories(csp-objects
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

### EXAMPLES
#add_subdirectory(examples)

if(${enable-python3-bindings})
	find_package(Python COMPONENTS Development.Module REQUIRED)
  if(Python_Development.Module_FOUND)
    Python_add_library(csp_py3 src/bindings/python/pycsp.c WITH_SOABI)
    target_include_directories(csp_py3 PUBLIC ${csp_inc})
    target_link_libraries(csp_py3 PUBLIC csp)
  endif()
endif()

configure_file(csp_autoconfig.h.in csp_autoconfig.h)

# Dynamic
if (CSP_BUILD_SHARED)
add_library(csp SHARED $<TARGET_OBJECTS:csp-objects>)
add_library(csp::csp ALIAS csp)
set_target_properties (csp PROPERTIES
    DEFINE_SYMBOL "LIBCSP_EXPORTS"
    SOVERSION "0"
    VERSION "${CSP_RELEASE}"
    COMPILE_DEFINITIONS "DLL_EXPORT"
    OUTPUT_NAME "csp"
    PREFIX "lib"
)

target_link_libraries(csp
    PUBLIC ${MORE_LIBRARIES}
)
if(LIBSOCKETCAN_FOUND)
target_link_libraries(csp PRIVATE driver_can_shared)
endif()
include(FindThreads)
target_link_libraries(csp PUBLIC Threads::Threads)

target_include_directories(csp
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
      $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(csp PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/csp/csp.h")
endif()

# STATIC
if (CSP_BUILD_STATIC)
  add_library(csp-static STATIC $<TARGET_OBJECTS:csp-objects>)
add_library(csp::csp-static ALIAS csp-static)

  set_target_properties(csp-static PROPERTIES
    PUBLIC_HEADER "${CSP_PUBLIC_HEADERS}"
    COMPILE_DEFINITIONS "CSP_STATIC"
    OUTPUT_NAME "csp"
    PREFIX "lib"
  )
if(LIBSOCKETCAN_FOUND)
  target_link_libraries(csp-static
	  PRIVATE $<TARGET_OBJECTS:driver_can>
  )
  target_link_libraries(csp-static PUBLIC
    ${LIBSOCKETCAN_LIBRARIES})
endif()

  target_include_directories(csp-static
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
      $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  target_compile_definitions(csp-static
	  PUBLIC CSP_STATIC
  )
set_target_properties(csp-static PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/csp/csp.h")

endif()



########################################################################
# installer
########################################################################

# Defines the headers for this library.
# https://cmake.org/cmake/help/v3.11/command/target_include_directories.html

# Define the configuration file locations.
set(PROJECT_CONFIG_VERSION_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake")
set(PROJECT_CONFIG_FILE         "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")

configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/Config.cmake.in
        ${PROJECT_CONFIG_FILE}
        INSTALL_DESTINATION ${INSTALL_CONFIG_DIR})

# This auto-generates a version file.
write_basic_package_version_file(
        ${PROJECT_CONFIG_VERSION_FILE}
        COMPATIBILITY SameMajorVersion)


# The INSTALL(TARGET) command specifies where to install specific targets.
# By specifying EXPORT here, we add these targets to an export group.
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-static
        EXPORT ${PROJECT_NAME}-targets
	PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR}
        RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR})

# The INSTALL(FILES) command specifies where to install specific files, such as headers.
install(FILES ${HEADER_FILES}
        DESTINATION ${INSTALL_INCLUDE_DIR})

# Always install autoconifg header due to stupid CSP.h dependency
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/csp_autoconfig.h DESTINATION ${INSTALL_INCLUDE_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/csp DESTINATION ${INSTALL_INCLUDE_DIR})
# The INSTALL(FILES) command specifies where to install specific files, such as headers.
install(FILES
            ${PROJECT_CONFIG_VERSION_FILE}
            ${PROJECT_CONFIG_FILE}
        DESTINATION ${INSTALL_CONFIG_DIR})

# The INSTALL(EXPORT) command installs a previously defined export group to a script.
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${INSTALL_CONFIG_DIR})

export(EXPORT ${PROJECT_NAME}-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake
       NAMESPACE ${PROJECT_NAME}::)

export(PACKAGE ${PROJECT_NAME})
include(FeatureSummary)
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
