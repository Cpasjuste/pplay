cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# pplay version
set(VERSION_MAJOR "1")
set(VERSION_MINOR "5")
set(PROJECT_AUTHOR "Cpasjuste")
set(TITLE_ID "PPLAY0001")

# delete cmake cache folder before changing this option
set(BUILD_LINUX OFF CACHE BOOL "Build with SDL2 support")
set(BUILD_WINDOWS OFF CACHE BOOL "Build with Windows support (MSYS/MinGW")
set(BUILD_VITA OFF CACHE BOOL "Build with PSP2 support")
set(BUILD_PS3 OFF CACHE BOOL "Build with PS3 support")
set(BUILD_3DS OFF CACHE BOOL "Build with 3DS support")
set(BUILD_SWITCH ON CACHE BOOL "Build with NX (SWITCH) support")

include(libcross2d/Toolchain.cmake)
add_subdirectory(libcross2d)

add_definitions(-DKIT_VERSION_MAJOR=1 -DKIT_VERSION_MINOR=0 -DKIT_VERSION_PATCH=6)

project(pplay)

##############
# pplay
##############
set(PPLAY_INC
        pplay
        pplay/filers
        pplay/menus
        pplay/player
        pplay/ftplib
        #pplay/fftools
        SDL_kitchensink/include
        SDL_kitchensink/include/kitchensink
        SDL_kitchensink/include/kitchensink/internal)

file(GLOB PPLAY_SRC
        pplay/*.c*
        pplay/filers/*.c*
        pplay/menus/*.c*
        pplay/player/*.c*
        pplay/Browser/*.c*
        pplay/ftplib/*.c*
        #pplay/fftools/*.c*
        SDL_kitchensink/src/*.c*
        SDL_kitchensink/src/internal/*.c*
        SDL_kitchensink/src/internal/audio/*.c*
        SDL_kitchensink/src/internal/subtitle/*.c*
        SDL_kitchensink/src/internal/subtitle/renderers/*.c*
        SDL_kitchensink/src/internal/utils/*.c*
        SDL_kitchensink/src/internal/video/*.c*
        )

set(PPLAY_CFLAGS
        -D__PPLAY__
        -D__VERSION_MAJOR__=${VERSION_MAJOR}
        -D__VERSION_MINOR__=${VERSION_MINOR})

set(PPLAY_LDFLAGS swscale swresample avformat avfilter avcodec avutil ass fribidi curl pthread png z m)

#####################
# PLATORM SPECIFIC
#####################
if (BUILD_SWITCH)
    #####################
    # SWITCH PLATORM
    #####################
    list(APPEND PPLAY_INC ${DEVKITPRO}/portlibs/switch/include/SDL2)
    list(APPEND PPLAY_LDFLAGS mbedtls mbedcrypto mbedx509)
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
target_link_libraries(${CMAKE_PROJECT_NAME}.elf cross2d ${PPLAY_LDFLAGS})

#####################
# targets
#####################
include(libcross2d/Targets.cmake)
