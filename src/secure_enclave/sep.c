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

#include "secure_enclave/sep.h"

#define IS64(image) (*(uint8_t *)(image) & 1)

#define MACHO(p) ((*(unsigned int *)(p) & ~1) == 0xfeedface)
#define MIN(x,y) ((x) < (y) ? (x) : (y))

static uint32_t
_sep_macho_calc_size (unsigned char *data, uint32_t size)
{
    mach_header_32_t *hdr = (mach_header_32_t *) data;
    unsigned char *base = data + sizeof (mach_header_32_t);
    size_t end, tsize = 0;

    if (size < 1024) return 0;
    if (!MACHO(data)) return 0;
    if (IS64(data)) base += 4;

    for (int i = 0; i < hdr->ncmds; i++) {
        mach_load_command_t *lc = (mach_load_command_t *) base;
        if (lc->cmd == LC_SEGMENT) {
            mach_segment_command_32_t *seg = (mach_segment_command_32_t *) base;
            end = seg->fileoff + seg->filesize;
            if (tsize < end) tsize = end;
        } else if (lc->cmd == LC_SEGMENT_64) {
            mach_segment_command_64_t *seg = (mach_segment_command_64_t *) base;
            end = seg->fileoff + seg->filesize;
            if (tsize < end) tsize = end;
        }
        base = base + lc->cmdsize;
    }
    return tsize;
}

/* idk why matteyeux does this, it's only modifying two bytes and doesn't have
    any effect on the resulting binary */
static uint32_t
_sep_macho_restore_linkedit (unsigned char *data, uint32_t size)
{
    mach_header_32_t *hdr = (mach_header_32_t *) data;
    unsigned char *base;
    uint64_t delta = 0;
    uint64_t min = -1;
    int is64 = 0;

    if (size < 1024) return -1;
    if (!MACHO(data)) return -1;
    if (IS64(data)) is64 = 4;

    base = data + sizeof (mach_header_32_t) + is64;
    for (int i = 0; i < hdr->ncmds; i++) {
        mach_load_command_t *lc = (mach_load_command_t *) base;
        if (lc->cmd == LC_SEGMENT) {
            mach_segment_command_32_t *seg = (mach_segment_command_32_t *) base;
            if (strcmp (seg->segname, "__PAGEZERO") && min > seg->vmaddr)
                min = seg->vmaddr;
        } else if (lc->cmd == LC_SEGMENT_64) {
            mach_segment_command_64_t *seg = (mach_segment_command_64_t *) base;
            if (strcmp (seg->segname, "__PAGEZERO") && min > seg->vmaddr)
                min = seg->vmaddr;
        }
        base = base + lc->cmdsize;
    }

    base = data + sizeof (mach_header_32_t) + is64;
    for (int i = 0; i < hdr->ncmds; i++) {
        mach_load_command_t *cmd = (mach_load_command_t *) base;
        if (cmd->cmd == LC_SEGMENT) {
            mach_segment_command_32_t *seg = (mach_segment_command_32_t *) base;
            if (!strcmp(seg->segname, "__LINKEDIT")) {
                delta = seg->vmaddr - min - seg->fileoff;
                seg->fileoff += delta;
            }
        }
        if (cmd->cmd == LC_SEGMENT_64) {
            mach_segment_command_64_t *seg = (mach_segment_command_64_t *) base;
            if (!strcmp(seg->segname, "__LINKEDIT")) {
                delta = seg->vmaddr - min - seg->fileoff;
                seg->fileoff += delta;
            }
        }
        if (cmd->cmd == LC_SYMTAB) {
            mach_symtab_command_t *sym = (mach_symtab_command_t *) base;
            if (sym->stroff) sym->stroff += delta;
            if (sym->symoff) sym->symoff += delta;
        }
        base = base + cmd->cmdsize;
    }
    return 0;
}

static htool_return_t
_parse_sep_firmware_name_version (unsigned char *data, char **name, char **version)
{
    uint32_t name_len = strcspn (data, "-");
    *name = calloc (1, name_len);
    memcpy (*name, data, name_len);

    data += name_len + 1;
    uint32_t vers_len = strcspn (data, "/");
    *version = calloc (1, vers_len);
    memcpy (*version, data, vers_len);
}


