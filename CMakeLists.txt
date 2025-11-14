cmake_minimum_required(VERSION 3.16)
# cmake_policy(SET CMP0076 OLD)

set(FLAMESHOT_VERSION 13.3.0)

# Flameshot-org
set(GIT_API_URL "https://api.github.com/repos/flameshot-org/flameshot/releases/latest")

project(
        flameshot
        VERSION ${FLAMESHOT_VERSION}
        LANGUAGES CXX)
set(PROJECT_NAME_CAPITALIZED "Flameshot")

include(FetchContent)
#Must be set before fetching external content!
#QT_DEFAULT_MAJOR_VERSION used by QHotkey
set(QT_DEFAULT_MAJOR_VERSION 6 CACHE STRING "")
#QT_VERSION_MAJOR used by Flameshot and QtColorWidgets
set(QT_VERSION_MAJOR 6 CACHE STRING "")
#BUILD_SHARED_LIBS used by QHotkey and QtColorWidgets
option(BUILD_SHARED_LIBS OFF)

#Needed due to linker error with QtColorWidget
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Dependency can be fetched via flatpak builder
if(EXISTS "${CMAKE_SOURCE_DIR}/external/Qt-Color-Widgets/CMakeLists.txt")
    add_subdirectory("${CMAKE_SOURCE_DIR}/external/Qt-Color-Widgets" EXCLUDE_FROM_ALL)
else()
    FetchContent_Declare(
        qtColorWidgets
        GIT_REPOSITORY https://gitlab.com/mattbas/Qt-Color-Widgets.git
        GIT_TAG 352bc8f99bf2174d5724ee70623427aa31ddc26a
    )
  #Workaround for duplicate GUID in windows WIX installer
  if (WIN32)
      FetchContent_GetProperties(qtColorWidgets)
      if(NOT qtcolorwidgets_POPULATED)
          FetchContent_Populate(qtColorWidgets)
          add_subdirectory(${qtcolorwidgets_SOURCE_DIR} ${qtcolorwidgets_BINARY_DIR} EXCLUDE_FROM_ALL)
      endif()
  else()
      FetchContent_MakeAvailable(qtColorWidgets)
  endif()
endif()

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
  set(Qt6_DIR "$(brew --prefix qt6)/lib/cmake/Qt6/" CACHE PATH "directory where Qt6Config.cmake exists.")
endif()
set(RUN_IN_PLACE
        ${DEFAULT_RUN_IN_PLACE}
        CACHE BOOL "Run directly in source directory structure")


option(FLAMESHOT_DEBUG_CAPTURE "Enable mode to make debugging easier" OFF)
option(USE_MONOCHROME_ICON "Build using monochrome icon as default" OFF)
option(GENERATE_TS "Regenerate translation source files" OFF)
option(USE_KDSINGLEAPPLICATION "Use KDSingleApplication library" ON)
option(USE_BUNDLED_KDSINGLEAPPLICATION "Use a bundled version of the KDSingleApplication library" ${USE_KDSINGLEAPPLICATION})
option(USE_LAUNCHER_ABSOLUTE_PATH "Use absolute path for the desktop launcher" ON)
option(USE_WAYLAND_CLIPBOARD "USE KF Gui Wayland Clipboard" OFF)
option(DISABLE_UPDATE_CHECKER "Disable check for updates" OFF)
option(ENABLE_IMGUR "Enable Imgur Uploader" OFF)

if (ENABLE_IMGUR)
  add_compile_definitions(ENABLE_IMGUR)
endif()

if (DISABLE_UPDATE_CHECKER)
  add_compile_definitions(DISABLE_UPDATE_CHECKER)
endif ()

include(cmake/StandardProjectSettings.cmake)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

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

if (USE_KDSINGLEAPPLICATION)
  if (USE_BUNDLED_KDSINGLEAPPLICATION)
    set(KDSingleApplication_EXAMPLES OFF CACHE BOOL "Don't build the examples")
    set(KDSingleApplication_STATIC ON CACHE BOOL "Build static versions of the libraries")

    # Check if KDSingleApplication is available locally
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/KDSingleApplication/CMakeLists.txt")
      add_subdirectory("${CMAKE_SOURCE_DIR}/external/KDSingleApplication")
    else()
      FetchContent_Declare(
          kdsingleApplication
          GIT_REPOSITORY https://github.com/KDAB/KDSingleApplication.git
          GIT_TAG v1.2.0
      )
      FetchContent_MakeAvailable(kdsingleApplication)
    endif()
  else()
    find_package(KDSingleApplication-qt6 REQUIRED)
  endif()
endif()

# ToDo: Check if this is used anywhere
option(BUILD_STATIC_LIBS ON)

if (APPLE)
  FetchContent_Declare(
      qHotKey
      GIT_REPOSITORY https://github.com/flameshot-org/QHotkey
      GIT_TAG master
  )
  FetchContent_MakeAvailable(QHotKey)
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
