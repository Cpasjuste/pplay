cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# psnes version
set(PPLAY_VERSION_MAJOR 1)
set(PPLAY_VERSION_MINOR 8)

# delete cmake cache folder before changing this option
set(BUILD_SFML OFF CACHE BOOL "Build with SFML support")
set(BUILD_SDL1 OFF CACHE BOOL "Build with SDL1 support (software, osmesa) support")
set(BUILD_SDL1_GL OFF CACHE BOOL "Build with SDL1 support (hardware rendering, fast, needs BUILD_SDL1=ON)")
set(BUILD_SDL2 OFF CACHE BOOL "Build with SDL2 (software) support")
set(BUILD_SDL2_GL OFF CACHE BOOL "Build with SDL2 support (hardware rendering, fast, needs BUILD_SDL2=ON)")
set(BUILD_PSP2 OFF CACHE BOOL "Build with PSP2 support")
set(BUILD_PS3 OFF CACHE BOOL "Build with PS3 support")
set(BUILD_3DS OFF CACHE BOOL "Build with 3DS support")
set(BUILD_NX ON CACHE BOOL "Build with NX (SWITCH) support")

set(BUILD_PFBA OFF CACHE BOOL "Build with PFBA support")
set(BUILD_SNES9X ON CACHE BOOL "Build with SNES9X support")

set(UPDATE_CONFIG ON CACHE BOOL "Force config file updating")

option(BUILD_SOUND "Build with audio support" ON)
option(BUILD_SCREENSHOT "Build with screenshot support" ON)
option(BUILD_NETPLAY "Build with screenshot support" OFF)
option(BUILD_GAMEPAD "Build with gamepad support" ON)
option(BUILD_GZIP "Build with gzip support" ON)
option(BUILD_ZIP "Build with zip support" ON)
option(BUILD_JMA "Build with jma support" OFF)
option(BUILD_DEBUGGER "Build with debugger support" OFF)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_DEBUG true CACHE BOOL "Debug build")
endif ()

# add libcross2d/libcross2dui library. If you want to port pfba to another platform,
# you should (may) only need to port libcross2d library.
include(libcross2dui/libcross2d/Toolchain.cmake)
add_subdirectory(libcross2dui)
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

