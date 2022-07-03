cmake_minimum_required(VERSION 3.13)
# cmake_policy(SET CMP0076 OLD)

set(FLAMESHOT_VERSION 12.1.0)

# Flameshot-org
set(GIT_API_URL "https://api.github.com/repos/flameshot-org/flameshot/releases/latest")
# Namecheap
# set(GIT_API_URL "https://api.github.com/repos/namecheap/flameshot/releases/latest")

# TODO - fix it for all linux distros
#find_package (Git)
#if (GIT_FOUND)
#  message("git found: ${GIT_EXECUTABLE} in version     ${GIT_VERSION_STRING}")
#
#  # set flameshot updates url
#  execute_process(COMMAND ${GIT_EXECUTABLE} ls-remote --get-url OUTPUT_VARIABLE GIT_ORIGIN_REMOTE)
#  message("GIT_ORIGIN_REMOTE: ${GIT_ORIGIN_REMOTE}")
#  string(REGEX REPLACE ".git\r*\n*$" "/releases/latest" GIT_API_URL ${GIT_ORIGIN_REMOTE})
#  string(REGEX REPLACE "^.*:" "https://api.github.com/repos/" GIT_API_URL ${GIT_API_URL})
#  message("GIT_API_URL: '${GIT_API_URL}'")
#
#  # get application version
#  execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0 --match v[0-9]* OUTPUT_VARIABLE FLAMESHOT_VERSION)
#  string(REGEX REPLACE "\r" "" FLAMESHOT_VERSION ${FLAMESHOT_VERSION})
#  string(REGEX REPLACE "\n" "" FLAMESHOT_VERSION ${FLAMESHOT_VERSION})
#  string(REGEX REPLACE "^v" "" FLAMESHOT_VERSION ${FLAMESHOT_VERSION})
#  message("FLAMESHOT_VERSION: '${FLAMESHOT_VERSION}'")
#else()
#  message("git command is not found")
#endif ()

project(
        flameshot
        VERSION ${FLAMESHOT_VERSION}
        LANGUAGES CXX)
set(PROJECT_NAME_CAPITALIZED "Flameshot")

# This can be read from ${PROJECT_NAME} after project() is called
if (APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
endif()


# Configuration options
set(DEFAULT_RUN_IN_PLACE FALSE)
if(WIN32)
  set(DEFAULT_RUN_IN_PLACE TRUE)
  # For Windows RC file.
  add_definitions(-DFLAMESHOT_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
  add_definitions(-DFLAMESHOT_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
  add_definitions(-DFLAMESHOT_VERSION_BUGFIX=${CMAKE_PROJECT_VERSION_PATCH})
  add_definitions(-DFLAMESHOT_VERSION_BUILD=1)
  add_definitions(-DFLAMESHOT_VERSION_STRING="${PROJECT_VERSION}")
elseif(APPLE)
  set(Qt5_DIR "$(brew --prefix qt5)/lib/cmake/Qt5/" CACHE PATH "directory where Qt5Config.cmake exists.")
  set(CMAKE_MACOSX_BUNDLE ON)
  set(CMAKE_MACOSX_RPATH ON)
endif()
set(RUN_IN_PLACE
        ${DEFAULT_RUN_IN_PLACE}
        CACHE BOOL "Run directly in source directory structure")


option(FLAMESHOT_DEBUG_CAPTURE "Enable mode to make debugging easier" OFF)
option(USE_MONOCHROME_ICON "Build using monochrome icon as default" OFF)
option(GENERATE_TS "Regenerate translation source files" OFF)
option(USE_EXTERNAL_SINGLEAPPLICATION "Use external QtSingleApplication library" OFF)
option(USE_LAUNCHER_ABSOLUTE_PATH "Use absolute path for the desktop launcher" ON)
option(USE_WAYLAND_CLIPBOARD "USE KF Gui Wayland Clipboard" OFF)

include(cmake/StandardProjectSettings.cmake)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
# set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options include(cmake/StaticAnalyzers.cmake)

set(QAPPLICATION_CLASS
        QApplication
        CACHE STRING "Inheritance class for SingleApplication")

if(USE_EXTERNAL_SINGLEAPPLICATION)
  # look for external QtSingleApplication
  # package dev-qt/qtsingleapplication provides no symlink to current version
  set(qtsingleapplication_libs libQt5Solutions_SingleApplication-2.6 Qt5Solutions_SingleApplication-2.6)
  find_library(QTSINGLEAPPLICATION_LIBRARY NAMES ${qtsingleapplication_libs})
  message(STATUS "Using external SingleApplication library")
else()
  add_subdirectory(external/singleapplication)
  set(QTSINGLEAPPLICATION_LIBRARY SingleApplication::SingleApplication)
endif()


option(BUILD_STATIC_LIBS ON)
option(BUILD_SHARED_LIBS OFF)
add_subdirectory(external/Qt-Color-Widgets EXCLUDE_FROM_ALL) 


if (APPLE)
  add_subdirectory(external/QHotkey)
endif()
add_subdirectory(src)

# CPack
set(CPACK_PACKAGE_VENDOR "flameshot-org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Powerful yet simple to use screenshot software.")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_HOMEPAGE_URL "https://flameshot.org")
set(CPACK_PACKAGE_CONTACT "flameshot-org developers <info@flameshot.org>")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/data/img/app/org.flameshot.Flameshot.svg") # TODO: Can any generator make
# use of this?
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md") # TODO: Where is this used? Do we need a better
# source?

if(WIN32)
  # Include all dynamically linked runtime libraries such as MSVCRxxx.dll
  include(InstallRequiredSystemLibraries)

  if(RUN_IN_PLACE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-win64")
    else()
      set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-win32")
    endif()

    set(CPACK_GENERATOR ZIP)

  else()
    set(CPACK_GENERATOR WIX ZIP)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME_CAPITALIZED}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME_CAPITALIZED}")
    set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} "${PROJECT_NAME_CAPITALIZED}")
    set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})

    # WIX (Windows .msi installer)
    # 48x48 pixels
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/data/img/app/flameshot.ico")
    # Supported languages can be found at http://wixtoolset.org/documentation/manual/v3/wixui/wixui_localization.html
    # set(CPACK_WIX_CULTURES "ar-SA,bg-BG,ca-ES,hr-HR,cs-CZ,da-DK,nl-NL,en-US,et-EE,fi-FI,fr-FR,de-DE")
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/packaging/win-installer/Bitmaps/CPACK_WIX_UI_BANNER.BMP")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/packaging/win-installer/Bitmaps/CPACK_WIX_UI_DIALOG.BMP")
    set(CPACK_WIX_PROPERTY_ARPHELPLINK "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_WIX_ROOT_FEATURE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
    set(CPACK_WIX_LIGHT_EXTRA_FLAGS "-dcl:high") # set high compression

    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/packaging/win-installer/LICENSE/GPL-3.0.txt")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

    # The correct way would be to include both x32 and x64 into one installer and install the appropriate one. CMake
    # does not support that, so there are two separate GUID's
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(CPACK_WIX_UPGRADE_GUID "26D8062A-66D9-48D9-8924-42090FB9B3F9")
    else()
      set(CPACK_WIX_UPGRADE_GUID "2C53E1B9-51D9-4429-AAE4-B02221959AA5")
    endif()
  endif()
elseif(APPLE)
  set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
  set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-osx")
  set(CPACK_GENERATOR ZIP)
else()
  set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-linux")
  set(CPACK_GENERATOR TGZ)
  set(CPACK_SOURCE_GENERATOR TGZ)
endif()

include(CPack)
