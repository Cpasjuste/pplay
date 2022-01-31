# A Simple libass Finder.
# (c) Tuomas Virtanen 2016 (Licensed under MIT license)
# Usage:
# find_package(ass)
#
# Declares:
#  * ASS_FOUND
#  * ASS_INCLUDE_DIRS
#  * ASS_LIBRARIES
#

set(ASS_SEARCH_PATHS
        /usr/local/
        /usr/
        /opt
        )

find_path(ASS_INCLUDE_DIR ass/ass.h
        HINTS
        PATH_SUFFIXES include
        PATHS ${ASS_SEARCH_PATHS}
        )

find_library(ASS_LIBRARY
        NAMES ass
        HINTS
        PATH_SUFFIXES lib
        PATHS ${ASS_SEARCH_PATHS}
        )

if (ASS_INCLUDE_DIR AND ASS_LIBRARY)
    set(ASS_FOUND TRUE)
endif ()

if (ASS_FOUND)
    set(ASS_LIBRARIES ${ASS_LIBRARY})
    set(ASS_INCLUDE_DIRS ${ASS_INCLUDE_DIR})
    message(STATUS "Found libass: ${ASS_LIBRARIES}")
else ()
    message(FATAL_ERROR "Could not find libass")
endif ()

mark_as_advanced(ASS_LIBRARY ASS_INCLUDE_DIR ASS_SEARCH_PATHS)
