//===----------------------------------------------------------------------===//
//
//                         === The HTool Project ===
//
//  This  document  is the property of "Is This On?" It is considered to be
//  confidential and proprietary and may not be, in any form, reproduced or
//  transmitted, in whole or in part, without express permission of Is This
//  On?.
//
//  Copyright (C) 2022, Harry Moulton - Is This On? Holdings Ltd
//
//  Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

#ifndef __HTOOL_H__
#define __HTOOL_H__

/* General definitions to make the code nicer to read */
#define HTOOL_PRIVATE     static

#include <libhelper-logger.h>

/**
 *  Exit codes
 */
typedef enum htool_return_t
{
    HTOOL_RETURN_FAILURE,
    HTOOL_RETURN_SUCCESS,
    HTOOL_RETURN_VOID,
    HTOOL_RETURN_EXIT
} htool_return_t;


/**
 * Colour codes
 */
#define WHITE            "\x1b[38;5;254m"
#define DARK_WHITE       "\x1b[38;5;251m"
#define DARK_GREY        "\x1b[38;5;243m"
#define YELLOW           "\x1b[38;5;214m"
#define DARK_YELLOW      "\x1b[38;5;94m"
#define RED              "\x1b[38;5;88m"
#define BLUE             "\x1b[38;5;32m"
#define BOLD             "\x1b[1m"
#define GREEN            "\x1b[32m"
#define MAGENTA          "\x1b[35m"
#define CYAN             "\x1b[36m"
#define RESET            "\x1b[0m"

#define DEBUG_FLAG      debugf ("flag");

#endif /* __htool_h__ */
