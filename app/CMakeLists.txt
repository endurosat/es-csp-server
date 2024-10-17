cmake_minimum_required(VERSION 3.16)
include(GNUInstallDirs)
project(csp_server DESCRIPTION "CSP server" LANGUAGES C)

include(${CMAKE_SOURCE_DIR}/cmake/install_paths.cmake)
find_package(PkgConfig REQUIRED)

if(USE_STATIC_ANALYSIS)
	include(${CMAKE_SOURCE_DIR}/cmake/analysis.cmake)
endif(USE_STATIC_ANALYSIS)

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(csp_server ${SOURCES})

target_include_directories(csp_server
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
target_include_directories(csp_server
	PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(csp_server PUBLIC es_csp_server::es_csp_server)
target_link_libraries(csp_server PUBLIC es_log::es_log)
target_link_libraries(csp_server PUBLIC es_tftp::es_tftp)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/es-csp-server-uart.service
		${CMAKE_CURRENT_SOURCE_DIR}/es-csp-server-can.service
		DESTINATION ${INSTALL_SYSTEMD_SERVICE})

install(TARGETS csp_server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
