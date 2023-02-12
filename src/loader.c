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

#include <libhelper.h>
#include <libhelper-image4.h>

#include "htool-loader.h"
#include "htool-client.h"

#include "darwin/darwin.h"

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
    const uint32_t fat_header_size = sizeof (fat_header_t);
    uint32_t magic = *(uint32_t *) bin->data;


    /**
     *  Check if the binary is an ELF format.
    */
    if (htool_binary_detect_elf (bin, magic)) {
        warningf ("TODO: Implement ELF parsing\n");
        return HTOOL_RETURN_SUCCESS;
    }

    /**
     *  Check if the binary is a Mach-O format.
     */
    if (htool_binary_detect_macho (bin, magic)) {
        
        /**
         *  The idea here is that we parse the files as normal. If the file type is
         *  a regular Mach-O, as set by the flag, then the `macho_list` property is
         *  treated as a single file, rather than a list of macho_t.
         *
         *  However, if the file is a FAT/Universal file, then we know to treat this
         *  array as an actual array. Other file types have their own fields.
         */
        if (bin->flags == HTOOL_BINARY_FILETYPE_MACHO64) {

            /**
             *  Loading 64-bit Mach-O files:
             *
             *  Try to load the macho from the mapped file in `data`. If it works, then
             *  append the macho to the macho_list. As mentioned, if the flag is set
             *  correctly (which it is for us to be here) then other calls to the loader
             *  API will know to only look at the first element in this list.
             */
            macho_t *m64 = macho_64_create_from_buffer (bin->data);
            if (!m64) {
                errorf ("Failed to load macho\n");
                return HTOOL_RETURN_FAILURE;
            }
            warningf ("m64->size: %d\n", m64->size);

            /* clear the list to ensure this is the only element */
            bin->macho_list = NULL;
            bin->macho_list = h_slist_append (bin->macho_list, m64);
        
        } else if (bin->flags == HTOOL_BINARY_FILETYPE_FAT) {

            /**
             * 
             */
            fat_header_t *fat = calloc (1, fat_header_size);
            bin->fat_info = calloc (1, sizeof (fat_info_t));

            /* copy the header data from the binary to bin->fat_info */
            memcpy (fat, (void *) (bin->data), fat_header_size);
            fat = swap_fat_header_bytes (fat);

            /* check if the fat header is valid */
            if (!fat) {
                errorf ("htool_binary_load: could not load FAT/Universal Binary: %s\n", bin->filepath);
                return HTOOL_RETURN_FAILURE;
            }

            /* check the fat archive has at least 1 mach-o */
            if (!fat->nfat_arch) {
                errorf ("htool_binary_load: could not load empty FAT/Universal Binary: %s\n", bin->filepath);
                return HTOOL_RETURN_FAILURE;
            }

            /**
             * Loop through nfat_arch times and collect each architecture descriptor
             * struct.
             */
            uint32_t arch_size = sizeof (fat_arch_t);
            uint32_t offset = fat_header_size;
            for (int i = 0; i < (int) fat->nfat_arch; i++) {

                /* copy the arch from (bin->data + offset) */
                fat_arch_t *arch = calloc (1, arch_size);
                memcpy (arch, (void *) (bin->data + offset), arch_size);

                /* swap the byte of 'arch' */
                arch = swap_fat_arch_bytes (arch);

                /* add it to the list */
                bin->fat_info->archs = h_slist_append (bin->fat_info->archs, arch);

                /* increment the offset */
                offset += arch_size;

                arch = NULL;
                free (arch);
            }

            /* copy the fat header and free it */
            bin->fat_info->header = calloc (1, fat_header_size);
            memcpy (bin->fat_info->header, fat, fat_header_size);
            free (fat);

            /**
             *  Now the FAT file header has been parsed we understand the Mach-O architectures
             *  that are contained within it, where they are placed, their size, etc. The next
             *  step is to go through the arch list and parse each one. 
             */
            for (int i = 0; i < h_slist_length (bin->fat_info->archs); i++) {

                fat_arch_t *arch = (fat_arch_t *) h_slist_nth_data (bin->fat_info->archs, i);

                void *raw = calloc (1, arch->size);
                memcpy (raw, (void *) (bin->data + arch->offset), arch->size);

                /**
                 *  Check the magic value of the discovered Mach-O file. It's either going
                 *  to be 32-bit or 64-bit, we don't get FAT files embedded in FAT files. 
                 */
                mach_header_t *arch_mh_hdr = (mach_header_t *) raw;
                mach_header_type_t arch_mh_type = mach_header_verify (arch_mh_hdr->magic);

                if (arch_mh_type == MH_TYPE_MACHO64) {

                    /* try to load 64-bit image */
                    macho_t *macho = macho_64_create_from_buffer ((unsigned char *) raw);
                    if (!macho) errorf ("htool_binary_load: Could not load Mach-O from FAT file: %s\n", mach_header_get_cpu_string (arch->cputype, arch->cpusubtype));

                    /* if the macho was loaded successfully, add it to the list */
                    bin->macho_list = h_slist_append (bin->macho_list, macho);
                } else {

                    /* try to load 32-bit image */
                    warningf ("32-bit macho loading not supported yet\n");
                    macho_t *macho = NULL;
                    bin->macho_list = h_slist_append (bin->macho_list, macho);
                }

                /* clear 'raw' for the next cycle */
                raw = NULL;
                free (raw);
            } 
        } else {
            /* implement */
            errorf ("cannot load file with mask: 0x%08x\n", bin->flags);
            return HTOOL_RETURN_FAILURE;
        }

        /**
         *  Check if the Mach-O that has been loaded is a Kernel, if it is,
         *  set the correct flag.
         */
        if (darwin_detect_firmware_component_kernel (bin))
            bin->flags |= HTOOL_BINARY_FIRMWARETYPE_KERNEL;

        return bin;
    } 
    
    /**
     *  Check if the binary is an Image4
     */
    if (htool_binary_detect_image4 (bin, magic)) {
        printf ("image4 detected\n");
        return bin;
    }

    /** TODO: Check if the binary is an iBoot, SecureROM or SEP. */

    errorf ("Could not determine file type: 0x%08x (err: 0x%08x)\n", magic, HTOOL_ERROR_FILETYPE);
    return NULL;
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

macho_t *
htool_binary_select_arch (htool_binary_t *bin, char *arch_name)
{
    debugf ("htool_binary_select_arch: arch_name: %s\n", arch_name);
    for (int i = 0; i < h_slist_length (bin->fat_info->archs); i++) {
        fat_arch_t *arch = (fat_arch_t *) h_slist_nth_data (bin->fat_info->archs, i);
        char *cpu_name = mach_header_get_cpu_string (arch->cputype, arch->cpusubtype);

        /* if teh cpu_name doesn't match arch_name, try the next item */
        if (!strcmp (cpu_name, arch_name))
            return (macho_t *) h_slist_nth_data (bin->macho_list, i);
    }
    errorf ("could not find arch\n");
    return NULL;
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

//===----------------------------------------------------------------------===//
//                          Image4 Loader Functions
//===----------------------------------------------------------------------===//

htool_return_t
htool_binary_detect_image4 (htool_binary_t *bin, uint32_t magic)
{
    image4_t *tmp = malloc (sizeof (image4_t));
    tmp->path = bin->filepath;
    tmp->size = bin->size;
    tmp->data = bin->data;

    img4type_t type = image4_get_file_type (tmp);

    if (type) {
        bin->flags |= HTOOL_BINARY_FILETYPE_IMAGE4;
        return HTOOL_RETURN_SUCCESS;
    }

    return HTOOL_RETURN_FAILURE;
}