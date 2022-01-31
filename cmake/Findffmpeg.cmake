# A Simple FFMPEG Finder.
# (c) Tuomas Virtanen 2016 (Licensed under MIT license)
# Usage:
# find_package(ffmpeg COMPONENTS avcodec avutil ...)
#
# Declares:
#  * FFMPEG_FOUND
#  * FFMPEG_INCLUDE_DIRS
#  * FFMPEG_LIBRARIES
#
# Also declares ${component}_FOUND for each component, eg. avcodec_FOUND etc.
#

set(FFMPEG_SEARCH_PATHS
        /usr/local
        /usr
        /opt
        )

set(FFMPEG_COMPONENTS
        avcodec
        avformat
        avdevice
        avfilter
        avutil
        swresample
        swscale
        )

set(FFMPEG_INCLUDE_DIRS)
set(FFMPEG_LIBRARIES)
set(FFMPEG_FOUND TRUE)

# Walk through all components declared above, and try to find the ones that have been asked
foreach (comp ${FFMPEG_COMPONENTS})
    list(FIND ffmpeg_FIND_COMPONENTS ${comp} _index)
    if (${_index} GREATER -1)
        # Component requested, try to look up the library and header for it.
        find_path(${comp}_INCLUDE_DIR lib${comp}/${comp}.h
                HINTS
                PATH_SUFFIXES include
                PATHS ${FFMPEG_SEARCH_PATHS}
                )
        find_library(${comp}_LIBRARY ${comp}
                HINTS
                PATH_SUFFIXES lib
                PATHS ${FFMPEG_SEARCH_PATHS}
                )

        # If library and header was found, set proper variables
        # Otherwise print out a warning!
        if (${comp}_LIBRARY AND ${comp}_INCLUDE_DIR)
            set(${comp}_FOUND TRUE)
            list(APPEND FFMPEG_INCLUDE_DIRS ${${comp}_INCLUDE_DIR})
            list(APPEND FFMPEG_LIBRARIES ${${comp}_LIBRARY})
        else ()
            set(FFMPEG_FOUND FALSE)
            set(${comp}_FOUND FALSE)
            message(FATAL_ERROR "Could not find component: ${comp}")
        endif ()

        # Mark the temporary variables as hidden in the ui
        mark_as_advanced(${${comp}_LIBRARY} ${${comp}_INCLUDE_DIR})
    endif ()
endforeach ()

if (FFMPEG_FOUND)
    message(STATUS "Found FFMPEG: ${FFMPEG_LIBRARIES}")
else ()
    message(WARNING "Could not find FFMPEG")
endif ()

mark_as_advanced(FFMPEG_COMPONENTS FFMPEG_SEARCH_PATHS)
