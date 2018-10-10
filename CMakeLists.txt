cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# pplay version
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(PROJECT_AUTHOR "Cpasjuste")
set(TITLE_ID "PPLAY0001")

# delete cmake cache folder before changing this option
set(BUILD_SFML OFF CACHE BOOL "Build with SFML support")
set(BUILD_SDL1 OFF CACHE BOOL "Build with SDL1 support (software, osmesa) support")
set(BUILD_SDL1_GL OFF CACHE BOOL "Build with SDL1 support (hardware rendering, fast, needs BUILD_SDL1=ON)")
set(BUILD_SDL2 ON CACHE BOOL "Build with SDL2 support")
set(BUILD_SDL2_SOFT OFF CACHE BOOL "Build with SDL2 support (software rendering, needs BUILD_SDL2=ON)")
set(BUILD_VITA OFF CACHE BOOL "Build with PSP2 support")
set(BUILD_PS3 OFF CACHE BOOL "Build with PS3 support")
set(BUILD_3DS OFF CACHE BOOL "Build with 3DS support")
set(BUILD_SWITCH OFF CACHE BOOL "Build with NX (SWITCH) support")

include(libcross2d/Toolchain.cmake)
add_subdirectory(libcross2d)

add_definitions(-DKIT_VERSION_MAJOR=1 -DKIT_VERSION_MINOR=0 -DKIT_VERSION_PATCH=6)

project(pplay)

##############
# pplay
##############
set(PPLAY_INC
        pplay
        SDL_kitchensink/include
        SDL_kitchensink/include/kitchensink
        SDL_kitchensink/include/kitchensink/internal)

file(GLOB_RECURSE PPLAY_SRC
        pplay/*.c*
        SDL_kitchensink/src/*.c*)

set(PPLAY_CFLAGS
        -D__PPLAY__
        -D__VERSION_MAJOR__=${VERSION_MAJOR}
        -D__VERSION_MINOR__=${VERSION_MINOR})

set(PPLAY_LDFLAGS swscale swresample avformat avcodec avutil ass fribidi m png z)

#####################
# PLATORM SPECIFIC
#####################
if (BUILD_SWITCH)
    #####################
    # NX PLATORM
    #####################
    list(APPEND PPLAY_INC ${DEVKITPRO}/portlibs/switch/include/SDL2)
elseif (BUILD_SDL2)
    #####################
    # SDL2 PLATORM
    #####################
    list(APPEND PPLAY_INC /usr/include/SDL2)
    list(APPEND PPLAY_LDFLAGS -lpthread -llzma)
endif (BUILD_SWITCH)

#####################
# pplay executable
#####################
add_executable(${CMAKE_PROJECT_NAME}.elf ${PPLAY_SRC})
target_include_directories(${CMAKE_PROJECT_NAME}.elf PRIVATE ${PPLAY_INC})
target_compile_options(${CMAKE_PROJECT_NAME}.elf PRIVATE ${PPLAY_CFLAGS})
target_compile_options(${CMAKE_PROJECT_NAME}.elf PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
target_link_libraries(${CMAKE_PROJECT_NAME}.elf cross2d ${PPLAY_LDFLAGS})

#####################
# targets
#####################
include(libcross2d/Targets.cmake)
