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

cmake_minimum_required(VERSION 3.15)

############################ CONFIGURATION #####################################

# Check this module should be built
#if (NOT LIBHELPER_)

target_include_directories(htool
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}../
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(htool
    PUBLIC
        main.c
        usage.c
        loader.c
)
