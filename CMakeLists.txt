cmake_minimum_required(VERSION 3.0)
set(CMAKE_VERBOSE_MAKEFILE OFF)

####################
# TARGET PLATFORM
####################
set(PLATFORM_LINUX OFF CACHE BOOL "Linux platform target")
set(PLATFORM_WINDOWS OFF CACHE BOOL "Windows platform target (msys/mingw64)")
set(PLATFORM_RPI3 OFF CACHE BOOL "RIP3 platform target")
set(PLATFORM_SWITCH OFF CACHE BOOL "Nintendo Switch platform target")
set(PLATFORM_3DS OFF CACHE BOOL "Nintendo 3DS platform target")
set(PLATFORM_VITA OFF CACHE BOOL "Sony PS Vita platform target")

####################
# BUILD OPTIONS
####################
set(OPTION_SDL1 OFF CACHE BOOL "SDL1 support (software, slow, unsupported)")
set(OPTION_SDL2_GL OFF CACHE BOOL "SDL2 support (OpenGL 3.3)")
set(OPTION_SDL2_GLES2 OFF CACHE BOOL "SDL2 support (OpenGLES 2.0)")
set(OPTION_SDL2_SOFT OFF CACHE BOOL "SDL2 support (software, slow)")
set(OPTION_BUILTIN_LIBCONFIG OFF CACHE BOOL "Build libconfig from sources")
set(OPTION_FREEPLAY OFF CACHE BOOL "Build with FreePlay (CM3/ZERO) support")
set(OPTION_RECALBOX_BUILDROOT OFF CACHE BOOL "Build from recalbox buildroot")
set(OPTION_SCREEN_WIDTH 1280 CACHE STRING "Custom screen width")
set(OPTION_SCREEN_HEIGHT 720 CACHE STRING "Custom screen height")
# needs libsmbclient port to the switch...
set(BUILD_SMBCLIENT OFF CACHE BOOL "Build with smbclient support")

add_subdirectory(libcross2d)

##############
# pplay
##############

project(pplay)

# pplay version
set(VERSION_MAJOR "3")
set(VERSION_MINOR "1")
set(PROJECT_AUTHOR "Cpasjuste")
set(TITLE_ID "PPLAY0001")

set(PPLAY_INC
        pplay
        pplay/filer
        pplay/filer/Browser
        pplay/filer/ftplib
        pplay/menus
        pplay/player
        pplay/scrapper
        )

file(GLOB PPLAY_SRC
        pplay/*.c*
        pplay/filer/*.c*
        pplay/filer/Browser/*.c*
        pplay/filer/ftplib/*.c*
        pplay/menus/*.c*
        pplay/player/*.c*
        pplay/scrapper/*.c*
        )

set(PPLAY_CFLAGS
        -D__PPLAY__
        -D__VERSION_MAJOR__=${VERSION_MAJOR}
        -D__VERSION_MINOR__=${VERSION_MINOR}
        -DM_PI=3.14159265358979323846
        -DM_SQRT2=1.41421356237309504880
        )

set(PPLAY_LDFLAGS curl)

if (BUILD_SMBCLIENT)
    list(APPEND PPLAY_CFLAGS -D__SMB_SUPPORT__)
    list(APPEND PPLAY_LDFLAGS dsm tasn1)
endif ()

##############
# mpv
##############
add_subdirectory(mpv)

##############
# pscrap
##############
add_subdirectory(pscrap)

#####################
# PLATORM SPECIFIC
#####################
if (PLATFORM_SWITCH)
    #####################
    # SWITCH PLATORM
    #####################
    list(APPEND PPLAY_INC ${DEVKITPRO}/portlibs/switch/include/SDL2)
    list(APPEND PPLAY_LDFLAGS)
elseif (PLATFORM_LINUX)
    #####################
    # LINUX PLATORM
    #####################
    list(APPEND PPLAY_INC /usr/include/SDL2)
    list(APPEND PPLAY_LDFLAGS)
endif ()

#####################
# pplay executable
#####################
add_executable(${CMAKE_PROJECT_NAME} ${PPLAY_SRC})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PPLAY_INC})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${PPLAY_CFLAGS})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
target_link_libraries(${CMAKE_PROJECT_NAME} cross2d mpv pscrap ${PPLAY_LDFLAGS})

#####################
# targets
#####################
include(libcross2d/cmake/targets.cmake)
