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
#include <libhelper-image4.h>
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
 *  The HTool Binary structure has a flag value used to describe a range
 *  of things about the loaded file, this includes the file type, the
 *  firmware component type, etc. The current format, as of 1.0, is:
 *
 *      flag = 0x00000000
 *               8      1
 *
 *  [1]     -   Describes the detected "Firmware Type", this could be iBoot,
 *              the Kernel, SEP, or something else.
 *
 *  [2]     -   Undefined.
 *
 *  [3]     -   Undefined.
 *
 *  [4]     -   Undefined.
 *
 *  [5]     -   Undefined.
 *
 *  [6]     -   Undefined.
 *
 *  [7]     -   Undefined.
 *
 *  [8]     -   Describes the "File Type", this could be a 32-bit Mach-O,
 *              64-bit Mach-O, FAT or DYLD Shared Cache. These masks enable
 *              other features, for example if the value is set as a Mach-O
 *              then the `macho_list` structure will be treated as a single
 *              Mach-O.
 *
 */
#define HTOOL_BINARY_FILETYPE_IMAGE4            0x90000000
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
    /* htool_binary version */
    uint32_t        version;

    /* raw data properties */
    unsigned char   *data;
    uint32_t        size;
    char            *filepath;

    /* flags */
    uint32_t        flags;

    /* filetype-specific fields */
    fat_info_t      *fat_info;
    HSList          *macho_list;
    //elf_t           *elf;
    image4_t        *image4;


    uint32_t debug;
};



htool_binary_t *
htool_binary_load_file (const char *path);

htool_binary_t *
htool_binary_parser (htool_binary_t *bin);

htool_binary_t *
htool_binary_load_and_parse (const char *path);

macho_t *
htool_binary_select_arch (htool_binary_t *bin, char *arch_name);


htool_return_t
htool_binary_detect_macho (htool_binary_t *bin, uint32_t magic);

htool_return_t
htool_binary_detect_elf (htool_binary_t *bin, uint32_t magic);

htool_return_t
htool_binary_detect_image4 (htool_binary_t *bin, uint32_t magic);

#endif /* __htool_loader_h__ */
