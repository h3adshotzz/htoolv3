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

#include <time.h>

#include "commands/macho.h"

/* return values for select_macho_arch */
#define SELECT_MACHO_ARCH_FAIL             0
#define SELECT_MACHO_ARCH_FAIL_NO_ARCH     1
#define SELECT_MACHO_ARCH_IS_FAT           2
#define SELECT_MACHO_ARCH_IS_MACHO         3

static int
_select_macho_arch (htool_client_t *client, macho_t **macho)
{
    htool_binary_t *bin = client->bin;

    /**
     *  Assuming here that --arch is set, or were dealing with a single Mach-O.
     * 
     *  First we will check the macho_list contains at least one value, then
     *  we will check for the --arch flag. If this is set, we'll load the mach
     *  header, otherwise we will load the header from the first element in the
     *  macho_list.
     */
    if (!h_slist_length (bin->macho_list))
        return SELECT_MACHO_ARCH_FAIL;
    
    macho_t *tmp = calloc (1, sizeof (macho_t));

    /* check for --arch */
    if (client->opts & HTOOL_CLIENT_MACHO_OPT_ARCH) {

        /* load the macho for --arch */
        tmp = htool_binary_select_arch (bin, client->arch);
        if (!tmp) return SELECT_MACHO_ARCH_FAIL_NO_ARCH;

    } else {

        /* load the first macho in the list */
        tmp = (macho_t *) h_slist_nth_data (bin->macho_list, 0);
        if (!tmp) return SELECT_MACHO_ARCH_FAIL;

    }

    /* set the value of macho */
    *macho = tmp;
    return SELECT_MACHO_ARCH_IS_MACHO;
}

static htool_return_t
_check_fat (htool_client_t *client)
{
    return (HTOOL_CLIENT_CHECK_FLAG (client->bin->flags, HTOOL_BINARY_FILETYPE_FAT) && 
        !(client->opts & HTOOL_CLIENT_MACHO_OPT_ARCH)) ? HTOOL_RETURN_SUCCESS : HTOOL_RETURN_FAILURE;
}


///////////////////////////////////////////////////////////////////////

htool_return_t
htool_print_header (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;      /* cleaner than client->bin-><ext> */

    /**
     *  If the file is a FAT archive, but --arch hasn't been set, print the
     *  FAT header.
     */
    if (_check_fat (client)) {

        // print fat header
        printf (BOLD RED "FAT Header:\n" RED BOLD RESET);
        htool_print_fat_header_from_struct (bin->fat_info, 1);

    } else {

        /**
         *  Load the correct Mach-O. Either the only one in the macho_list, or
         *  the one specified by --arch.
         */
        macho_t *macho = calloc (1, sizeof (macho_t));
        int res = _select_macho_arch (client, &macho);

        debugf ("res_: %d\n", res);

        printf (BOLD RED "Mach Header:\n" RED BOLD RESET);
        htool_print_macho_header_from_struct (macho->header);

    }
}