htool_return_t
detect_sep_firmware (htool_binary_t *bin)
{
    /**
     *  There are two types of SEP firmware we can look for, the first is the
     *  Secure Enclave's BootROM, which can be identified with the string
     *  "AppleSEPROM", or the SEPs firmware that ships with an IPSW file that
     *  can be identified with the string "Built by legion2".
     */
    unsigned char *data;
    char *needle;

    /* Look for SEP Firmware */
    needle = "Built by legion2";
    data = bh_memmem (bin->data, bin->size, needle, strlen (needle));
    if (data) return HTOOL_RETURN_SUCCESS;

    /* Look for AppleSEPROM */
    needle = "AppleSEPROM";
    data = bh_memmem (bin->data, bin->size, needle, strlen (needle));
    if (data) return HTOOL_RETURN_SUCCESS;

    needle = "private_build..";
    data = bh_memmem (bin->data, bin->size, needle, strlen (needle));
    if (data) return HTOOL_RETURN_SUCCESS;

    /* Unknown Secure Boot firmware */
    needle = "Apple Secure Boot";
    data = bh_memmem (bin->data, bin->size, needle, strlen (needle));
    if (data) return HTOOL_RETURN_VOID;

    return HTOOL_RETURN_FAILURE;
}


#define SWAP_INT(a)         ( ((a) << 24) | \
            (((a) << 8) & 0x00ff0000) | \
            (((a) >> 8) & 0x0000ff00) | \
            ((unsigned int)(a) >> 24) )

/* the 64-bit SEPs are completely fucked */
htool_return_t
parse_sepos_64 (sep_t *sep, uint32_t hdr_offset)
{
    sep_data_hdr_64_t *hdr;

    hdr = (sep_data_hdr_64_t *) (sep->data + hdr_offset);

    /**
     *  The SEP Bootloader on 64-bit is located from the base of the image to the start
     *  of the kernel, found at `hdr->kernel_base_paddr`.
     */
    sep->bootloader_offset = 0;
    sep->bootloader_size = hdr->kernel_base_paddr;
    sep->bootloader_version = bh_memmem (sep->data + sep->bootloader_offset, sep->size, "AppleSEPOS-", 11);

    /**
     * 
     */
    sep->kernel_offset = hdr->kernel_base_paddr;
    sep->kernel_size = _sep_macho_calc_size (sep->data + sep->kernel_offset, sep->size - hdr->kernel_base_paddr);
    sep->kernel_version = sep->bootloader_version;
    sep->kernel = sep->data + sep->kernel_offset;

    /*printf ("n_apps: %d\n", SWAP_INT (hdr->n_apps));
    sepapp_64_t *apps = (sepapp_64_t *) (((uint8_t *) hdr) + sizeof (sep_data_hdr_64_t));
    for (int i = 0; i < hdr->n_apps; i++) {
        printf ("i: %d\n", i);
        //uint32_t sz = _sep_macho_calc_size (sep->data + apps[i].phys_text, sep->size - apps[i].phys_text);
        //char tail[12 + 1];
        //_tail_from_name (tail, apps[i].app_name);

        printf("%-12s phys_text 0x%llx, virt 0x%llx, size_text 0x%llx, phys_data 0x%llx, size_data 0x%llx, entry 0x%llx\n",
                "tail", apps[i].phys_text, apps[i].virt, apps[i].size_text, apps[i].phys_data, apps[i].size_data, apps[i].entry);
    }*/


    return HTOOL_RETURN_FAILURE;
}

