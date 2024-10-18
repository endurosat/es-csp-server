cmake_minimum_required(VERSION 3.21)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(es_version)
project(es_csp_server VERSION ${ES_VERSION} LANGUAGES C)

include(FeatureSummary)
include(install_paths)

set(WARNINGS "-Wall -Wextra -pedantic")
set(CMAKE_COLOR_DIAGNOSTICS ON)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    # require at least gcc 11.2
    message(INFO "-- Found GCC ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
    if (CMAKE_C_COMPILER_VERSION VERSION_LESS 11.2)
        message(FATAL_ERROR "GCC version must be at least 11!")
    endif()
	set(ANALYSIS "-fanalyzer")

elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    # require at least clang 17
    message(INFO "-- Found CLANG ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
    if (CMAKE_C_COMPILER_VERSION VERSION_LESS 17.0)
        message(FATAL_ERROR "Clang version must be at least 17!")
    endif()
	set(ANALYSIS "") # linkage fails when clang --analyze option is on
else()
	message(FATAL_ERROR "You are using an unsupported compiler! Compilation has only been tested with Clang and GCC.")
endif()


set(CMAKE_C_FLAGS_DEBUG "${WARNINGS} ${ANALYSIS} -O0 -ggdbi " CACHE STRING " Global Debug flags "FORCE)
set(CMAKE_C_FLAGS_MINSIZEREL "${WARNINGS} ${ANALYSIS} -Os " CACHE STRING "Global MinSizeRel flags" FORCE)
set(CMAKE_C_FLAGS_RELEASE "${WARNINGS} ${ANALYSIS} -O3 " CACHE STRING "Global Release flags" FORCE)

option(USE_STATIC_ANALYSIS "Enable static analysis with cppcheck on sources" OFF)
enable_testing()
find_package(Git REQUIRED)

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	# Update submodules as needed
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/libcsp/README.rst")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

add_subdirectory(libs/libcsp ${CMAKE_BINARY_DIR}/libcsp)
add_subdirectory(libs/es_log ${CMAKE_BINARY_DIR}/es_log)
add_subdirectory(libs/es_tftp ${CMAKE_BINARY_DIR}/es_tftp)
add_subdirectory(libs/es_csp_server ${CMAKE_BINARY_DIR}/es_csp_server)
add_subdirectory(app ${CMAKE_BINARY_DIR}/app)
