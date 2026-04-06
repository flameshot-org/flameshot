option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
option(ENABLE_INCLUDE_WHAT_YOU_USE "Enable static analysis with include-what-you-use" OFF)
set(ENABLE_QT_DEPRECATION_CHECK "" CACHE STRING "Define hex value (e.g. 0x061100 for Qt 6.11) for testing for deprecations. Empty = OFF")

if(ENABLE_CPPCHECK)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    message(STATUS "CPPCHECK is enabled!")
    set(CMAKE_CXX_CPPCHECK
        ${CPPCHECK}
        --library=qt
        --suppress=missingInclude
        --enable=all
        --inline-suppr
        --inconclusive)
  else()
    message(SEND_ERROR "cppcheck requested but executable not found")
  endif()
endif()

if(ENABLE_CLANG_TIDY)
  find_program(CLANGTIDY clang-tidy)
  if(CLANGTIDY)
    message(STATUS "CLANGTIDY is enabled!")
    set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY} -extra-arg=-Wno-unknown-warning-option)
  else()
    message(SEND_ERROR "clang-tidy requested but executable not found")
  endif()
endif()

if(ENABLE_INCLUDE_WHAT_YOU_USE)
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
  if(INCLUDE_WHAT_YOU_USE)
    message(STATUS "INCLUDE_WHAT_YOU_USE is enabled!")
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
  else()
    message(SEND_ERROR "include-what-you-use requested but executable not found")
  endif()
endif()

if(NOT "${ENABLE_QT_DEPRECATION_CHECK}" STREQUAL "")
    message(STATUS "Qt deprecations warnings enabled for: ${ENABLE_QT_DEPRECATION_CHECK}")
    add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=${ENABLE_QT_DEPRECATION_CHECK})
endif()
