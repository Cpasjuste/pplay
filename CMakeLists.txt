cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# pplay version
set(PPLAY_VERSION_MAJOR 1)
set(PPLAY_VERSION_MINOR 0)

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

set(UPDATE_CONFIG OFF CACHE BOOL "Force config file updating")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_DEBUG true CACHE BOOL "Debug build")
endif ()

include(libcross2d/Toolchain.cmake)
add_subdirectory(libcross2d)
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

execute_process(COMMAND "date" "+%Y.%m.%d" OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

set(KIT_VERSION_MAJOR "1")
set(KIT_VERSION_MINOR "0")
set(KIT_VERSION_PATCH "6")
set(KIT_VERSION ${KIT_VERSION_MAJOR}.${KIT_VERSION_MINOR}.${KIT_VERSION_PATCH})
add_definitions(
        -DKIT_VERSION_MAJOR=${KIT_VERSION_MAJOR}
        -DKIT_VERSION_MINOR=${KIT_VERSION_MINOR}
        -DKIT_VERSION_PATCH=${KIT_VERSION_PATCH}
)

project(pplay)

##############
# pplay
##############
set(PPLAY_INC
        pplay
        SDL_kitchensink/include
        SDL_kitchensink/include/kitchensink
        SDL_kitchensink/include/kitchensink/internal
        /opt/devkitpro/portlibs/switch/include/SDL2
        )
file(GLOB_RECURSE PPLAY_SRC
        pplay/*.c*
        SDL_kitchensink/src/*.c*
        )

set(PPLAY_CFLAGS
        -D__PPLAY__
        -D__PPLAY_VERSION_MAJOR__=${PPLAY_VERSION_MAJOR}
        -D__PPLAY_VERSION_MINOR__=${PPLAY_VERSION_MINOR}
        )

if (BUILD_DEBUG)
    list(APPEND PPLAY_CFLAGS -g -O0)
endif (BUILD_DEBUG)

set(PPLAY_LDFLAGS
        swscale swresample avformat avcodec avutil ass fribidi
        m png z)

#####################
# PLATORM SPECIFIC
#####################
if (BUILD_NX)
    #####################
    # NX PLATORM
    #####################
elseif (BUILD_SDL2)
    #####################
    # SDL2 PLATORM
    #####################
    list(APPEND PPLAY_LDFLAGS -lpthread -llzma)
endif (BUILD_NX)

#####################
# pplay executable
#####################
add_executable(${CMAKE_PROJECT_NAME}.elf ${PPLAY_SRC})
target_include_directories(${CMAKE_PROJECT_NAME}.elf PRIVATE ${PPLAY_INC})
target_compile_options(${CMAKE_PROJECT_NAME}.elf PRIVATE ${PPLAY_CFLAGS})
target_compile_options(${CMAKE_PROJECT_NAME}.elf PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
target_link_libraries(${CMAKE_PROJECT_NAME}.elf cross2d ${PPLAY_LDFLAGS})

#####################
# NX (SWITCH) target
#####################
if (BUILD_NX)
    set_target_properties(${PROJECT_NAME}.elf PROPERTIES LINK_FLAGS "-specs=${DEVKITPRO}/libnx/switch.specs")
    add_custom_target(${PROJECT_NAME}.nro
            DEPENDS ${PROJECT_NAME}.elf
            COMMAND nacptool --create "pPLAYER" "Cpasjuste" "${PPLAY_VERSION_MAJOR}.${PPLAY_VERSION_MINOR}" pplay.nacp
            COMMAND elf2nro ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro --icon=${CMAKE_SOURCE_DIR}/pplay/data/nx/icon.jpg --nacp=pplay.nacp)
    add_custom_target(${PROJECT_NAME}_switch_release.zip
            DEPENDS ${PROJECT_NAME}.nro
            COMMAND rm -rf ${CMAKE_BINARY_DIR}/release/pplay
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/pplay
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/pplay/media
            COMMAND cp -f ${CMAKE_BINARY_DIR}/pplay.nro ${CMAKE_BINARY_DIR}/release/pplay/pplay.nro
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/pplay/data/skin ${CMAKE_BINARY_DIR}/release/pplay/
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/pplay/data/nx/skin ${CMAKE_BINARY_DIR}/release/pplay/
            COMMAND cd ${CMAKE_BINARY_DIR}/release && zip -r ../${PROJECT_NAME}-${PPLAY_VERSION_MAJOR}.${PPLAY_VERSION_MINOR}_switch.zip pplay && cd ${CMAKE_BINARY_DIR}
            )
endif (BUILD_NX)
