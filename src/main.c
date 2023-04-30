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

#include <stdlib.h>
#include <getopt.h>
#include <assert.h>

#include <libhelper.h>
#include <libhelper-logger.h>
#include <libhelper-macho.h>
#include <libhelper-file.h>

#include <libarch.h>

#include "htool-version.h"
#include "htool-loader.h"
#include "htool-client.h"
#include "htool-error.h"
#include "htool.h"

#include "colours.h"
#include "usage.h"

#include "commands/macho.h"
#include "commands/analyse.h"
#include "commands/disassembler.h"


/* Command handler functions */
static htool_return_t
handle_command_file (htool_client_t *client);
static htool_return_t
handle_command_macho (htool_client_t *client);
static htool_return_t
handle_command_analyse (htool_client_t *client);
static htool_return_t
handle_command_disass (htool_client_t *client);

///////////////////////////////////////////////////////////////////////////////

/**
 *  Command Option arrays using `option` from getopt.h to define each
 *  commands available options.
*/

/**
 * \brief   Standard HTool options if a command is not specified
 * 
 */
static struct option standard_opts[] = {
    { "help",       no_argument,    NULL,   'h' },
    { "version",    no_argument,    NULL,   'V' },

    { NULL,         0,              NULL,   0 }
};

/**
 * \brief   HTool `macho` command options.
 * 
 */
static struct option macho_cmd_opts[] = {
    { "arch",       required_argument,  NULL,   'a' },
    { "verbose",    no_argument,        NULL,   'v' },
    { "help",       no_argument,        NULL,   'H' },
    { "header",     no_argument,        NULL,   'h' },
    { "loadcmd",    no_argument,        NULL,   'l' },
    { "libs",       no_argument,        NULL,   'L' },
    { "symbols",    no_argument,        NULL,   's' },
    { "sym-dbg",    no_argument,        NULL,   'D' },
    { "sym-sect",   no_argument,        NULL,   'C' },
    { "signing",    no_argument,        NULL,   'S' },

    { NULL,         0,                  NULL,    0  }
};

/**
 * \brief   HTool `analyse` command options.
 * 
 */
static struct option analyse_cmd_opts[] = {
    { "help",       no_argument,        NULL,   'h' },
    { "analyse",    no_argument,        NULL,   'a' },
    { "list-all",   no_argument,        NULL,   'l' },
    { "extract",    required_argument,  NULL,   'e' },
    { NULL,         0,                  NULL,   0   }
};

/**
 * \brief   HTool `disass` command options.
 * 
 */
static struct option disass_cmd_opts[] = {
    { "arch",               required_argument,  NULL,   'a' },
    { "verbose",            no_argument,        NULL,   'v' },
    { "help",               no_argument,        NULL,   'h' },

    { "disassemble",        no_argument,        NULL,   'd' },
    { "disassemble-all",    no_argument,        NULL,   'D' },

    { "base-address",       required_argument,  NULL,   'b' },
    { "stop-address",       required_argument,  NULL,   's' },
    { "count",              required_argument,  NULL,   'c' },

    { NULL,                 0,                  NULL,    0  },
};

/**
 * \brief   List of commands available in HTool. Each command will have
 *          a corresponding options array above.
 */
static struct command commands[] = {
    { "file",       handle_command_file     },
    { "macho",      handle_command_macho    },
    { "analyse",    handle_command_analyse  },
    { "disass",     handle_command_disass   },
    { NULL,         NULL                    }
};


/* Number of commands and standard options */
#define STANDARD_OPTION_COUNT       sizeof(standardopts) / sizeof(standardopts[0])
#define HTOOL_COMMAND_COUNT         sizeof(commands) / sizeof(commands[0])

///////////////////////////////////////////////////////////////////////////////

/**
 *  NOTE:   Command handler are defined here as static functions, so they're not
 *          defined in another header file.
 */

/**
 * \brief   Command handler for the `file` command.
 * 
 */
static htool_return_t handle_command_file (htool_client_t *client)
{
    htool_error_throw (HTOOL_ERROR_NOT_IMPLEMENTED, "'file' not implemented.");
    return HTOOL_RETURN_FAILURE;
}

/**
 * \brief   Command handler for the `macho` command.
 * 
 */
