cmake_minimum_required(VERSION 3.9)
# Set the PROJECT_VERSION_FILE variable before including the cmake script
set(PROJECT_VERSION_FILE "../version.json")
include(../cmake/es_version.cmake)

#---------- Version generation ----------#
message(STATUS, "Version: ${ES_VERSION}")
# Split the string based on '.' and create a list
string(REPLACE "." ";" VERSION_LIST ${ES_VERSION})

# Convert the list into individual variables
list(GET VERSION_LIST 0 VERSION_MAJOR_PART)
list(GET VERSION_LIST 1 VERSION_MAJOR_PART2)
list(GET VERSION_LIST 2 VERSION_MINOR)
list(GET VERSION_LIST 3 VERSION_RELEASE_NUMBER)

# Construct the major as per specified pattern
set(VERSION_MAJOR "${VERSION_MAJOR_PART}.${VERSION_MAJOR_PART2}")

# Patch (release number) remains the same
set(VERSION_PATCH "${VERSION_RELEASE_NUMBER}")

# Display the variables (for debugging purposes)
message(STATUS "Configured version major: ${VERSION_MAJOR}")
message(STATUS "Configured version minor: ${VERSION_MINOR}")
message(STATUS "Configured version release number: ${VERSION_RELEASE_NUMBER}")

set(VERSION_IN_FILE "${CMAKE_SOURCE_DIR}/version.in.xml")
set(VERSION_OUT_FILE "${CMAKE_BINARY_DIR}/version.xml")  
# Destination is now in CMAKE_BINARY_DIR
# Process and generate the version.xml file
configure_file(${VERSION_IN_FILE} ${VERSION_OUT_FILE} @ONLY)

project(libcsp_docs VERSION ${ES_VERSION})

set(SPHINX_EXECUTABLE sphinx-build)
set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/html)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

file(GLOB DOCS_FILES
    "${SPHINX_SOURCE}/*.md"
    "${SPHINX_SOURCE}/*.rst"
    "${SPHINX_SOURCE}/modules/*.rst")

file(COPY ${SPHINX_SOURCE}/.nojekyll DESTINATION ${SPHINX_BUILD})
add_custom_target(docs ALL DEPENDS ${SPHINX_INDEX_FILE})
add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
    COMMAND
        ${SPHINX_EXECUTABLE} -E -b html ${SPHINX_SOURCE} ${SPHINX_BUILD} ${DOCS_FILES}
    MAIN_DEPENDENCY conf.py
    COMMENT "Building DOCS for lib CSP")
