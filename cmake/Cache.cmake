option(ENABLE_CACHE "Enable cache if available" ON)
if(NOT ENABLE_CACHE)
  return()
endif()

set(CACHE_OPTION
    "ccache"
    CACHE STRING "Compiler cache to be used")
set(CACHE_OPTION_VALUES "ccache" "sccache")
set_property(CACHE CACHE_OPTION PROPERTY STRINGS ${CACHE_OPTION_VALUES})
list(
  FIND
  CACHE_OPTION_VALUES
  ${CACHE_OPTION}
  CACHE_OPTION_INDEX)

if(${CACHE_OPTION_INDEX} EQUAL -1)
  message(
    STATUS
      "Using custom compiler cache system: '${CACHE_OPTION}', explicitly supported entries are ${CACHE_OPTION_VALUES}")
endif()

find_program(CACHE_BINARY ${CACHE_OPTION})
if(CACHE_BINARY)
  message(STATUS "${CACHE_OPTION} found and enabled")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CACHE_BINARY})
else()
  message(WARNING "${CACHE_OPTION} is enabled but was not found. Not using it")
endif()
