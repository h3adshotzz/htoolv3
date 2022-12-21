##===----------------------------------------------------------------------===//
##
##                                  HTool
##
##  This  document  is the property of "Is This On?" It is considered to be
##  confidential and proprietary and may not be, in any form, reproduced or
##  transmitted, in whole or in part, without express permission of Is This
##  On?.
##
##  Copyright (C) 2022, Harry Moulton - Is This On? Holdings Ltd
##
##  Harry Moulton <me@h3adsh0tzz.com>
##
##===----------------------------------------------------------------------===//

cmake_minimum_required (VERSION 3.15)

############################ CONFIGURATION #####################################


############################## PROJECT #########################################

# Declare the project
project("HTool" VERSION ${HTOOL_VERSION_NUMBER} LANGUAGES C)

# Declare the executable
add_executable (htool)

# Add Libhelper
add_subdirectory(libhelper)

# HTool public headers.
target_include_directories(htool
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(htool libhelper)

# All the source files are located in htool/
add_subdirectory(src/)

# Add other subdirectories
#add_subdirectory(tools)
#add_subdirectory(tests)

#include(config/install.cmake)
