cmake_minimum_required(VERSION 3.2)
#set(CMAKE_VERBOSE_MAKEFILE ON)

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
set(OPTION_SMB2 OFF CACHE BOOL "Build with smb2 support")

###################
# MODULES
###################
add_subdirectory(libcross2d)
add_subdirectory(pscrap)

##############
# pplay
##############
project(pplay)

# pplay version
set(VERSION_MAJOR "3")
set(VERSION_MINOR "6")
set(PROJECT_AUTHOR "Cpasjuste")
set(TITLE_ID "PPLAY0001")

# deps
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package(BZip2 REQUIRED)
find_package(MPV REQUIRED)
find_package(ffmpeg COMPONENTS avfilter avcodec avformat avutil swscale swresample)
find_package(ass REQUIRED)

# sources
file(GLOB PPLAY_SRC
        src/*.c*
        src/filer/*.c*
        src/filer/Browser/*.c*
        src/filer/ftplib/*.c*
        src/menus/*.c*
        src/player/*.c*
        src/scrapper/*.c*
        )

# includes
set(PPLAY_INC
        src
        src/filer
        src/filer/Browser
        src/filer/ftplib
        src/menus
        src/player
        src/scrapper
        ${MPV_INCLUDE_DIRS}
        ${ASS_INCLUDE_DIRS}
        ${FFMPEG_INCLUDE_DIRS}
        ${BZIP2_INCLUDE_DIR}
        )

# cflags
set(PPLAY_CFLAGS
        -D__PPLAY__
        -D__VERSION_MAJOR__=${VERSION_MAJOR}
        -D__VERSION_MINOR__=${VERSION_MINOR}
        #-DM_PI=3.14159265358979323846
        #-DM_SQRT2=1.41421356237309504880
        )

# ldflags
set(PPLAY_LDFLAGS
        ${MPV_LIBRARY}
        ${ASS_LIBRARIES}
        ${FFMPEG_LIBRARIES}
        ${BZIP2_LIBRARIES}
        )

if (OPTION_SMB2)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(OPENSSL_INCLUDE_DIR "" CACHE STRING "" FORCE)
    add_subdirectory(libsmb2)
    list(APPEND PPLAY_CFLAGS -D__SMB2__)
    list(APPEND PPLAY_INC libsmb2/include libsmb2/include/smb2)
    list(APPEND PPLAY_LDFLAGS smb2)
endif ()

#####################
# PLATORM SPECIFIC
#####################
if (PLATFORM_SWITCH)
    #####################
    # SWITCH PLATORM
    #####################
    set(MPV_LIBRARIES mpv)
    list(APPEND PPLAY_INC)
    list(APPEND PPLAY_LDFLAGS
            swscale swresample avformat avfilter avcodec avutil
            lzma opus vpx ass freetype fribidi png bz2 usbhsfs ntfs-3g lwext4
            )
elseif (PLATFORM_PS4)
    set(PS4_PKG_TITLE "pplay @ cpasjuste")
    set(PS4_PKG_TITLE_ID "PPLA00001")
    set(PS4_PKG_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")
    list(APPEND PPLAY_INC)
    list(APPEND PPLAY_LDFLAGS opus freetype fribidi png bz2)
elseif (PLATFORM_LINUX)
    #####################
    # LINUX PLATORM
    #####################
endif ()

#####################
# pplay executable
#####################
add_executable(${CMAKE_PROJECT_NAME} ${PPLAY_SRC})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PPLAY_INC})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${PPLAY_CFLAGS})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
target_link_libraries(${CMAKE_PROJECT_NAME} cross2d pscrap ${PPLAY_LDFLAGS})

#####################
# targets
#####################
include(libcross2d/cmake/targets.cmake)