static htool_return_t handle_command_macho (htool_client_t *client)
{
    /* reset getopt */
    optind = 1, opterr = 1;

    /* set the appropriate flag for the client struct */
    client->cmd |= HTOOL_CLIENT_CMDFLAG_MACHO;

    /* parse the `file` options */
    int opt = 0, optindex = 0;
    while ((opt = getopt_long (client->argc, client->argv, "vhlLsSDCA", macho_cmd_opts, &optindex)) > 0) {
        switch (opt) {

            /* -a, --arch */
            case 'a':
                client->opts |= HTOOL_CLIENT_MACHO_OPT_ARCH;
                client->arch = strdup ((const char *) optarg);
                break;

            /* -v, --verbose */
            case 'v':
                client->opts |= HTOOL_CLIENT_MACHO_OPT_VERBOSE;
                break;

            /* -h, --header */
            case 'h':
                client->opts |= HTOOL_CLIENT_MACHO_OPT_HEADER;
                break;

            /* -l, --loadcmd */
            case 'l':
                client->opts |= HTOOL_CLIENT_MACHO_OPT_LCMDS;
                break;

            /* -L, --libs */
            case 'L':
                client->opts |= HTOOL_CLIENT_MACHO_OPT_LIBS;
                break;

            /* -s, --symbols */
            case 's':
                client->opts |= HTOOL_CLIENT_MACHO_OPT_SYMBOLS;
                break;

                /* -sym-dbg */
                case 'D':
                    client->opts |= HTOOL_CLIENT_MACHO_OPT_SYMDBG;
                    break;

                /* -sym-sect */
                case 'C':
                    client->opts |= HTOOL_CLIENT_MACHO_OPT_SYMSECT;
                    break;

            /* -S, --signing */
            case 'S':
                client->opts |= HTOOL_CLIENT_MACHO_OPT_CODE_SIGNING;
                break;

            /* default, print usage */
            case 'H':
            default:
                macho_subcommand_usage (client->argc, client->argv, 0);
                return HTOOL_RETURN_FAILURE;
        }
    }

    /**
     *  Option:             None
     *  Description:        No option has been passed, so print the help menu again.
     */
    if (!client->opts) {
        macho_subcommand_usage (client->argc, client->argv, 0);
        return HTOOL_RETURN_FAILURE;
    }

    /**
     *  Load the file into client->bin, if the file is not valid exit with an error
     *  message.
     */
    if ((client->bin = htool_binary_load_and_parse (client->filename)) == HTOOL_RETURN_FAILURE) {
        htool_error_throw (HTOOL_ERROR_INVALID_FILENAME, "%s", client->filename);
        exit (EXIT_FAILURE);
    }

    /**
     *  Option:             -h, --header
     *  Description:        Print out the header of a Mach-O, DYLD Shared Cache or
     *                      a FAT/Universal Binary.
     */
    if (client->opts & HTOOL_CLIENT_MACHO_OPT_HEADER)
        htool_print_header (client);

    /**
     *  Option:             -l. --loadcmd
     *  Description:        Print out the Load and Segment Commands contained in a
     *                      Mach-O file.
     */
    if (client->opts & HTOOL_CLIENT_MACHO_OPT_LCMDS)
        htool_print_load_commands (client);

    /**
     *  Option:             -L, --libs
     *  Description:        Print out the Shared Libraries contained in a Mach-O file.
     */
    if (client->opts & HTOOL_CLIENT_MACHO_OPT_LIBS)
        htool_print_shared_libraries (client);

    /**
     *  Option:             -s, --symbols
     *  Description:        Print out the Static Symbols contained in a Mach-O file.
     */
    if (client->opts & HTOOL_CLIENT_MACHO_OPT_SYMBOLS)
        htool_print_static_symbols (client);

    /**
     *  Option:             -S, --signing
     *  Description:        Print code-signature information contained in a Mach-O file.
     */
    if (client->opts & HTOOL_CLIENT_MACHO_OPT_CODE_SIGNING)
        htool_print_code_signature (client);

    return HTOOL_RETURN_SUCCESS;
}

/**
 * \brief   Command handler for the `analyse` command.
 * 
 */
