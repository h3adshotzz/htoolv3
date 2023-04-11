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

#include <libhelper.h>
#include <libhelper-logger.h>
#include <libhelper-macho.h>
#include <libhelper-file.h>

#include "htool-version.h"
#include "htool-loader.h"
#include "htool-client.h"
#include "htool-error.h"
#include "htool.h"

#include "colours.h"
#include "usage.h"


/* headers for commands */
#include "commands/macho.h"
#include "commands/analyse.h"


/* Command handler functions */

static htool_return_t
handle_command_file (htool_client_t *client);
static htool_return_t
handle_command_macho (htool_client_t *client);
static htool_return_t
handle_command_analyse (htool_client_t *client);

/* debug */
static htool_return_t
handle_command_debug (htool_client_t *client);


///////////////////////////////////////////////////////////////////////////////

/* Command optins definitions, using getopt.h option structs */

/* standard cli options */
static struct option standard_opts[] = {
    { "help",       no_argument,    NULL,   'h' },
    { "version",    no_argument,    NULL,   'V' },

    { NULL,         0,              NULL,   0 }
};

/* macho options */
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

/* analyse options */
static struct option analyse_cmd_opts[] = {
    { "analyse",    no_argument,        NULL,   'a' },
    { "list-all",   no_argument,        NULL,   'l' },
    { "extract",    required_argument,  NULL,   'e' },
    { NULL,         0,                  NULL,   0   }
};

/**
 *  List of top-level commands that htool provides
 */
static struct command commands[] = {
    { "file",       handle_command_file     },
    { "macho",      handle_command_macho    },
    { "analyse",    handle_command_analyse  },
    { "debug",      handle_command_debug    },
    { NULL,         NULL                    }
};


/* command counts */
#define STANDARD_OPTION_COUNT       sizeof(standardopts) / sizeof(standardopts[0])
#define HTOOL_COMMAND_COUNT         sizeof(commands) / sizeof(commands[0])

///////////////////////////////////////////////////////////////////////////////

static htool_return_t handle_command_file (htool_client_t *client)
{
    htool_error_throw (HTOOL_ERROR_NOT_IMPLEMENTED, "'file' not implemented.");
    return HTOOL_RETURN_FAILURE;
}

