cmake_minimum_required(VERSION 3.21)

project(es_log VERSION 1.0.0 LANGUAGES C)

include(CMakePackageConfigHelpers)
include(FindThreads)
include(${CMAKE_SOURCE_DIR}/cmake/install_paths.cmake)

if(USE_STATIC_ANALYSIS)
	include(${CMAKE_SOURCE_DIR}/cmake/analysis.cmake)
endif(USE_STATIC_ANALYSIS)

message(STATUS "Building ${PROJECT_NAME}!")

file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADER_FILES "include/*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADER_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties (${PROJECT_NAME} PROPERTIES
	DEFINE_SYMBOL "LIB${PROJECT_NAME}_EXPORTS"
	VERSION "${CMAKE_PROJECT_VERSION}"
	COMPILE_DEFINITIONS "SO_EXPORT"
	OUTPUT_NAME "${PROJECT_NAME}"
	PREFIX "lib"
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include/${PROJECT_PREFIX}>)

add_library(${PROJECT_NAME}-static STATIC ${SOURCES} ${HEADER_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME}-static ALIAS ${PROJECT_NAME}-static)

set_property(TARGET ${PROJECT_NAME}-static PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(${PROJECT_NAME}-static
        PUBLIC
            $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include/${PROJECT_PREFIX}>)


set(PROJECT_CONFIG_VERSION_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake")
set(PROJECT_CONFIG_FILE         "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")

configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/Config.cmake.in
        ${PROJECT_CONFIG_FILE}
        INSTALL_DESTINATION ${INSTALL_CONFIG_DIR})

write_basic_package_version_file(
        ${PROJECT_CONFIG_VERSION_FILE}
        COMPATIBILITY SameMajorVersion)

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-static
        EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR})

install(FILES ${HEADER_FILES}
        DESTINATION ${INSTALL_INCLUDE_DIR})

install(FILES
            ${PROJECT_CONFIG_VERSION_FILE}
            ${PROJECT_CONFIG_FILE}
        DESTINATION ${INSTALL_CONFIG_DIR})

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${INSTALL_CONFIG_DIR})

export(EXPORT ${PROJECT_NAME}-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake
       NAMESPACE ${PROJECT_NAME}::)

export(PACKAGE ${PROJECT_NAME})
