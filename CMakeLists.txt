#
# Qodem Terminal Emulator
#
# CMake configuration contributed by Richard Shaw (KF5OIM)
# Please report questions, comments, problems, or patches to the github
# page: https://github.com/klamonte/qodem
#

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "Minimum OS X deployment version")

cmake_minimum_required(VERSION 3.0)
# Allow version to be set by the project command.
if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()

# Prevent in-source builds to protect automake/autoconf config.
# If an in-source build is attempted, you will still need to clean up a few
# files manually.
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds in ${CMAKE_BINARY_DIR} are not "   
   "allowed, please remove ./CMakeCache.txt and ./CMakeFiles/, create a "
   "separate build directory and run cmake from there.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

# Set local module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(qodem 
    LANGUAGES C
    VERSION 1.0.0
    DESCRIPTION "Qodem Terminal Emulator")

#
# Set version and generate src/version.h
#
message(STATUS "Qodem version: ${PROJECT_VERSION}")

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
    message(STATUS "Build type not specified, defaulting to ${CMAKE_BUILD_TYPE}")
endif(NOT CMAKE_BUILD_TYPE)

# Set default build flags.
# Set Q_NO_NEWTERM by default so that Arch Linux works out of the box.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Do not call newterm so Arch Linux works out of the box.
add_definitions("-DQ_NO_NEWTERM")

#
# Setup cmake options
#
option(USE_BUILTIN_CURSES
    "Use builtin curses instead of the system library.")
option(USE_BUILTIN_MINIUPNPC
    "Use builtin miniupnp instead of the system library.")
option(ENABLE_X11 "Enable X11 build using PDCurses.")
option(DISABLE_SERIAL
    "Disable serial communications.")
if(DISABLE_SERIAL)
    add_definitions("-DQ_NO_SERIAL")
endif()

#
# Various hacks and work arounds for building under MinGW.
#
if(MINGW)
    message(STATUS "System is MinGW.")
    # Setup HOST variable.
    include(cmake/MinGW.cmake)
    # This sets up the exe icon for windows under mingw.
    set(RES_FILES "")
    set(RES_FILES "${CMAKE_SOURCE_DIR}/build/icons/qodem.ico")
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT
        "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
    include(InstallRequiredSystemLibraries)
endif(MINGW)

# Math library is automatic on MinGW
if(UNIX)
    set(CMAKE_REQUIRED_LIBRARIES m)
    set(LINK_LIBS m)
endif(UNIX)

# Determine if we're on a 64bit system.
include(CheckTypeSize)
check_type_size("int" SIZEOF_INT)

# Checks for header files.
include(CheckIncludeFile)
check_include_file("byteswap.h"            HAVE_BYTESWAP_H)
check_include_file("sys/select.h"          HAVE_SYS_TYPES_H)
check_include_file("pty.h"                 HAVE_PTY_H)
check_include_file("libutil.h"             HAVE_LIBUTIL_H)
check_include_file("util.h"                HAVE_UTIL_H)
check_include_file("getopt.h"              HAVE_GETOPT_H)
check_include_file("ncursesw/curses.h"     HAVE_NCURSESW_CURSES_H)
check_include_file("miniupnpc/miniupnpc.h" HAVE_MINIUPNPC_MINIUPNPC_H)
if(ENABLE_X11)
check_include_file("sys/select.h"          HAVE_SYS_SELECT_H)
endif()

include(CheckFunctionExists)
check_function_exists(sqrt HAVE_SQRT)

# Find what library provides forkpty
find_library(LIBUTIL util)
if(LIBUTIL)
    set(CMAKE_REQUIRED_LIBRARIES ${LIBUTIL})
    check_function_exists(forkpty HAVE_FORKPTY)
    if(HAVE_FORKPTY)
        list(APPEND LINK_LIBS ${LIBUTIL})
    else()
        message(FATAL_ERROR "Function forkpty not found.")
    endif()
endif()

#
# Find SDL
#
message(STATUS "Looking for SDL...")
find_package(SDL)
if(SDL_FOUND)
    list(APPEND LINK_LIBS ${SDL_LIBRARY})
    include_directories(${SDL_INCLUDE_DIR})
    add_definitions("-DQ_SOUND_SDL")
else()
    message(WARNING "SDL library not found.
On Linux systems try installing:
    sdl-devel  (RPM based systems)
    libsdl-dev (DEB based systems)"
    )
endif()

#
# Find ncursesw or ncurses... Need to add PDCurses.
#
find_library(NCURSES NAMES ncursesw ncurses)
if(NCURSES)
    message(STATUS "Found ncurses: ${NCURSES}")
    set(CMAKE_REQUIRED_LIBRARIES ${NCURSES})
    check_function_exists(mvwadd_wch HAS_MVWADD_WCH)
else()
    message(FATAL_ERROR "Neither ncursesw or ncurses found.")
endif()
if(NCURSES AND HAS_MVWADD_WCH)
    list(APPEND LINK_LIBS ${NCURSES})
    add_definitions("-DQ_HAS_NCURSES")
else()
    message(FATAL_ERROR "Curses library found but function mvwadd_wch not found.")
endif()

find_library(MINIUPNPC miniupnpc)
find_path(MINIUPNPC_INCLUDE miniupnpc.h
    PATH_SUFFIXES miniupnpc)
if(MINIUPNPC)
    message(STATUS "Found miniupnpc: ${MINIUPNPC}")
    set(CMAKE_REQUIRED_LIBRARIES ${MINIUPNPC})
    check_function_exists(upnpDiscover USE_SYSTEM_UPNP)
endif()
if(MINIUPNPC AND USE_SYSTEM_UPNP)
    list(APPEND LINK_LIBS ${MINIUPNPC})
    include_directories(${MINIUPNPC_INCLUDE})
    add_definitions("-DQ_UPNP -DQ_USE_SYSTEM_UPNP")
endif()

configure_file(cmake/config.h.in config.h @ONLY)
include_directories(${CMAKE_BINARY_DIR})
add_definitions("-DHAVE_CONFIG_H")

# qodem sources
add_subdirectory(source)

# Icons and desktop file
#add_subdirectory(contrib)

message(STATUS "Build type will be: ${CMAKE_BUILD_TYPE}")

#
# Cpack NSIS configuration for Windows.
#
if(WIN32)
    # Detect if we're doing a 32-bit or 64-bit windows build.
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(CMAKE_CL_64 TRUE)
        set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    endif()
    if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(CPACK_STRIP_FILES TRUE)
    endif()
    configure_file(cmake/GetDependencies.cmake.in cmake/GetDependencies.cmake
        @ONLY
    )
    install(SCRIPT ${CMAKE_BINARY_DIR}/cmake/GetDependencies.cmake)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
    set(CPACK_PACKAGE_VENDOR "CMake")
    #set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.
    #set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\qodem.exe")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    set(CPACK_NSIS_PACKAGE_NAME "Qodem")
    set(CPACK_PACKAGE_EXECUTABLES qodem;Qodem)
    set(CPACK_NSIS_URL_INFO_ABOUT "http://qodem.sourceforge.net/")
    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_MENU_LINKS
        "http://qodem.sourceforge.net/" "Qodem Homepage")
    include(CPack)
endif(WIN32)