htool_return_t
parse_sepos_32 (sep_t *sep)
{
    int index = 0;
    size_t last = 0;
    for (uint32_t i = 0; i < sep->size; i += 4) {
        /**
         *  The _sep_macho_calc_size call can be removed once 32-bit mach-o's are supported
         *  by libhelper
         */        
        size_t sz = _sep_macho_calc_size ((unsigned char *) (sep->data + i), sep->size - i);
        if (sz) {

            /**
             *  The first item we come across should be the bootloader, followed by the kernel
             *  and then the applications.
             */
            if (index == 0) {

                /* Set the offset and size of the bootloader */
                sep->bootloader_offset = i;
                sep->bootloader_size = i - last;
                
                /* Try to find and parse the version string */
                unsigned char *vers = bh_memmem (sep->data + sep->bootloader_offset, sep->bootloader_size, "AppleSEPOS-", 11);
                sep->bootloader_version = strdup (vers);

            } else if (index == 1) {

                /* Can't set the `kernel` property because libhelper doesn't support 32-bit Mach-O's */
                //sep->kernel = macho_32_create (sep->data, sz);
                sep->kernel_version = sep->bootloader_version;
                sep->kernel_offset = i;
                sep->kernel_size = sz;

            } else {

                sep_app_t *app = calloc (1, sizeof (sep_app_t));
                unsigned char *name;
                unsigned char *needle;
                unsigned char *identifier;

                /* Find AppleCredentialManager */
                needle = "AppleCredentialManager-";
                identifier = bh_memmem (sep->data + i, sz, needle, strlen (needle));
                if (identifier) {
                    _parse_sep_firmware_name_version (identifier, &app->name, &app->version);
                    goto add_app_to_list;
                }

                /* Find AppleKeyStore */
                needle = "AppleKeyStore-";
                identifier = bh_memmem (sep->data + i, sz, needle, strlen (needle));
                if (identifier) {
                    _parse_sep_firmware_name_version (identifier, &app->name, &app->version);
                    goto add_app_to_list;
                }

                /* Find Mesa Firmware */
                needle = "Mesa-605.100.11";
                identifier = bh_memmem (sep->data + i, sz, needle, strlen (needle));
                if (identifier) {
                    _parse_sep_firmware_name_version (identifier, &app->name, &app->version);
                    goto add_app_to_list;
                }

                /* Unknown Firmware */
                app->name = "Unknown";
                app->version = "000.0.0";

add_app_to_list:
                /* Set remaining app properties */
                app->offset = i;
                app->size = sz;
                app->data = sep->data + i;
                sep->apps = h_slist_append (sep->apps, app);
                goto next_app;

            }

next_app:
            /* Move the pointer forwards to find next app */
            last = i;
            i += sz - 4;
            index += 1;
        }
    }
    return HTOOL_RETURN_SUCCESS;
}