void
htool_print_load_commands (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;

    /**
     *  First, check if the file is a FAT and has --arch unset. If this is the
     *  case, print out an error as we cannot print load commands of a FAT if
     *  --arch is not set.
     */
    if (_check_fat (client)) {
        
        /* print an error as --arch is not set */
        errorf ("htool_print_load_commands: Cannot print Load Commands of a FAT archive. Please run with --arch=\n\n");
        
        /* if the --header option has been used, don't print the header again */
        if (!(client->opts & HTOOL_CLIENT_MACHO_OPT_HEADER)) {
            printf (BOLD RED "FAT Header:\n" RESET);
            htool_print_fat_header_from_struct (bin->fat_info, 1);    
        }
        
        /* there's nothing that can be done now, so exit */
        exit (HTOOL_RETURN_FAILURE);

    } else {

        /**
         *  Load the correct Mach-O. Either the only one in the macho_list, or
         *  the one specified by --arch.
         */
        macho_t *macho = calloc (1, sizeof (macho_t));
        if (_select_macho_arch (client, &macho) == SELECT_MACHO_ARCH_FAIL_NO_ARCH) {
            errorf ("htool_print_load_commands: Could not load architecture from FAT archive: %s\n", client->arch);
            htool_print_fat_header_from_struct (bin->fat_info, 1);

            exit (HTOOL_RETURN_FAILURE);
        }

        /* lists for segment and load commands */
        HSList *lc_list = macho->lcmds;
        HSList *sc_list = macho->scmds;

        /* counter for load commands and dylibs */
        int lc_count = 0, dylib_count = 0;

        printf (BOLD RED "\nLoad Command:\n" RESET);

        /* printing segment commands */
        for (int i = 0; i < (int) h_slist_length (sc_list); i++) {

            /**
             *  Segment commands can be either 32- or 64-bit, even on 64-bit Mach-O's. The
             *  mach_segment_info_t API has an `arch` field, so we can check with architecture
             *  the underlying segment command is.
             */
            mach_segment_info_t *info = (mach_segment_info_t *) h_slist_nth_data (sc_list, i);
            
            if (info->arch == LIBHELPER_ARCH_64) {

                mach_segment_command_64_t *seg64 = (mach_segment_command_64_t *) info->segcmd;

                /* load command info */
                printf (BOLD DARK_GREY "LC %02d:  Mem: 0x%08llx → 0x%08llx\n" DARK_GREY BOLD RESET,
                        i, seg64->vmaddr, (seg64->vmaddr + seg64->vmsize));
                printf (YELLOW "  LC_SEGMENT_64:" YELLOW RESET);

                /* segment memory protection */
                printf (DARK_GREY " %s/%s\t" DARK_GREY RESET "%s\n" RESET,
                        mach_segment_read_vm_protection (seg64->initprot),
                        mach_segment_read_vm_protection (seg64->maxprot),
                        seg64->segname);

                /* section commands */
                int nsect = (int) h_slist_length (info->sections);
                if (nsect) {

                    /* section table header, print each section */
                    printf (BOLD DARK_YELLOW "  %-24s%-10s%-10s\n", "Name", "Size", "Offset" DARK_YELLOW BOLD RESET);

                    for (int i = 0; i < nsect; i++) {
                        mach_section_64_t *sect = (mach_section_64_t *) h_slist_nth_data (info->sections, i);
                        printf (BOLD DARK_WHITE "  %s%-23s" RESET DARK_GREY "%-10llu0x%08llx → 0x%08llx\n" RESET,
                                ".", sect->sectname, sect->size, sect->addr, sect->addr + sect->size);
                    }
                } else {
                    printf (BLUE "  No Data\n" BLUE RESET);
                }

                lc_count += 1;

            } else {
                warningf ("32-bit segment commands not implemented\n");
            }
        }

        /* print the other load commands */
        for (int i = 0; i < (int) h_slist_length (lc_list); i++) {

            mach_load_command_info_t *info = (mach_load_command_info_t *) h_slist_nth_data (lc_list, i);
            mach_load_command_t *lc = info->lc;

            /* set the formatting for "LC XX" depending on lc_count */
            printf (BOLD DARK_GREY "LC ");
            if (lc_count < 100) printf ("%02d\n" RESET, lc_count);
            else printf ("%03d\n" RESET, lc_count);

            /* load the whole command into a new buffer */
            void *rawlc = (void *) macho_load_bytes (macho, lc->cmdsize, info->offset);
            
            printf ("lc->cmd: %d\n", lc->cmd);

            /**
             *  Go through every type of Load Command. LCs that have been deprecated will
             *  print a warningf, as it makes it too difficult to test if their output is
             *  correct.
             *
             *  These *should* be in the order they appear in loader.h. Commands that are
             *  similar and use a similar print function will be grouped though.
             */
            switch (lc->cmd) {

                /* FVM Load Commands */
                case LC_IDFVMLIB:
                case LC_LOADFVMLIB:
                    warningf ("Load Command (%s) is deprecated in XNU and is not supported in HTool.\n",
                              mach_header_get_file_type_string (lc));
                    break;

                /* Dynamic Library Load Commands */
                case LC_ID_DYLIB:
                case LC_LOAD_DYLIB:
                case LC_REEXPORT_DYLIB:
                case LC_LOAD_WEAK_DYLIB:
                    htool_print_dylib_command (macho->dylibs, dylib_count);
                    break;

                /* Sub Framework Load Commands */
                case LC_SUB_CLIENT:
                case LC_SUB_LIBRARY:
                case LC_SUB_UMBRELLA:
                case LC_SUB_FRAMEWORK:
                    htool_print_sub_framework_command (macho, info, rawlc);
                    break;

                /* Prebound Dylib Command */
                case LC_PREBOUND_DYLIB:
                    htool_print_prebound_dylib_command (macho, info);
                    break;

                /* Dynamic Linker Command */
                case LC_ID_DYLINKER:
                case LC_LOAD_DYLINKER:
                case LC_DYLD_ENVIRONMENT:
                    htool_print_dylinker_command (macho, info);
                    break;

                /* Linkedit Data Command */
                case LC_CODE_SIGNATURE:
                case LC_SEGMENT_SPLIT_INFO:
                case LC_FUNCTION_STARTS:
                case LC_DATA_IN_CODE:
                case LC_DYLIB_CODE_SIGN_DRS:
                case LC_LINKER_OPTIMIZATION_HINT:
                case LC_DYLD_EXPORTS_TRIE:
                case LC_DYLD_CHAINED_FIXUPS:
                    htool_print_linkedit_data_command (rawlc);
                    break;

                /* Dynamic Linker Info */
                case LC_DYLD_INFO:
                case LC_DYLD_INFO_ONLY:
                    htool_print_dylid_info_command (rawlc);
                    break;

                default:
                    warningf ("Load Command (%s) not implemented.\n", mach_load_command_get_name (lc));
                    break;
            }

            lc_count += 1;

        }

    }
}

