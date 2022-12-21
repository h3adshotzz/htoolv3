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

#ifndef __HTOOL_LOADER_H__
#define __HTOOL_LOADER_H__

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libhelper.h>
#include <libhelper-macho.h>
#include <libhelper-logger.h>

#include "elf/elf-loader.h"
#include "htool.h"

/**
 *  Define a header version, some features in the future might write this
 *  structure to a file. It might be useful to keep track of the struct
 *  version.
 *
 */
#define HTOOL_BINARY_VERSION_1_0                0x01


/**
 *  The HTOol Binary structure has a flag value ...
 *
 *      flag = 0x00000000
 *               8      1
 *
 *
 *
 */
#define HTOOL_BINARY_FILETYPE_RAWBINARY         0xa0000000
#define HTOOL_BINARY_FILETYPE_MACHO32           0xb0000000
#define HTOOL_BINARY_FILETYPE_MACHO64           0xc0000000
#define HTOOL_BINARY_FILETYPE_MACHO64_32        0xd0000000
#define HTOOL_BINARY_FILETYPE_FAT               0xe0000000
#define HTOOL_BINARY_FILETYPE_ELF               0xf0000000

#define HTOOL_BINARY_FIRMWARETYPE_KERNEL        0x00000001
#define HTOOL_BINARY_FIRMWARETYPE_IBOOT         0x00000002
#define HTOOL_BINARY_FIRMWARETYPE_SEP           0x00000003


/**
 *
 */
typedef struct __htool_binary           htool_binary_t;
struct __htool_binary
{
    uint32_t        version;

    unsigned char   *data;
    uint32_t        size;
    char            *filepath;

    uint32_t        flags;

    uint32_t debug;
};



htool_binary_t *
htool_binary_load_file (const char *path);

htool_binary_t *
htool_binary_parser (htool_binary_t *bin);

htool_binary_t *
htool_binary_load_and_parse (const char *path);


htool_return_t
htool_binary_detect_macho (htool_binary_t *bin, uint32_t magic);

htool_return_t
htool_binary_detect_elf (htool_binary_t *bin, uint32_t magic);

#endif /* __htool_loader_h__ */
