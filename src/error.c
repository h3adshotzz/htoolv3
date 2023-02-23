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

#include <libhelper.h>
#include <libhelper-hlibc.h>
#include <libhelper-logger.h>

#include <stdint.h>
#include <stdarg.h>

#include "htool-error.h"

void
htool_error_throw (error_code_t error_code, char *fmt, ...)
{
    va_list arg;
    int done;

    /* fetch the correct error type */
    htool_error_t *err;
    for (int i = 0; i < ERROR_LIST_LEN; i++) {
        htool_error_t *tmp = &error_list[i];
        if (tmp->error_code == error_code) {
            err = tmp;
            break;
        }
    }

    fmt = mstrappend ("%s%s%s%s%s%s", RED "[*Error*] ", err->error_msg, ": ", fmt, RESET, "\n");

    va_start (arg, fmt);
    done = vfprintf (stdout, fmt, arg);
    va_end (arg);
}

htool_return_t 
error_test ()
{
    htool_error_throw (HTOOL_ERROR_INVALID_FILENAME, "test data");
    return HTOOL_RETURN_SUCCESS;
}