//===----------------------------------------------------------------------===//
//                       Formatted Print Functions
//===----------------------------------------------------------------------===//

void
htool_print_macho_header_from_struct (mach_header_t *hdr)
{
    /* get macho header type string */
    char *magic_text;
    if (hdr->magic == MACH_CIGAM_64 || hdr->magic == MACH_MAGIC_64) magic_text = "Mach-O 64-bit";
    else magic_text = "Mach-O 32-bit";

    /* output header details */
    printf (BOLD DARK_WHITE "    Magic: " RESET DARK_GREY "0x%08x (%s)\n" RESET, hdr->magic, magic_text);
    printf (BOLD DARK_WHITE "     Type: " RESET DARK_GREY "%s\n" RESET, mach_header_get_file_type_string (hdr->filetype));
    printf (BOLD DARK_WHITE "      CPU: " RESET DARK_GREY "%s (%s)\n" RESET,
            mach_header_get_cpu_string (hdr->cputype, hdr->cpusubtype),
            mach_header_get_cpu_descriptor (hdr->cputype, hdr->cpusubtype));
    printf (BOLD DARK_WHITE "   ↳ Type: " RESET DARK_GREY "0x%08x, Subtype: 0x%08x\n" RESET, hdr->cputype, hdr->cpusubtype);
    printf (BOLD DARK_WHITE "  LoadCmd: " RESET DARK_GREY "%d\n" RESET, hdr->ncmds);
    printf (BOLD DARK_WHITE "  LC Size: " RESET DARK_GREY "%d Bytes\n" RESET, hdr->sizeofcmds);
}

void
htool_print_fat_header_from_struct (fat_info_t *info, int expand)
{
    printf (BOLD DARK_WHITE "  Magic: " RESET DARK_GREY "0x%08x (Universal Binary)\n" RESET, info->header->magic);
    printf (BOLD DARK_WHITE "  Archs: " RESET DARK_GREY "%d\n\n" RESET, info->header->nfat_arch);

    printf (BOLD DARK_YELLOW "%-20s%-10s%-10s%-10s\n", "Name", "Size", "Align", "Offset" RESET);
    for (int i = 0; i < h_slist_length (info->archs); i++) {
        fat_arch_t *arch = (fat_arch_t *) h_slist_nth_data (info->archs, i);

        char *name = mach_header_get_cpu_string (arch->cputype, arch->cpusubtype);
        printf (BOLD WHITE "  %-18s" RESET DARK_GREY "%-10d%-10d0x%08x → 0x%08x\n" RESET,
                name, arch->size, arch->align, arch->offset, arch->offset + arch->size);
    }
}

void
htool_print_dylib_command (HSList *dylibs, int dylib_count)
{
    mach_dylib_command_info_t *info = (mach_dylib_command_info_t *) h_slist_nth_data (dylibs, dylib_count);
    mach_dylib_command_t *lc = info->dylib;
    time_t t = lc->dylib.timestamp;

    /* print the info */
    printf (YELLOW "  %s\n" RESET, mach_load_command_get_name ((mach_load_command_t *) lc));

    printf (BOLD DARK_WHITE "                   Path: " RESET DARK_GREY "%s\n" RESET,
            info->name);
    printf (BOLD DARK_WHITE "              Timestamp: " RESET DARK_GREY "%s" RESET,
            ctime (&t));
    printf (BOLD DARK_WHITE "        Current Version: " RESET DARK_GREY "%u.%u.%u\n" RESET,
            lc->dylib.current_version >> 16,
            (lc->dylib.current_version >> 8) & 0xff,
            lc->dylib.current_version & 0xff);
    printf (BOLD DARK_WHITE "  Compatibility Version: " RESET DARK_GREY "%u.%u.%u\n" RESET,
            lc->dylib.compatibility_version >> 16,
            (lc->dylib.compatibility_version >> 8) & 0xff,
            lc->dylib.compatibility_version & 0xff);
}

