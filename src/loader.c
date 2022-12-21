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

#include "htool-loader.h"


htool_binary_t *
htool_binary_create ()
{
    htool_binary_t *bin = calloc (1, sizeof (htool_binary_t));
    bin->version = HTOOL_BINARY_VERSION_1_0;
    return bin;
}

htool_binary_t *
htool_binary_load_file (const char *path)
{
    htool_binary_t *bin = htool_binary_create ();

    /* verify the file path */
    if (!path) {
        errorf ("htool_binary_load_file: file path is invalid\n");
        return HTOOL_RETURN_FAILURE;
    }
    bin->filepath = (char *) strdup (path);

    /* create the file descriptor */
    int fd = open (bin->filepath, O_RDONLY);

    /* calc the file size */
    struct stat st;
    fstat (fd, &st);
    bin->size = st.st_size;

    /* mmap the file */
    bin->data = mmap (NULL, bin->size, PROT_READ, MAP_PRIVATE, fd, 0);
    close (fd);

    /* verify the map was sucessful */
    if (bin->data == MAP_FAILED) {
        errorf ("htool_binary_load_file: failed to map file: %s\n", bin->filepath);
        return HTOOL_RETURN_FAILURE;
    }

    return (bin) ? bin : NULL;
}

htool_binary_t *
htool_binary_parser (htool_binary_t *bin)
{
    uint32_t magic = *(uint32_t *) bin->data;
    debugf ("bin->data: 0x%08x\n", magic);

    /**
     * Check if the binary is an ELF format.
    */
    if (htool_binary_detect_elf (bin, magic)) {
        warningf ("TODO: Implement ELF parsing\n");
        return HTOOL_RETURN_SUCCESS;
    }


   /**
     * Check if the binary is a Mach-O format.
    */
    if (htool_binary_detect_macho (bin, magic)){
        warningf ("TODO: Implement Mach-O parsing\n");
        return HTOOL_RETURN_SUCCESS;
    }

    errorf ("Could not determine file type: 0x%08x (err: 0x%08x)\n", magic, HTOOL_ERROR_FILETYPE);
    return HTOOL_RETURN_SUCCESS;
}


htool_binary_t *
htool_binary_load_and_parse (const char *path)
{
    htool_binary_t *bin = htool_binary_load_file (path);
    if (!bin) return HTOOL_RETURN_FAILURE;

    return htool_binary_parser (bin);
}


//===----------------------------------------------------------------------===//
//                         Mach-O Loader Functions
//===----------------------------------------------------------------------===//

htool_return_t
htool_binary_detect_macho (htool_binary_t *bin, uint32_t magic)
{
    /* Use the libhelper api to determine the macho magic type */
    mach_header_type_t mh_type = mach_header_verify (magic);

    /* Depending on the value of mh_type, modify the bin->flags */
    if (mh_type == MH_TYPE_MACHO64) {
        bin->flags |= HTOOL_BINARY_FILETYPE_MACHO64;
    } else if (mh_type == MH_TYPE_MACHO32) {
        bin->flags |= HTOOL_BINARY_FILETYPE_MACHO32;
    } else if (mh_type == MH_TYPE_FAT) {
        bin->flags |= HTOOL_BINARY_FILETYPE_FAT;
    } else {
        /* No Mach-O was detected, so return a failure */
        return HTOOL_RETURN_FAILURE;
    }

    /* If the flags were set, return success */
    return HTOOL_RETURN_SUCCESS;
}


//===----------------------------------------------------------------------===//
//                          ELF Loader Functions
//===----------------------------------------------------------------------===//

htool_return_t
htool_binary_detect_elf (htool_binary_t *bin, uint32_t magic)
{
    /**
     * NOTE: The current ELF api is temporary, this feature will be expanded later,
     *       for the moment we're just trying to detect whether a file is an ELF.
    */
   return (magic == ELF_MAGIC || magic == ELF_CIGAM) ? HTOOL_RETURN_SUCCESS : HTOOL_RETURN_FAILURE;
}