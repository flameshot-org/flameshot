find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

qt5_wrap_cpp(MOC_HEADERS
        qhotkey.h
        qhotkey_p.h)

set(LIBS
        Qt5::Core
        Qt5::Widgets)

set(SRC_FILES
        qhotkey.cpp)

if(APPLE)
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")

    find_library(CARBON_LIBRARY Carbon)
    mark_as_advanced(CARBON_LIBRARY)

    set(SRC_FILES ${SRC_FILES} qhotkey_mac.cpp)
    set(LIBS ${LIBS} ${CARBON_LIBRARY})
elseif(WIN32)
    set(SRC_FILES ${SRC_FILES} qhotkey_win.cpp)
else()
    find_package(X11 REQUIRED)
    find_package(Qt5X11Extras REQUIRED)

    include_directories(${X11_INCLUDE_DIR})
    set(LIBS ${LIBS} ${X11_LIBRARIES} Qt5::X11Extras)
    set(SRC_FILES ${SRC_FILES} qhotkey_x11.cpp)
endif()

add_library(qhotkey ${SRC_FILES} ${MOC_HEADERS})
add_library(QHotkey::QHotkey ALIAS qhotkey)
target_link_libraries(qhotkey ${LIBS})

target_include_directories(qhotkey
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>)