void
htool_print_sub_framework_command (macho_t *macho, mach_load_command_info_t *info, void *lc_raw)
{
    /**
     *  This covers LC_SUB_CLIENT, LC_SUB_LIBRARY, LC_SUB_UMBRELLA and
     *  LC_SUB_FRAMEWORK commands. They all have the same structure, so
     *  we can just use one struct for them all.
     */
    mach_load_command_t *lc = (mach_load_command_t *) lc_raw;
    union lc_str *str = (union lc_str *) lc_raw + 8;

    printf (YELLOW "  %-20s" RESET, mach_header_get_file_type_string (lc->cmd));
    printf (BOLD DARK_WHITE "  Name: " RESET DARK_GREY "%s" RESET,
            mach_load_command_load_string (macho, lc->cmdsize, sizeof (mach_sub_framework_command_t), info->offset, str->offset));
}

void
htool_print_prebound_dylib_command (macho_t *macho, mach_load_command_info_t *info)
{
    mach_prebound_dylib_command_t *lc = info->lc;

    printf (YELLOW "  %s\n" RESET, mach_load_command_get_name (lc));

    printf (BOLD DARK_WHITE "            Path: " RESET DARK_GREY "%s\n" RESET,
            mach_load_command_load_string (macho, lc->cmdsize, sizeof (mach_prebound_dylib_command_t), info->offset, lc->name.offset));
    printf (BOLD DARK_WHITE "         Modules: " RESET DARK_GREY "%d\n" RESET,
            lc->nmodules);
    printf (BOLD DARK_WHITE "  Linked Modules: " RESET DARK_GREY "%s\n" RESET,
            mach_load_command_load_string (macho, lc->cmdsize, sizeof (mach_prebound_dylib_command_t), info->offset, lc->linked_modules.offset));
}

void
htool_print_dylinker_command (macho_t *macho, mach_load_command_info_t *info)
{
    mach_dylinker_command_t *lc = (mach_dylinker_command_t *) info->lc;

    printf (YELLOW "  %-20s" RESET BOLD DARK_WHITE "%-20s" RESET DARK_GREY "%s\n" RESET,
            mach_load_command_get_name (lc),
            "Dylinker Path: ",
            mach_load_command_load_string (macho, lc->cmdsize, sizeof (mach_dylinker_command_t), info->offset, lc->name.offset));
}

void
htool_print_linkedit_data_command (void *cmd)
{
    mach_linkedit_data_command_t *lc = (mach_linkedit_data_command_t *) cmd;

    printf (YELLOW "  %-27s" RESET, mach_load_command_get_name ((mach_load_command_t *) cmd));
    printf (BOLD DARK_WHITE "%-20s" RESET DARK_GREY "0x%08x (%d bytes)\n",
            "Offset:", lc->dataoff, lc->datasize);
}

void
htool_print_dylid_info_command (void *cmd)
{
    mach_dyld_info_command_t *lc = (mach_dyld_info_command_t *) cmd;

    printf (YELLOW "%s\n" RESET, mach_load_command_get_name ((mach_load_command_t *) cmd));
    printf (BOLD DARK_YELLOW "  %-12s%-8s%s\n", "Info", "Size", "Offset" DARK_YELLOW BOLD RESET);

    // rebase
    printf (BOLD DARK_WHITE "  %-12s" RESET, "Rebase");
    printf (DARK_GREY "%-8d0x%08x → 0x%08x\n" RESET, lc->rebase_size, lc->rebase_off, lc->rebase_off + lc->rebase_size);

    // bind
    printf (BOLD DARK_WHITE "  %-12s" RESET, "Bind");
    printf (DARK_GREY "%-8d0x%08x → 0x%08x\n" RESET, lc->bind_size, lc->bind_off, lc->bind_off + lc->bind_size);

    // weak bind
    printf (BOLD DARK_WHITE "  %-12s" RESET, "Weak Bind");
    printf (DARK_GREY "%-8d0x%08x → 0x%08x\n" RESET, lc->weak_bind_size, lc->weak_bind_off, lc->weak_bind_off + lc->weak_bind_size);

    // lazy bind
    printf (BOLD DARK_WHITE "  %-12s" RESET, "Lazy Bind");
    printf (DARK_GREY "%-8d0x%08x → 0x%08x\n" RESET, lc->lazy_bind_size, lc->lazy_bind_off, lc->lazy_bind_off + lc->lazy_bind_size);

    // export
    printf (BOLD DARK_WHITE "  %-12s" RESET, "Export");
    printf (DARK_GREY "%-8d0x%08x → 0x%08x\n" RESET, lc->export_size, lc->export_off, lc->export_off + lc->export_size);
}
