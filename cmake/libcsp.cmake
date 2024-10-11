include(FetchContent)

set(LIBCSP libcsp)

FetchContent_Declare(
    ${LIBCSP}
    GIT_REPOSITORY   https://github.com/libcsp/libcsp.git
    GIT_TAG          8523be4e727f674b211e99c5ab73461b58d5129e
)

FetchContent_GetProperties(${LIBCSP})
if(NOT ${LIBCSP}_POPULATED)
	FetchContent_Populate(${LIBCSP})
	file(COPY patches/CMakeLists.txt patches/Config.cmake.in patches/csp_autoconfig.h.in DESTINATION ${${LIBCSP}_SOURCE_DIR}
)
endif()
message(STATUS "CSP directories are ${${LIBCSP}_SOURCE_DIR} ${${LIBCSP}_BINARY_DIR} ")
set(LIBCSP_INCLUDE_DIR ${${LIBCSP}_SOURCE_DIR}/include CACHE INTERNAL "Path to libcsp include dir")
set(libcsp_SOURCE_DIR ${${LIBCSP}_SOURCE_DIR}/ CACHE INTERNAL "Path to libcsp source dir")

add_subdirectory(${${LIBCSP}_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/artifacts)