static htool_return_t handle_command_macho (htool_client_t *client)
{
    /* reset getopt */
    optind = 1;
    opterr = 1;

    /* set the appropriate flag for the client struct */
    client->cmd |= HTOOL_CLIENT_CMDFLAG_MACHO;

    /* parse the `file` options */
    int opt = 0;
    int optindex = 0;
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
    ci_logf ("File successfully loaded\n");

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

static htool_return_t handle_command_analyse (htool_client_t *client)
{
    /* reset getopt */
    htool_return_t res;
    optind = 1;
    opterr = 1;

    /* set the appropriate flag for the client struct */
    client->cmd |= HTOOL_CLIENT_CMDFLAG_ANALYSE;

    /* parse the `file` options */
    int opt = 0;
    int optindex = 2;
    while ((opt = getopt_long (client->argc, client->argv, "e:alHA", analyse_cmd_opts, &optindex)) > 0) {
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
                client->extract = strdup ((const char *) client->argv[client->argc-1]);

            /* default, print usage */
            case 'H':
            default:
                analyse_subcommand_usage (client->argc, client->argv, 0);
                break;
        }
    }

    /**
     *  Option:             None
     *  Description:        No option has been passed, so print the help menu again.
     */
    if (!client->opts) {
        analyse_subcommand_usage (client->argc, client->argv, 0);
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
    //ci_logf ("File successfully loaded\n");

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

static htool_return_t handle_command_debug (htool_client_t *client)
{
    error_test ();
    htool_error_throw (HTOOL_ERROR_INVALID_FILENAME, "HTOOL_ERROR_INVALID_FILENAME test");
    htool_error_throw (HTOOL_ERROR_INVALID_ARCH, "HTOOL_ERROR_INVALID_ARCH test");
    htool_error_throw (HTOOL_ERROR_NOT_IMPLEMENTED, "HTOOL_ERROR_NOT_IMPLEMENTED test");
    htool_error_throw (HTOOL_ERROR_FILE_LOADING, "HTOOL_ERROR_FILE_LOADING test");
    htool_error_throw (HTOOL_ERROR_FILETYPE, "HTOOL_ERROR_FILETYPE test");
    htool_error_throw (HTOOL_ERROR_GENERAL, "HTOOL_ERROR_GENERAL test");
    htool_error_throw (HTOOL_ERROR_ARCH, "HTOOL_ERROR_ARCH test");
}

///////////////////////////////////////////////////////////////////////////////

/* print version details */
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
#if HTOOL_DEBUG
        printf (BLUE "\n    HTool Version %s: %s; root:%s/%s_%s %s\n\n" RESET,
            HTOOL_BUILD_VERSION, __TIMESTAMP__, HTOOL_SOURCE_VERSION, HTOOL_BUILD_TYPE, BUILD_ARCH_CAP, BUILD_ARCH);
#endif

        printf (BOLD RED "\n  Extensions:\n", RESET);
        printf (BOLD DARK_WHITE "    Libhelper:        " RESET DARK_GREY "%s\n", libhelper_get_version_string());

    } else {
        printf ("-----------------------------------------------------\n");
        printf ("  HTool %s - Built " __TIMESTAMP__ "\n", HTOOL_BUILD_VERSION);
        printf (BLUE "  Source version: %s\n" RESET, HTOOL_SOURCE_VERSION);
        printf ("-----------------------------------------------------\n");
    }
}


int main(int argc, char **argv)
{
#if HTOOL_DEBUG
    debugf ("argc: %d\n", argc);
    for (int i = 0; i < argc; i++)
        debugf ("[%d]: %s\n", i, argv[i]);
#endif

    /* always print the version info (short) */
    print_version_detail (0);

    /* create a new client */
    htool_client_t *client = calloc (1, sizeof (htool_client_t));

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

    /* set client->argv and ->argc */
    client->argc = argc - 1;
    client->argv = (char **) malloc (client->argc * sizeof (char *));
    for (int i = 0; i < client->argc; i++) client->argv[i] = argv[i];

    /* try to get the filename */
    char *filename = argv[argc - 1];
    if (!filename) {
        htool_error_throw (HTOOL_ERROR_INVALID_FILENAME, "No filename provided");
        general_usage (argc, argv, 0);
        return EXIT_FAILURE;
    }
    client->filename = strdup (filename);
#if HTOOL_DEBUG
    debugf ("client->filename: %s\n", client->filename);
#endif

    /**
     *  work out which command has been invoked. Use the `optind` index from getopt.h
     *  to get the next item in the argv array. This value will be compared to each
     *  command name until there is either a match, at which point the handler can be
     *  invoked, or until there are no commands left, in that case the general usage
     *  menu is printed to the screen.
     *
     */
    char *cmdname = argv[optind];
    for (int i = 0; i < HTOOL_COMMAND_COUNT; i++) {
        struct command *c = &commands[i];
#if HTOOL_DEBUG
        debugf ("check: %s, %s\n", cmdname, c->cmdname);
#endif

        if (!c->cmdname || !cmdname)
            continue;

        if (!strcmp (c->cmdname, cmdname))
#if HTOOL_DEBUG
        {
            debugf ("matched command: %s\n", cmdname);
            int res = c->handler (client);
            debugf ("res: %d\n", res);
            return (res == HTOOL_RETURN_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
#else
            return (c->handler (client) == HTOOL_RETURN_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
#endif
    }

    /* if we get here, print the usage again */
    general_usage (argc, argv, 0);
    return EXIT_SUCCESS;
}
