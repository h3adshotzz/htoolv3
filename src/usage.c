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

#include "usage.h"

#include <stdio.h>
#include <string.h>

void general_usage (int argc, char *argv[], int err)
{
    char *name = strchr (argv[0], '/');
    fprintf ((err) ? stderr : stdout,
    "Usage: %s [SUBCOMMAND] [OPTIONS] PATH\n" \
    "\n" \
    "Commands:\n" \
    "  file             Identify file and print header.\n" \
    "  macho            Mach-O parser.\n" \
    "  dyld             Dyld Shared Cache parser.\n" \
    "  kernel           XNU Kernel/Kernelcache parser.\n" \
    "  iboot            iBoot parser.\n" \
    "  sep              SEP OS/Firmware parser.\n" \
    "  devtree          DeviceTree parser.\n" \
    "\n"\
    "Options:\n" \
    "  --help           HTool Usage info.\n" \
    "  --version        HTool Version info.\n" \
    "\n",

    (name ? name + 1 : argv[0]));
}

void file_subcommand_usage (int argc, char *argv[], int err)
{
    char *name = strchr (argv[0], '/');
    fprintf ((err) ? stderr : stdout,
    "Usage: %s file PATH\n" \
    "\n"\
    "Options:\n" \
    "  --help           HTool Usage info.\n" \
    "\n",

    (name ? name + 1 : argv[0]));
}

void macho_subcommand_usage (int argc, char *argv[], int err)
{
    char *name = strchr (argv[0], '/');
    fprintf ((err) ? stderr : stdout,
    "Usage: %s macho [OPTIONS] PATH\n" \
    "\n" \
    "Commands:\n" \
    "  -h, --header     Print the Mach-O/FAT Header.\n" \
    "  -l, --loadcmd    Print the Load Commands in the Mach-O.\n" \
    "  -L, --libs       Print the Shared Libraries linked to the Mach-O.\n" \
    "  -s, --symbols    Print the Symbols contained in the Mach-O.\n" \
    "       -sym-dbg        Print additional debug symbols.\n" \
    "       -sym-sect       Pritn additional section info.\n" \
    "\n"\
    "Options:\n" \
    "  --arch=ARCH      Specify architecture (e.g. arm64e, arm64, x86_64, ...)\n" \
    "  --help           HTool Usage info.\n" \
    "\n",

    (name ? name + 1 : argv[0]));
}