sep_t *
parse_sep_firmware (htool_binary_t *bin)
{
    sep_t *sep;
    char *needle;
    unsigned char *base;

    /**
     *  Annoyingly, there are a few types of SEP firmware:
     *      - AppleSEPROM
     *      - SEPOS 32-bit
     *      - SEPOS 64-bit
     *      - SEPOS Compressed(?)
     * 
     *  The AppleSEPROM can be determined by searching for that string. The 32-bit and 64-bit   
     *  SEPs are handled in their own ways but both contain application Mach-O's, whereas the
     *  compressed SEPOS I can't quite work out the format yet, so that will have a warning
     *  message.
     */

    /* Start by looking for the "legion2" string, indicating SEPOS */
    needle = "Built by legion2";
    base = bh_memmem (bin->data, bin->size, needle, strlen (needle));

    if (base) {

        sep = calloc (1, sizeof (sep_t));
        printf (ANSI_COLOR_GREEN "[*]" RESET ANSI_COLOR_GREEN " Detected Secure Enclave Operating System (SEPOS)\n" RESET);
        
        /**
         *  If "legion2" was found, we now need to determine whether the firmware is 32-bit,
         *  64-bit or the weird compressed format. The 32-bit files have a header offset of 
         *  0x0, the 64-bit one has a regular offset, and the compressed one with have an
         *  offset greater than 0xf.
         */
        uint64_t hdr_offset = *(uint64_t *) (base + 16);
        if (hdr_offset > 0xf0fff) {
            // handle compressed file error
            sep->type = SEP_FIRMWARE_TYPE_OS_32_COMP;

            printf ( BOLD DARK_WHITE "%sSEPOS Version:       " RESET DARK_GREY "Unknown\n" RESET, "   ");
            printf ( BOLD DARK_WHITE "%sBootloader Offset:   " RESET DARK_GREY "0x0 (0 bytes)\n" RESET, "   ");
            printf ( BOLD DARK_WHITE "%sKernel Offset:       " RESET DARK_GREY "0x0 (0 bytes)\n" RESET, "   ");
            printf ( BOLD DARK_WHITE "%sArchitecture:        " RESET DARK_GREY "32-bit?\n" RESET, "   ");
            printf ( BOLD DARK_WHITE "%sApplications:        " RESET DARK_GREY "0\n" RESET, "   ");

            warningf ("HTool cannot handle the compressed SEPOS Firmware Files: 0x%llx\n", hdr_offset);
            goto complete;
        }
        sep->data = bin->data;
        sep->size = bin->size;

        /* 32-bit SEPOS */
        if (hdr_offset == 0) {
            sep->type = SEP_FIRMWARE_TYPE_OS_32;
            parse_sepos_32 (sep);

            printf ( BOLD DARK_WHITE "%sSEPOS Version:       " RESET DARK_GREY "%s\n" RESET, "   ", sep->bootloader_version);
            printf ( BOLD DARK_WHITE "%sBootloader Offset:   " RESET DARK_GREY "0x%llx (%d bytes)\n" RESET, "   ", sep->bootloader_offset, sep->bootloader_size);
            printf ( BOLD DARK_WHITE "%sKernel Offset:       " RESET DARK_GREY "0x%llx (%d bytes)\n" RESET, "   ", sep->kernel_offset, sep->kernel_size);
            printf ( BOLD DARK_WHITE "%sArchitecture:        " RESET DARK_GREY "32-bit\n" RESET, "   ");
            printf ( BOLD DARK_WHITE "%sApplications:        " RESET DARK_GREY "%d\n" RESET, "   ", h_slist_length (sep->apps));

        } else {
            parse_sepos_64 (sep, hdr_offset);
            sep->type = SEP_FIRMWARE_TYPE_OS_64;

            /**
             *  Although we can't parse the 64-bit SEPOS yet because it's format is unknwon,
             *  we can still determine the version number.
             */

            printf ( BOLD DARK_WHITE "%sSEPOS Version:       " RESET DARK_GREY "%s\n" RESET, "   ", sep->bootloader_version);
            printf ( BOLD DARK_WHITE "%sBootloader Offset:   " RESET DARK_GREY "0x%llx (%d bytes)\n" RESET, "   ", sep->bootloader_offset, sep->bootloader_size);
            printf ( BOLD DARK_WHITE "%sKernel Offset:       " RESET DARK_GREY "0x%llx (%d bytes)\n" RESET, "   ", sep->kernel_offset, sep->kernel_size);
            printf ( BOLD DARK_WHITE "%sArchitecture:        " RESET DARK_GREY "64-bit\n" RESET, "   ");
            printf ( BOLD DARK_WHITE "%sApplications:        " RESET DARK_GREY "%d\n" RESET, "   ", h_slist_length (sep->apps));

            warningf ("HTool cannot handle 64-bit SEPOS Firmware Files: 0x%llx\n", hdr_offset);
        }

    } else {

        /* If "legion2" isn't found, there is a chance this is actually a ROM file */
        needle = "AppleSEPROM-";
        base = bh_memmem (bin->data, bin->size, needle, strlen (needle));

        sep = calloc (1, sizeof (sep_t *));
        if (!base) {

            needle = "private_build..";
            base = bh_memmem (bin->data, bin->size, needle, strlen (needle));
            if (!base) goto complete;

            sep->rom_version = "private_build";
            
            uint32_t start = strcspn (base, "(") + 1;
            uint32_t end = strcspn (base, ")");

            sep->rom_builder = calloc (1, end - start);
            memcpy (sep->rom_builder, base + start, end - start);

        } else {
            sep->rom_version = base;
        }

        sep->data = bin->data;
        sep->size = bin->size;
        sep->type = SEP_FIRMWARE_TYPE_ROM;

        printf (ANSI_COLOR_GREEN "[*]" RESET ANSI_COLOR_GREEN " Detected Secure Enclave BootROM (SEPROM)\n" RESET);
        printf ( BOLD DARK_WHITE "%sSEPROM Version:  " RESET DARK_GREY "%s\n" RESET, "   ", sep->rom_version);
        if (sep->rom_builder)
            printf ( BOLD DARK_WHITE "%sSEPROM Builder:  " RESET DARK_GREY "%s\n" RESET, "   ", sep->rom_builder);
        printf ( BOLD DARK_WHITE "%sSize:            " RESET DARK_GREY "%d bytes\n" RESET, "   ", sep->size);
        printf ( BOLD DARK_WHITE "%sArchitecture:    " RESET DARK_GREY "64-bit?\n" RESET, "   ");
    }

complete:
    return sep;
}