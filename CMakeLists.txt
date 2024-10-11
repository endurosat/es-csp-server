cmake_minimum_required(VERSION 3.21)

include(es_version.cmake)
project(es_csp_server VERSION ${ES_VERSION} LANGUAGES C)

include(FeatureSummary)
include(${PROJECT_SOURCE_DIR}/cmake/install_paths.cmake)

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb -Wall -Wextra -pedantic -fanalyzer  ")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -Wall -Wextra  -fanalyzer")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -Wextra -pedantic -fanalyzer")

set(CMAKE_COLOR_DIAGNOSTICS ON)

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
add_subdirectory(libs/es_csp_server ${CMAKE_BINARY_DIR}/es_csp_server)
add_subdirectory(libs/es_log ${CMAKE_BINARY_DIR}/es_log)
add_subdirectory(libs/es_tftp ${CMAKE_BINARY_DIR}/es_tftp)
add_subdirectory(app ${CMAKE_BINARY_DIR}/app)