execute_process(COMMAND "date" "+%Y.%m.%d" OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

project(psnes)

##############
# SNES9X
##############
set(SNES9X_INC
        snes9x
        snes9x/apu
        snes9x/apu/bapu
        snes9x/filter
        )

file(GLOB SNES9X_SRC
        snes9x/*.c*
        snes9x/apu/*.c*
        snes9x/apu/bapu/*.c*
        snes9x/apu/bapu/dsp/*.c*
        snes9x/apu/bapu/smp/*.c*
        snes9x/filter/*.c*
        )

list(REMOVE_ITEM SNES9X_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/snes9x/spc7110dec.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/snes9x/spc7110emu.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/snes9x/srtcemu.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/snes9x/apu/bapu/smp/algorithms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/snes9x/apu/bapu/smp/core.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/snes9x/apu/bapu/smp/memory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/snes9x/apu/bapu/smp/timing.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/snes9x/apu/bapu/smp/iplrom.cpp
        )

set(SNES9X_CFLAGS
        -fomit-frame-pointer -fno-exceptions
        -pedantic -Wall -W -Wno-unused-parameter
        -DHAVE_LIBPNG -DHAVE_MKSTEMP -DHAVE_STRINGS_H
        -DHAVE_SYS_IOCTL_H -DHAVE_STDINT_H -DRIGHTSHIFT_IS_SAR
        -DGFX_MULTI_FORMAT
        -D__PPLAY__
        -D__PPLAY_VERSION_MAJOR__=${PPLAY_VERSION_MAJOR}
        -D__PPLAY_VERSION_MINOR__=${PPLAY_VERSION_MINOR}
        )

if (BUILD_DEBUG)
    list(APPEND SNES9X_CFLAGS -g -O0)
endif (BUILD_DEBUG)

set(SNES9X_LDFLAGS m z png)

#####################
# OPTIONS
#####################
if (BUILD_SOUND)
    #list(APPEND SNES9X_CFLAGS -DUSE_THREADS)
    #list(APPEND SNES9X_LDFLAGS pthread)
else ()
    list(APPEND SNES9X_CFLAGS -DNOSOUND)
endif (BUILD_SOUND)
if (BUILD_SCREENSHOT)
endif (BUILD_SCREENSHOT)
if (BUILD_NETPLAY)
endif (BUILD_NETPLAY)
if (BUILD_GAMEPAD)
    list(APPEND SNES9X_CFLAGS -DJOYSTICK_SUPPORT)
endif (BUILD_GAMEPAD)
if (BUILD_GZIP)
    list(APPEND SNES9X_CFLAGS -DZLIB)
endif (BUILD_GZIP)
if (BUILD_ZIP)
    set(UNZIP_INC snes9x/unzip)
    set(UNZIP_SRC snes9x/unzip/ioapi.c snes9x/unzip/unzip.c)
    list(APPEND SNES9X_CFLAGS -DUNZIP_SUPPORT)
endif (BUILD_ZIP)
if (BUILD_JMA)
    set(JMA_INC snes9x/jma)
    file(GLOB JMA_SRC snes9x/jma/*.c*)
    list(APPEND SNES9X_CFLAGS -DJMA_SUPPORT)
endif (BUILD_JMA)
if (BUILD_DEBUGGER)
endif (BUILD_DEBUGGER)

##############
# psnes
##############
set(PPLAY_INC psnes)
file(GLOB PPLAY_SRC psnes/*.c*)

#####################
# PLATORM SPECIFIC
#####################
if (BUILD_PSP2)
    #####################
    # PSP2 PLATORM
    #####################
    list(APPEND SNES9X_LDFLAGS ScePower_stub)
elseif (BUILD_NX)
    #####################
    # NX PLATORM
    #####################
    list(APPEND SNES9X_CFLAGS -Wno-pedantic -Wno-missing-field-initializers -Wno-format-truncation)
elseif (BUILD_SDL2)
    #####################
    # SDL2 PLATORM
    #####################
endif (BUILD_PSP2)

#####################
# snes9x executable
#####################
add_executable(${CMAKE_PROJECT_NAME}.elf ${PPLAY_SRC} ${SNES9X_SRC} ${UNZIP_SRC} ${JMA_SRC})
target_include_directories(${CMAKE_PROJECT_NAME}.elf PRIVATE ${PPLAY_INC} ${SNES9X_INC} ${UNZIP_INC} ${JMA_INC})
target_compile_options(${CMAKE_PROJECT_NAME}.elf PRIVATE ${SNES9X_CFLAGS})
target_compile_options(${CMAKE_PROJECT_NAME}.elf PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
target_link_libraries(${CMAKE_PROJECT_NAME}.elf cross2dui ${SNES9X_LDFLAGS})

#####################
# PSP2 (vita) target
#####################
if (BUILD_PSP2)
    vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME}.elf)
    add_custom_target(${PROJECT_NAME}_vita_release.zip
            DEPENDS ${PROJECT_NAME}.self
            COMMAND rm -rf ${CMAKE_BINARY_DIR}/release/psnes
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/psnes/sce_sys
            COMMAND ${VITASDK}/bin/vita-mksfoex -s TITLE_ID="PSNES0001" "pSNES" ${CMAKE_BINARY_DIR}/release/psnes/sce_sys/param.sfo
            COMMAND cp -f ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.self ${CMAKE_BINARY_DIR}/release/psnes/eboot.bin
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/psnes/data/skin ${CMAKE_BINARY_DIR}/release/psnes/
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/psnes/data/psp2/skin ${CMAKE_BINARY_DIR}/release/psnes/
            #COMMAND cp -f ${CMAKE_SOURCE_DIR}/psnes/data/databases/Super_Nintendo_Entertainment_System_Parent-Clone_20180601-221617_dat-o-matic.xml ${CMAKE_BINARY_DIR}/release/psnes/db.xml
            COMMAND cd ${CMAKE_BINARY_DIR}/release/psnes && zip -r ../../${PROJECT_NAME}-${PPLAY_VERSION_MAJOR}.${PPLAY_VERSION_MINOR}_vita.vpk . && cd ${CMAKE_BINARY_DIR}
            )
endif (BUILD_PSP2)

#####################
# NX (SWITCH) target
#####################
if (BUILD_NX)
    set_target_properties(${PROJECT_NAME}.elf PROPERTIES LINK_FLAGS "-specs=${DEVKITPRO}/libnx/switch.specs")
    add_custom_target(${PROJECT_NAME}.nro
            DEPENDS ${PROJECT_NAME}.elf
            COMMAND nacptool --create "pSNES" "Cpasjuste" "${PPLAY_VERSION_MAJOR}.${PPLAY_VERSION_MINOR}" psnes.nacp
            COMMAND elf2nro ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro --icon=${CMAKE_SOURCE_DIR}/psnes/data/nx/icon.jpg --nacp=psnes.nacp)
    add_custom_target(${PROJECT_NAME}_switch_release.zip
            DEPENDS ${PROJECT_NAME}.nro
            COMMAND rm -rf ${CMAKE_BINARY_DIR}/release/psnes
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/psnes
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/psnes/roms
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/psnes/titles
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/psnes/previews
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/psnes/icons
            COMMAND cp -f ${CMAKE_BINARY_DIR}/psnes.nro ${CMAKE_BINARY_DIR}/release/psnes/psnes.nro
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/psnes/data/skin ${CMAKE_BINARY_DIR}/release/psnes/
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/psnes/data/nx/skin ${CMAKE_BINARY_DIR}/release/psnes/
            COMMAND cp -f ${CMAKE_SOURCE_DIR}/psnes/data/databases/Super_Nintendo_Entertainment_System_Parent-Clone_20180601-221617_dat-o-matic.xml ${CMAKE_BINARY_DIR}/release/psnes/db.xml
            COMMAND cd ${CMAKE_BINARY_DIR}/release && zip -r ../${PROJECT_NAME}-${PPLAY_VERSION_MAJOR}.${PPLAY_VERSION_MINOR}_switch.zip psnes && cd ${CMAKE_BINARY_DIR}
            )
endif (BUILD_NX)
