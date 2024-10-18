# Example versioning:
#     1.52.131.592938
#     │ │   │    └── [4] patch/release number [CI_PIPELINE_ID or GITHUB_RUN ID]
#     │ │   └── [3] minor version [commits into master]
#     │ └── [2] major version (manually increment on each release)
#     └── [1] software variant

# Default value
set(ES_VERSION 1.0.0.0)

if (NOT DEFINED PROJECT_VERSION_FILE)
  set(PROJECT_VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/version.json)
endif()
file(READ ${PROJECT_VERSION_FILE} VERSION_JSON_STR)

# [1] Extract the software variant from the JSON-formatted string
string(JSON SW_VARIANT GET ${VERSION_JSON_STR} 0 software_variant)
list(APPEND EXTRACTED_SW_VARIANT ${SW_VARIANT})
message(TRACE "EXTRACTED_SW_VARIANT: ${EXTRACTED_SW_VARIANT}")

# [2] Extract the major version from the JSON-formatted string
string(JSON MAJOR_VERSION GET ${VERSION_JSON_STR} 0 major_version)
list(APPEND EXTRACTED_MAJOR_VERSION ${MAJOR_VERSION})

message(TRACE "EXTRACTED_MAJOR_VERSION: ${EXTRACTED_MAJOR_VERSION}")

# [3] Get the number of commits in the master branch using git
execute_process(
    COMMAND git rev-list --count master
    RESULT_VARIABLE script_result
    OUTPUT_VARIABLE commits_in_master
    ERROR_VARIABLE script_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
)

if(NOT script_result EQUAL 0)
    message(ERROR "Failed to get commit count: ${script_error}")
    set(commits_in_master "0")
endif()

message(TRACE "Commits in master: ${commits_in_master}")

# [4] Runnning locally
set(patch 0)

# Running in CI/CD
# Use $CI_PIPELINE_ID (GitLab) or $GIHUB_RUN_ID in GitHub
if (DEFINED ENV{CI_PIPELINE_ID})
    set(patch $ENV{CI_PIPELINE_ID})
    message(TRACE "patch: ${patch}")
elseif(DEFINED ENV{GITHUB_RUN_ID})
    set(patch $ENV{GITHUB_RUN_ID})
    message(TRACE "patch: ${patch}")
endif()

string(CONCAT ES_VERSION "${EXTRACTED_SW_VARIANT}.${EXTRACTED_MAJOR_VERSION}.${commits_in_master}.${patch}")

message(STATUS "ES_VERSION is set to ${ES_VERSION}")