static htool_return_t handle_command_analyse (htool_client_t *client)
{
    /* reset getopt */
    htool_return_t res;
    optind = 1, opterr = 1;

    /* set the appropriate flag for the client struct */
    client->cmd |= HTOOL_CLIENT_CMDFLAG_ANALYSE;

    /* parse the `file` options */
    int opt = 0, optindex = 2;
    while ((opt = getopt_long (client->argc, client->argv, "e:alhA", analyse_cmd_opts, &optindex)) > 0) {
        switch (opt) {

            /* -a, --analyse */
            case 'a':
                client->opts |= HTOOL_CLIENT_ANALYSE_OPT_ANALYSE;
                break;

            /* -l, --list-all */
            case 'l':
                client->opts |= HTOOL_CLIENT_ANALYSE_OPT_LIST_ALL;
                break;

            /* -e, --extract */
            case 'e':
                client->opts |= HTOOL_CLIENT_ANALYSE_OPT_EXTRACT;
                client->extract = strdup ((const char *) optarg);
                break;

            /* default, print usage */
            case 'H':
            default:
                analyse_subcommand_usage (client->argc, client->argv, 0);
                return HTOOL_RETURN_FAILURE;
        }
    }

    /**
     *  Load the file into client->bin, if the file is not valid exit with an error
     *  message.
     */
    if ((client->bin = htool_binary_load_and_parse (client->filename)) == HTOOL_RETURN_FAILURE) {
        htool_error_throw (HTOOL_ERROR_INVALID_FILENAME, "%s", client->filename);
        exit (EXIT_FAILURE);
    }

    /**
     *  Option:             -a, --analyse
     *  Description:        Run a complete analysis of the given firmware file.
     */
    if (client->opts & HTOOL_CLIENT_ANALYSE_OPT_ANALYSE) {
        res = htool_generic_analyse (client);
        if (res == HTOOL_RETURN_FAILURE) return res;
    }

    /**
     *  Option:             -l, --list-all
     *  Description:        List all embedded binaries / Mach-O's.
     */
    if (client->opts & HTOOL_CLIENT_ANALYSE_OPT_LIST_ALL)
        res = htool_analyse_list_all (client);

    /**
     *  Option:             -e, --extract
     *  Description:        Extract an embedded firmware that matches the given
     *                      name.
     */
    if (client->opts & HTOOL_CLIENT_ANALYSE_OPT_EXTRACT) {
        res = htool_analyse_extract (client);
        if (res) printf (ANSI_COLOR_GREEN "[*] Extracted %s\n" RESET, client->extract);
    }

    return HTOOL_RETURN_SUCCESS;
}

/**
 * \brief   Command handler for the `disass` command.
 * 
 */
