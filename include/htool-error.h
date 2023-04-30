//===----------------------------------------------------------------------===//
//
//                         === The HTool Project ===
//
//  This  document  is the property of "Is This On?" It is considered to be
//  confidential and proprietary and may not be, in any form, reproduced or
//  transmitted, in whole or in part, without express permission of Is This
//  On?.
//
//  Copyright (C) 2023, Harry Moulton - Is This On? Holdings Ltd
//
//  Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

/**
 *  NOTE:       To make analysing output in CI easier, this header defines a number
 *              of error codes and messages so they are consistent throughout the
 *              program.
*/

#include "htool.h"

htool_return_t error_test ();

////////////////

typedef struct htool_error_t
{
    int      error_code;
    char    *error_msg;

    //int    error_severity;
} htool_error_t;

typedef enum error_code_t
{
    HTOOL_ERROR_INVALID_FILENAME,
    HTOOL_ERROR_INVALID_ARCH,

    HTOOL_ERROR_NOT_IMPLEMENTED,
    HTOOL_ERROR_FILE_LOADING,
    HTOOL_ERROR_FILETYPE,
    HTOOL_ERROR_GENERAL,
    HTOOL_ERROR_ARCH,

} error_code_t;

/* Define all errors here */
static htool_error_t error_list[] =
{
    { HTOOL_ERROR_INVALID_FILENAME,     "Invalid filename" },
    { HTOOL_ERROR_INVALID_ARCH,         "Invalid architecture" },

    { HTOOL_ERROR_NOT_IMPLEMENTED,      "Not implemented" },
    { HTOOL_ERROR_FILE_LOADING,         "Failed to load file" },
    { HTOOL_ERROR_FILETYPE,             "Unknown/Incorrect Filetype" },
    { HTOOL_ERROR_GENERAL,              "General Error" },
    { HTOOL_ERROR_ARCH,                 "No architecture specified" },

    { NULL, NULL, },
};

#define ERROR_LIST_LEN              (sizeof (error_list) / sizeof (error_list[0])) - 1

void
htool_error_throw (error_code_t error_code, char *fmt, ...);
