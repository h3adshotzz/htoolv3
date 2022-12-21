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

#ifndef __HTOOL_HTOOL_H__
#define __HTOOL_HTOOL_H__

#include <libhelper-file.h>

#include "version.h"

#define HTOOL_DEBUG 1


/**
 * \brief       The htool_client_t struct is created once the tool starts running
 *              and is used to retain information for the current instance, such as
 *              the command the tool was run with, the parsed file, optional flags
 *              and other properties.
 *
 *              The aim here is to prevent too many parameters being parsed around,
 *              so everything is stored in a single structure.
 *
 */
typedef struct __htool_client           htool_client_t;
struct __htool_client
{
    /* Loaded file info */
    char                *filename;  // loaded file path
    char                *args;      // argument string

    /* Flags */
    uint32_t             cmd;       // command
    uint32_t             opts;      // command options
    char                *arch;      // --arch value.

    /* Parsed binary */
    //htool_binary_t      *bin;       // parsed `filename`
};

/**
 * \brief       CLI Command option struct.
 *
 */
struct command {
    char        *cmdname;
    int          (*handler)(htool_client_t*, int, char*);
};


/**
 *  \brief      Macro for checking if a given bitmask is set on a value. The `flags`
 *              value would be, for example, client->cmd or client->opts, whereas
 *              the `mask` would be HTOOL_CLIENT_CMDFLAG_MACHO.
 *
 */
#define HTOOL_CLIENT_CHECK_FLAG (flags, mask)              ((flags & mask) == mask) ? 1 : 0


/**
 *  List of flags and options for htool. These are set when parsing the command line
 *  arguments in main.c. To check if a flag is set, use the macro:
 *
 *      htool_client_check_flags (client_flags, mask);
 *
 */
#define HTOOL_CLIENT_CMDFLAG_MACHO          0x10000000
#define HTOOL_CLIENT_MACHO_OPT_ARCH         (1 << 1)
#define HTOOL_CLIENT_MACHO_OPT_HEADER       (1 << 2)
#define HTOOL_CLIENT_MACHO_OPT_LCMDS        (1 << 3)
#define HTOOL_CLIENT_MACHO_OPT_LIBS         (1 << 4)
#define HTOOL_CLIENT_MACHO_OPT_SYMBOLS      (1 << 5)
#define HTOOL_CLIENT_MACHO_OPT_SYMDBG       (1 << 6)
#define HTOOL_CLIENT_MACHO_OPT_SYMSECT      (1 << 7)

//#define


/**
 *  Exit codes
 */
typedef enum htool_return_t
{
    HTOOL_RETURN_FAILURE,
    HTOOL_RETURN_SUCCESS
} htool_return_t;



#endif /* __htool_htool_h__ */