static htool_return_t handle_command_disass (htool_client_t *client)
{
    /* reset getopt */
    optind = 1, opterr = 1;

    /* set the appropriate flag for the client struct */
    client->cmd |= HTOOL_CLIENT_CMDFLAG_DISASS;
    char *base_addr, *stop_address, *size;

    /* parse the `disass` options */
    int opt = 0, optindex = 2;
    while ((opt = getopt_long (client->argc, client->argv, "Ddb:c:s:hA", disass_cmd_opts, &optindex)) > 0) {
        switch (opt) {

            /* -D, --disassemble-all */
            case 'D':
                client->opts |= HTOOL_CLIENT_DISASS_OPT_DISASSEMBLE_FULL;
                break;

            /* -d, --disassemble */
            case 'd':
                client->opts |= HTOOL_CLIENT_DISASS_OPT_DISASSEMBLE_QUICK;
                break;

            /* -b, --base-address */
            case 'b':
                client->opts |= HTOOL_CLIENT_DISASS_OPT_BASE_ADDRESS;
                client->base_address = strtoull (optarg, NULL, 16);
                break;

            /* -s, --stop-address */
            case 's':
                client->opts |= HTOOL_CLIENT_DISASS_OPT_STOP_ADDRESS;
                client->stop_address = strtoull (optarg, NULL, 16);
                break;

            /* -c, --count */
            case 'c':
                client->opts |= HTOOL_CLIENT_DISASS_OPT_COUNT;
                //size = strdup (client->argv[optind + optarg]);
                client->size = strtoull (optarg, NULL, 16);
                break;


            /* default, print usage */
            case 'h':
            default:
                disass_subcommand_usage (client->argc, client->argv, 0);
                return HTOOL_RETURN_FAILURE;
        }
    }

    /**
     *  Load the file into client->bin, if the file is not valid exit with an error
     *  message.
     */
    if ((client->bin = htool_binary_load_and_parse (client->filename)) == HTOOL_RETURN_FAILURE) {
        htool_error_throw (HTOOL_ERROR_INVALID_FILENAME, "%s", client->filename);
        exit (EXIT_FAILURE);
    }

    /**
     *  Option:             -d, --disassemble
     *  Description:        Run a quick disassembly of a binary with minimal annotations.
     */
    if (client->opts & HTOOL_CLIENT_DISASS_OPT_DISASSEMBLE_QUICK)
        htool_disassemble_binary_quick (client);
    
    return HTOOL_RETURN_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief   Print version detail, either as a full version output or a shortened
 *          banner just showing the source version.
 */
void print_version_detail (int opt)
{
    if (opt == 1) {
        printf (BOLD "Copyright (C) Is This On? Holdings Ltd.\n\n" RESET);

        printf (BOLD RED "  Debug Information:\n", RESET);
        printf (BOLD DARK_WHITE "    Build Version:    " RESET DARK_GREY "%s (%s)\n", HTOOL_BUILD_VERSION, HTOOL_SOURCE_VERSION);
        printf (BOLD DARK_WHITE "    Build Target:     " RESET DARK_GREY "%s-%s\n", BUILD_TARGET, BUILD_ARCH);        
        printf (BOLD DARK_WHITE "    Build time:       " RESET DARK_GREY "%s\n", __TIMESTAMP__);
        printf (BOLD DARK_WHITE "    Compiler:         " RESET DARK_GREY "%s\n", __VERSION__);

        printf (BOLD DARK_WHITE "    Platform:         " RESET DARK_GREY);
#if HTOOL_MACOS_PLATFORM_TYPE == HTOOL_PLATFORM_TYPE_APPLE_SILICON
        printf ("apple-silicon (Apple Silicon)\n");
#else
        printf ("intel-genuine (Intel Genuine)\n");
#endif
        printf (BLUE "\n    HTool Version %s: %s; root:%s/%s_%s %s\n" RESET,
            HTOOL_BUILD_VERSION, __TIMESTAMP__, HTOOL_SOURCE_VERSION, HTOOL_BUILD_TYPE, BUILD_ARCH_CAP, BUILD_ARCH);

        printf (BOLD RED "\n  Extensions:\n", RESET);
        printf (BOLD DARK_WHITE "    Libhelper:        " RESET DARK_GREY "%s\n", libhelper_get_version_string());
        printf (BOLD DARK_WHITE "    Libarch:          " RESET DARK_GREY "%s\n", LIBARCH_SOURCE_VERSION);

    } else {
        printf ("-----------------------------------------------------\n");
        printf ("  HTool %s - Built " __TIMESTAMP__ "\n", HTOOL_BUILD_VERSION);
        printf (BLUE "  Source version: %s\n" RESET, HTOOL_SOURCE_VERSION);
        printf ("-----------------------------------------------------\n");
    }
}


int main(int argc, char **argv)
{
    /* always print the version info (short) */
    print_version_detail (0);

    /* create a new client */
    htool_client_t *client = calloc (1, sizeof (htool_client_t));

    /* set client->argv and ->argc */
    client->argc = argc;
    client->argv = (char **) malloc (client->argc * sizeof (char *));
    for (int i = 0; i < client->argc; i++) client->argv[i] = argv[i];

    /**
     * check for the --help or --version options. These are handled differently to the
     * other commands.
     */
    int opt = 0, optindex = 0;
    opterr = 0;
    while ((opt = getopt_long (argc, argv, "hVA", standard_opts, &optindex)) > 0) {
        switch (opt) {

            /* --help command */
            case 'h':
                client->opts |= HTOOL_CLIENT_GENERIC_OPT_HELP;
                break;

            /* --version command */
            case 'V':
                print_version_detail (1);
                return EXIT_SUCCESS;

            /* default */
            default:
                break;
        }
    }

    /* try to get the filename */
    char *filename = argv[argc - 1];
    if (!filename) {
        htool_error_throw (HTOOL_ERROR_INVALID_FILENAME, "No filename provided");
        general_usage (argc, argv, 0);
        return EXIT_FAILURE;
    }
    client->filename = strdup (filename);

    /**
     *  Work out which command has been invoked. Use the `optind` index from getopt.h
     *  to get the next item in the argv array. This value will be compared to each
     *  command name until there is either a match, at which point the handler can be
     *  invoked, or until there are no commands left, in that case the general usage
     *  menu is printed to the screen.
     *
     */
    char *cmdname = argv[optind];
    for (int i = 0; i < HTOOL_COMMAND_COUNT; i++) {
        struct command *c = &commands[i];

        if (!c->cmdname || !cmdname)
            continue;

        if (!strcmp (c->cmdname, cmdname))
            return (c->handler (client) == HTOOL_RETURN_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    /* if we get here, print the usage again */
    general_usage (argc, argv, 0);
    return EXIT_SUCCESS;
}
