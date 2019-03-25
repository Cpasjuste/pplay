cmake_minimum_required(VERSION 3.0)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# delete cmake cache folder before changing this option
set(BUILD_LINUX OFF CACHE BOOL "Build with SDL2 support")
set(BUILD_WINDOWS OFF CACHE BOOL "Build with Windows support (MSYS/MinGW")
set(BUILD_VITA OFF CACHE BOOL "Build with PSP2 support")
set(BUILD_PS3 OFF CACHE BOOL "Build with PS3 support")
set(BUILD_3DS OFF CACHE BOOL "Build with 3DS support")
set(BUILD_SWITCH ON CACHE BOOL "Build with NX (SWITCH) support")

add_subdirectory(libcross2d)

##############
# pplay
##############

project(pplay)

# pplay version
set(VERSION_MAJOR "1")
set(VERSION_MINOR "5")
set(PROJECT_AUTHOR "Cpasjuste")
set(TITLE_ID "PPLAY0001")

set(PPLAY_INC
        pplay
        pplay/filers
        pplay/menus
        pplay/player
        pplay/ftplib
        #pplay/fftools
        )

file(GLOB PPLAY_SRC
        pplay/*.c*
        pplay/filers/*.c*
        pplay/menus/*.c*
        pplay/player/*.c*
        pplay/Browser/*.c*
        pplay/ftplib/*.c*
        #pplay/fftools/*.c*
        )

set(PPLAY_CFLAGS
        -D__PPLAY__
        -D__VERSION_MAJOR__=${VERSION_MAJOR}
        -D__VERSION_MINOR__=${VERSION_MINOR}
        -DM_PI=3.14159265358979323846
        -DM_SQRT2=1.41421356237309504880
        )

set(PPLAY_LDFLAGS curl)

##############
# mpv
##############
add_subdirectory(mpv)

#####################
# PLATORM SPECIFIC
#####################
if (BUILD_SWITCH)
    #####################
    # SWITCH PLATORM
    #####################
    list(APPEND PPLAY_INC ${DEVKITPRO}/portlibs/switch/include/SDL2)
    list(APPEND PPLAY_LDFLAGS)
elseif (BUILD_LINUX)
    #####################
    # LINUX PLATORM
    #####################
    list(APPEND PPLAY_INC /usr/include/SDL2)
    list(APPEND PPLAY_LDFLAGS lzma)
endif ()

#####################
# pplay executable
#####################
add_executable(${CMAKE_PROJECT_NAME}.elf ${PPLAY_SRC})
target_include_directories(${CMAKE_PROJECT_NAME}.elf PRIVATE ${PPLAY_INC})
target_compile_options(${CMAKE_PROJECT_NAME}.elf PRIVATE ${PPLAY_CFLAGS})
target_compile_options(${CMAKE_PROJECT_NAME}.elf PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
target_link_libraries(${CMAKE_PROJECT_NAME}.elf cross2d mpv ${PPLAY_LDFLAGS})

#####################
# targets
#####################
include(libcross2d/Targets.cmake)
