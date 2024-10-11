find_program(CPPCHECK "cppcheck")
if (CPPCHECK)
  set(CMAKE_C_CPPCHECK "${CPPCHECK}"
    "--language=c"
    "--enable=all"
    "--inconclusive"
    "--force"
    "--inline-suppr"
    "--xml"
    "--xml-version=2" 
    "--output-file=${CMAKE_BINARY_DIR}/cppcheck-${PROJECT_NAME}.xml"
    )
endif()
find_program(CLANGTIDY "clang-tidy")
if (CLANGTIDY)
  set(CMAKE_C_CLANG_TIDY "${CLANGTIDY}")
endif()

# tests
find_program( MEMORYCHECK_COMMAND valgrind )
set( MEMORYCHECK_COMMAND_OPTIONS "--xml=yes --memcheck:leak-check=full --show-reachable=yes" CACHE STRING "" FORCE )
