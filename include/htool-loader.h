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
#define HTOOL_BINARY_FIRMWARETYPE_KEXT          0x00000002
#define HTOOL_BINARY_FIRMWARETYPE_IBOOT         0x00000003
#define HTOOL_BINARY_FIRMWARETYPE_SEP           0x00000004


/**
 * \brief       HTool Binary Loader structure.
 * 
 *              Each file that is loaded by HTool will be parsed into one of
 *              these structures which tracks the original file properties as
 *              well as any additional filetype-specific structures and a pointer
 *              to a firmware struct.
 */
typedef struct htool_binary_t
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

    /* firmware struct pointer */
    void *firmware;
} htool_binary_t;


/**
 * \brief       Load a given file into a new `htool_binary_t` struct by
 *              only populating the "raw data properties". 
 * 
 * \param   path    Filepath to load.
 * 
 * \returns     A new `htool_binary_t` structure with the loaded file.
 */
htool_binary_t *
htool_binary_load_file (const char *path);

/**
 * \brief       Parse a given binary and populate the appropriate filetype
 *              fields
 * 
 * \param   bin     The `htool_binary_t` to parse.
 * 
 * \return      An updated `bin` with the appropriate filetype properties set,
 *              and the flags updated to reflect the parsed filetype.
 * 
 */
htool_binary_t *
htool_binary_parser (htool_binary_t *bin);

/**
 * \brief       Load and Parse a given file. Just calls the load function, verifies
 *              the result, and returns the parser function.
 * 
 * \param   path    Filepath to load.
 * 
 * \return      The result from calling htool_binary_parser() with `path`.
 */
htool_binary_t *
htool_binary_load_and_parse (const char *path);

/**
 * \brief       Iterate through the list of architectures within a given `bin` to
 *              find and return the `macho_t` for the desired `arch_name`, or NULL.
 * 
 * \param   bin         The `htool_binary_t` containing multiple architectures a a FAT file.
 * \param   arch_name   The desired architecture to find.
 * 
 * \return      Either the desired `macho_t`, or NULL.
 */
macho_t *
htool_binary_select_arch (htool_binary_t *bin, char *arch_name);

/**
 * \brief       Detect whether a magic number value is a Mach-O.
 * 
 * \param   bin     The binary to update the flags for.
 * \param   magic   The magic number to verify.
 * 
 * \return      Success or Failure based on the result of the magic check.
 */
htool_return_t
htool_binary_detect_macho (htool_binary_t *bin, uint32_t magic);

/**
 * \brief       Detect whether a magic number value is an ELF.
 * 
 * \param   bin     The binary to update the flags for.
 * \param   magic   The magic number to verify.
 * 
 * \return      Success or Failure based on the result of the magic check.
 */
htool_return_t
htool_binary_detect_elf (htool_binary_t *bin, uint32_t magic);

/**
 * \brief       Detect whether a magic number value is an Image4.
 * 
 * \param   bin     The binary to update the flags for.
 * \param   magic   The magic number to verify.
 * 
 * \return      Success or Failure based on the result of the magic check.
 */
htool_return_t
htool_binary_detect_image4 (htool_binary_t *bin, uint32_t magic);

#endif /* __htool_loader_h__ */
