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

/**
 * NOTE:    This file handles the majority of the functionality of the `macho` 
 *          command. However, handling of symbols is found in nm.c as much
 *          functionality isn't implemented in Libhelper.
*/

#include <time.h>

#include "htool-error.h"

/* libhelper doesn't implement support for arm thread state */
#if defined(__APPLE__) && defined(__MACH__)
#   include <mach/arm/thread_status.h>
#else
#   include <libhelper-macho.h>
#endif

#include "commands/macho.h"

#define DEBUG 1

int
htool_macho_select_arch (htool_client_t *client, macho_t **macho)
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
    if (HTOOL_CLIENT_CHECK_FLAG(client->opts, HTOOL_CLIENT_MACHO_OPT_ARCH) && 
        HTOOL_CLIENT_CHECK_FLAG (client->bin->flags, HTOOL_BINARY_FILETYPE_FAT)) {

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

htool_return_t
htool_macho_check_fat (htool_client_t *client)
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
    if (htool_macho_check_fat (client)) {

        // print fat header
        printf (BOLD RED "FAT Header:\n" RED BOLD RESET);
        htool_print_fat_header_from_struct (bin->fat_info, 1);

        ci_logf("Printed FAT Header\n");
    } else {

        /**
         *  Load the correct Mach-O. Either the only one in the macho_list, or
         *  the one specified by --arch.
         */
        macho_t *macho = calloc (1, sizeof (macho_t));
        int res = htool_macho_select_arch (client, &macho);

        debugf ("res_: %d\n", res);

        printf (BOLD RED "Mach Header:\n" RED BOLD RESET);
        htool_print_macho_header_from_struct (macho->header);
        
        ci_logf("Printed Mach-O Header\n");
    }
}

htool_return_t
htool_print_load_commands (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;

    /**
     *  First, check if the file is a FAT and has --arch unset. If this is the
     *  case, print out an error as we cannot print load commands of a FAT if
     *  --arch is not set.
     */
    if (htool_macho_check_fat (client)) {
        
        /* print an error as --arch is not set */
        htool_error_throw (HTOOL_ERROR_ARCH, "Cannot print Load Commands of a FAT archive");
        
        /* if the --header option has been used, don't print the header again */
        if (!(client->opts & HTOOL_CLIENT_MACHO_OPT_HEADER)) {
            printf (BOLD RED "FAT Header:\n" RESET);
            htool_print_fat_header_from_struct (bin->fat_info, 1);    
        }
        
        /* there's nothing that can be done now, so exit */
        return HTOOL_RETURN_EXIT;

    } else {

        /**
         *  Load the correct Mach-O. Either the only one in the macho_list, or
         *  the one specified by --arch.
         */
        macho_t *macho = calloc (1, sizeof (macho_t));
        if (htool_macho_select_arch (client, &macho) == SELECT_MACHO_ARCH_FAIL_NO_ARCH) {
            htool_error_throw (HTOOL_ERROR_FILETYPE, "Could not load architecture from FAT archive: %s\n", client->arch);
            htool_print_fat_header_from_struct (bin->fat_info, 1);

            return HTOOL_RETURN_EXIT;
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
                printf (BOLD DARK_GREY "LC %02d:  Mem: 0x%08llx → 0x%08llx" DARK_GREY BOLD RESET,
                        i, seg64->vmaddr, (seg64->vmaddr + seg64->vmsize));
#if DEBUG
                printf ("offset: 0x%llx - 0x%llx", seg64->fileoff, (seg64->fileoff + seg64->filesize));
#endif
                printf ("\n");
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
                    printf (BOLD DARK_YELLOW "  %-24s%-10s%-10s\n", "Name", "Size", "Range" DARK_YELLOW BOLD RESET);

                    for (int i = 0; i < nsect; i++) {
                        mach_section_64_t *sect = (mach_section_64_t *) h_slist_nth_data (info->sections, i);
                        printf (BOLD DARK_WHITE "  %s%-23s" RESET DARK_GREY "%-10llu0x%08llx → 0x%08llx\n" RESET,
                                ".", sect->sectname, sect->size, sect->addr, sect->addr + sect->size);
                        //debugf ("offset: 0x%08x\n", sect->offset);
                    }
                } else {
                    printf (BLUE "  No Data\n" BLUE RESET);
                }

                lc_count += 1;

            } else {
                warningf ("32-bit segment commands not implemented\n");
            }
        }
        ci_logf("Printed Mach-O Segment Commands\n");

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

                /* Thread Command */
                case LC_THREAD:
                case LC_UNIXTHREAD:
                    htool_print_thread_state_command (rawlc);
                    break;

                /* Symbol Table Command */
                case LC_SYMTAB:
                    htool_print_symtab_command (rawlc);
                    break;

                /* Dynamic Symbol Table Command */
                case LC_DYSYMTAB:
                    htool_print_dysymtab_command (rawlc);
                    break;

                /* UUID Command */
                case LC_UUID:
                    htool_print_uuid_command (rawlc);
                    break;

                /* RPATH Command */
                case LC_RPATH:
                    htool_print_rpath_command (macho, info, rawlc);
                    break;

                /* Source Version Command */
                case LC_SOURCE_VERSION:
                    htool_print_source_version_command (rawlc);
                    break;

                /* Build Version Command */
                case LC_BUILD_VERSION:
                    htool_print_build_version_command (macho, info->offset, rawlc);
                    break;

                /* Entry Point Command */
                case LC_MAIN:
                    htool_print_entry_point_command (rawlc);
                    break;

                /* Fileset Entry Commnad */
                case LC_FILESET_ENTRY:
                    htool_print_fileset_entry_command (macho, info->offset, rawlc);
                    break;

                default:
                    warningf ("Load Command (%s) not implemented.\n", mach_load_command_get_name (lc));
                    break;
            }

            lc_count += 1;

        }

    }
    ci_logf("Printed Mach-O Load Commands\n");
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_print_shared_libraries (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;

    /**
     *  First, check if the file is a FAT and has --arch unset. If this is the
     *  case, print out an error as we cannot print load commands of a FAT if
     *  --arch is not set.
     */
    if (htool_macho_check_fat (client)) {
        
        /* print an error as --arch is not set */
        htool_error_throw (HTOOL_ERROR_FILETYPE, "Cannot print Shared libraries of a FAT archive");
        
        /* if the --header option has been used, don't print the header again */
        if (!(client->opts & HTOOL_CLIENT_MACHO_OPT_HEADER)) {
            printf (BOLD RED "FAT Header:\n" RESET);
            htool_print_fat_header_from_struct (bin->fat_info, 1);    
        }
        
        /* there's nothing that can be done now, so exit */
        return HTOOL_RETURN_EXIT;

    } else {

        /**
         *  Load the correct Mach-O. Either the only one in the macho_list, or
         *  the one specified by --arch.
         */
        macho_t *macho = calloc (1, sizeof (macho_t));
        if (htool_macho_select_arch (client, &macho) == SELECT_MACHO_ARCH_FAIL_NO_ARCH) {
            htool_error_throw (HTOOL_ERROR_FILETYPE, "Could not load architecture from FAT archive: %s\n", client->arch);
            htool_print_fat_header_from_struct (bin->fat_info, 1);

            return HTOOL_RETURN_EXIT;
        }

        printf (RED BOLD "Dynamically-linked Libraries:\n" RESET);
        printf (BOLD DARK_YELLOW "  %-35s%-20s%-10s\n", "Library", "Compat. Vers", "Curr. Vers" DARK_YELLOW BOLD RESET);

        HSList *dylibs = macho->dylibs;

        for (int i = 0; i < h_slist_length (dylibs); i++) {
            mach_dylib_command_info_t *info = (mach_dylib_command_info_t *) h_slist_nth_data (dylibs, i);
            mach_dylib_command_t *dylib = info->dylib;

            printf (BOLD DARK_WHITE "  %-35s" BOLD DARK_GREY "%-20s%-10s\n" RESET,
                info->name,
                mach_load_command_dylib_format_version (dylib->dylib.compatibility_version),
                mach_load_command_dylib_format_version (dylib->dylib.current_version));
        }
    }
    return HTOOL_RETURN_SUCCESS;
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

/*

SOME CODE FOR HANDLING BYTE SWAPS IF THE MACHINE IS A DIFFERENT
ENDIAN TO THE TARGET FILE

typedef struct {
    uint32_t cmd;
    uint32_t cmdsize;

    uint32_t flavour;
    uint32_t count;

    arm_thread_state64_t state;
} thread_test;

enum byte_sex {
    UNKNOWN_BYTE_SEX,
    BIG_ENDIAN_BYTE_SEX,
    LITTLE_ENDIAN_BYTE_SEX
};

static long long
SWAP_LONG_LONG (long long ll)
{
	union {
	    char c[8];
	    long long ll;
	} in, out;
	in.ll = ll;
	out.c[0] = in.c[7];
	out.c[1] = in.c[6];
	out.c[2] = in.c[5];
	out.c[3] = in.c[4];
	out.c[4] = in.c[3];
	out.c[5] = in.c[2];
	out.c[6] = in.c[1];
	out.c[7] = in.c[0];

    printf ("in: 0x%016llx\nout: 0x%016llx\n", in.ll, out.ll);
	return(out.ll);
}

#define SWAP_INT(a)  ( ((a) << 24) | \
		      (((a) << 8) & 0x00ff0000) | \
		      (((a) >> 8) & 0x0000ff00) | \
	 ((unsigned int)(a) >> 24) )

static void
_swap_arm_thread_state64_t (arm_thread_state64_t *state)
{
    for (int i = 0; i < 29; i++)
        state->__x[i] = SWAP_LONG_LONG (state->__x[i]);

    state->__fp = SWAP_LONG_LONG (state->__fp);
    state->__lr = SWAP_LONG_LONG (state->__lr);
    state->__sp = SWAP_LONG_LONG (state->__sp);
    //state->__pc = SWAP_LONG_LONG (state->__pc);
    state->__cpsr = SWAP_INT (state->__cpsr);
}*/

void
htool_print_thread_state_command (void *lc_raw)
{
    mach_thread_state_command_t *thread_command = (mach_thread_state_command_t *) lc_raw;

    printf (YELLOW "  %s\n" RESET, mach_load_command_get_name ((mach_load_command_t *) thread_command));

    printf (BOLD DARK_WHITE "   Flavour: " RESET);
    if (thread_command->flavour == ARM_THREAD_STATE64) printf (DARK_GREY "ARM_THREAD_STATE64\n" RESET);
    else if (thread_command->flavour == ARM_THREAD_STATE32) printf (DARK_GREY "ARM_THREAD_STATE32\n" RESET);
    else printf (DARK_GREY "UNKNOWN_THREAD_STATE\n" RESET);

    printf (BOLD DARK_WHITE "     Count: " RESET);
    if (thread_command->count == ARM_THREAD_STATE64_COUNT) printf (DARK_GREY "ARM_THREAD_STATE64_COUNT\n" RESET);
    else if (thread_command->count == ARM_THREAD_STATE32_COUNT) printf (DARK_GREY "ARM_THREAD_STATE32_COUNT\n" RESET);
    else printf (DARK_GREY "UNKNOWN_THREAD_STATE_COUNT\n" RESET);

    /* 64-bit thread state command */
    if (thread_command->flavour == ARM_THREAD_STATE64) {

        arm_thread_state64_t *cpu = (arm_thread_state64_t *) (lc_raw + sizeof (mach_thread_state_command_t));

        printf (
            "\t" BOLD DARK_WHITE " x0 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE " x1 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE " x2 " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE " x3 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE " x4 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE " x5 " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE " x6 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE " x7 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE " x8 " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE " x9 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x10 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x11 " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE "x12 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x13 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x14 " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE "x15 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x16 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x17 " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE "x18 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x19 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x20 " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE "x21 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x22 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x23 " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE "x24 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x25 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x26 " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE "x27 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE "x28 " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE " fp " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE " lr " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE " sp " RESET DARK_GREY "0x%016llx"
            "\t" BOLD DARK_WHITE " pc " RESET DARK_GREY "0x%016llx\n"

            "\t" BOLD DARK_WHITE "cpsr " RESET DARK_GREY "0x%08x\n",

            cpu->__x[0], cpu->__x[1], cpu->__x[2], cpu->__x[3],
            cpu->__x[4], cpu->__x[5], cpu->__x[6], cpu->__x[7],
            cpu->__x[8], cpu->__x[9], cpu->__x[10], cpu->__x[11],
            cpu->__x[12], cpu->__x[13], cpu->__x[14], cpu->__x[15],
            cpu->__x[16], cpu->__x[17], cpu->__x[18], cpu->__x[19],
            cpu->__x[20], cpu->__x[21], cpu->__x[22], cpu->__x[23],
            cpu->__x[24], cpu->__x[25], cpu->__x[26], cpu->__x[27],
            cpu->__x[28], cpu->__fp, cpu->__lr, cpu->__sp, cpu->__pc,
            cpu->__cpsr);

    /* 32-bit thread state command */
    } else if (thread_command->flavour == ARM_THREAD_STATE32) {
        warningf ("ARM_THREAD_STATE32 is not supported yet\n");

    /* Unknown thread state command */
    } else {
        htool_error_throw (HTOOL_ERROR_GENERAL, "Unknown thread state flavour: 0x%08x", thread_command->flavour);
    }
}

void
htool_print_symtab_command (void *lc_raw)
{
    mach_symtab_command_t *lc = (mach_symtab_command_t *) lc_raw;

    /* check no of symbols */
    if (!lc->nsyms) {
        printf (BLUE "  No Symbols\n" RESET);
        return;
    }

    printf (YELLOW "  %s" RESET DARK_GREY "  %d symbols\n" RESET,
            "LC_SYMTAB:", lc->nsyms);

    /* symbol table offset, string table offset, string table size. */
    printf (BOLD DARK_WHITE "  Symbol table offset:" RESET DARK_GREY "\t0x%08x\n",
            lc->symoff);
    printf (BOLD DARK_WHITE "  String table offset:" RESET DARK_GREY "\t0x%08x\n",
            lc->stroff);
    printf (BOLD DARK_WHITE "    String table size:" RESET DARK_GREY "\t%d Bytes\n",
            lc->strsize);
}

void
htool_print_dysymtab_command (void *lc_raw)
{
    mach_dysymtab_command_t *lc = (mach_dysymtab_command_t *) lc_raw;

    printf (YELLOW "  %s:\n" YELLOW, mach_load_command_get_name ((mach_load_command_t *) lc_raw));

    // local symbols
    printf (BOLD DARK_WHITE "                Local Symbols: " RESET);
    if (lc->nlocalsym) printf (DARK_GREY "%d at %d\n" RESET, lc->nlocalsym, lc->ilocalsym);
    else printf (BLUE "None\n" RESET);

    // external symbols
    printf (BOLD DARK_WHITE "             External Symbols: " RESET);
    if (lc->nextdefsym) printf (DARK_GREY "%d at %d\n" RESET, lc->nextdefsym, lc->iextdefsym);
    else printf (BLUE "None\n" RESET);

    // undefined symbols
    printf (BOLD DARK_WHITE "            Undefined Symbols: " RESET);
    if (lc->nundefsym) printf (DARK_GREY "%d at %d\n" RESET, lc->nundefsym, lc->iundefsym);
    else printf (BLUE "None\n" RESET);

    // toc
    printf (BOLD DARK_WHITE "                          TOC: " RESET);
    if (lc->ntoc) printf (DARK_GREY "%d entries at 0x%08x\n" RESET, lc->ntoc, lc->tocoff);
    else printf (RED "No\n" RESET);

    // modtab
    printf (BOLD DARK_WHITE "                       Modtab: " RESET);
    if (lc->nmodtab) printf (DARK_GREY "%d entries at 0x%08x\n" RESET, lc->nmodtab, lc->modtaboff);
    else printf (RED "No\n" RESET);

    // indirect symtab entries
    printf (BOLD DARK_WHITE "      Indirect symtab entries: " RESET);
    if (lc->nindirectsyms) printf (DARK_GREY "%d at 0x%08x\n" RESET, lc->nindirectsyms, lc->indirectsymoff);
    else printf (RED "No\n" RESET);

    // external relocation entries
    printf (BOLD DARK_WHITE "  External Relocation Entries: " RESET);
    if (lc->nextrel) printf (DARK_GREY "%d at 0x%08x\n" RESET, lc->nextrel, lc->extreloff);
    else printf (BLUE "None\n" RESET);

    // local relocation entries
    printf (BOLD DARK_WHITE "     Local Relocation Entries: " RESET);
    if (lc->nlocrel) printf (DARK_GREY "%d at 0x%08x\n" RESET, lc->nlocrel, lc->locreloff);
    else printf (BLUE "None\n" RESET);
}

void
htool_print_uuid_command (void *lc_raw)
{
    mach_uuid_command_t *lc = (mach_uuid_command_t *) lc_raw;

    printf (YELLOW "  %-20s" RESET BOLD DARK_WHITE "%-20s" RESET DARK_GREY "%-10s\n" RESET,
            "LC_UUID:", "UUID:", mach_load_command_uuid_parse_string (lc));
}

void
htool_print_rpath_command (macho_t *macho, mach_load_command_info_t *info, void *lc_raw)
{
    mach_rpath_command_t *lc = (mach_rpath_command_t *) info->lc;

    printf (YELLOW "  %-20s" RESET BOLD DARK_WHITE "%-20s" RESET DARK_GREY "%s\n" RESET,
            mach_load_command_get_name (lc),
            "Path: ",
            mach_load_command_load_string (macho, lc->cmdsize, sizeof (mach_rpath_command_t), info->offset, lc->path.offset));
}

void
htool_print_source_version_command (void *lc_raw)
{
    mach_source_version_command_t *lc = (mach_source_version_command_t *) lc_raw;

    printf (YELLOW "  %-20s" RESET BOLD DARK_WHITE "%-20s" RESET DARK_GREY "%-26s\n" RESET,
            "LC_SOURCE_VERSION:", "Source Version:", mach_load_command_get_source_version_string (lc));
}

void
htool_print_build_version_command (macho_t *macho, uint32_t offset, void *lc_raw)
{
    mach_build_version_command_t *lc = (mach_build_version_command_t *) lc_raw;
    mach_build_version_info_t *info = mach_load_command_build_version_info (lc, offset, macho);

    printf (YELLOW "  %-20s" RESET BOLD DARK_WHITE "%-20s" RESET DARK_GREY "%s %s, %s %s, %s %s\n" RESET,
            "LC_BUILD_VERSION:", "Build Version:",
            "Platform:", info->platform,
            "MinOS:", info->minos,
            "SDK:", info->sdk);

    /* tool list */
    if (info->ntools) {
        printf (BOLD DARK_YELLOW "  %-10s%-10s\n", "Tool", "Version" RESET);
        for (int i = 0; i < (int) h_slist_length (info->tools); i++) {
            build_tool_info_t *b = (build_tool_info_t *) h_slist_nth_data (info->tools, i);

            if (!b) {
                printf (BLUE "Invalid Tool\n" RESET);
            } else {
                printf (BOLD DARK_WHITE "  %-10s" RESET DARK_GREY "%d.%d.%d\n" RESET,
                    b->tool,
                    b->version >> 16, (b->version >> 8) & 0xf, b->version & 0xf);
            }
        }
    }
}

void
htool_print_entry_point_command (void *lc_raw)
{
    mach_entry_point_command_t *lc = (mach_entry_point_command_t *) lc_raw;
    printf (YELLOW "  %-20s" RESET BOLD DARK_WHITE "%-20s" RESET DARK_GREY "0x%08llx (Stack: %llu bytes)\n" RESET,
            "LC_MAIN:", "Entry Offset:", lc->entryoff, lc->stacksize);
}

void
htool_print_fileset_entry_command (macho_t *macho, uint32_t offset, void *lc_raw)
{
    mach_fileset_entry_command_t *fs = (mach_fileset_entry_command_t *) lc_raw;
    char *entry_name = mach_load_command_load_string (macho, fs->cmdsize, 
            sizeof (mach_fileset_entry_command_t), offset, fs->entry_id.offset);

    printf (YELLOW "  %s:\n" YELLOW, mach_load_command_get_name ((mach_load_command_t *) lc_raw));

    printf (BOLD DARK_WHITE "\tEntry Name: " RESET DARK_GREY "%s\n", entry_name);
    printf (BOLD DARK_WHITE "\t    vmaddr: " RESET DARK_GREY "0x%llx\n", fs->vmaddr);
    printf (BOLD DARK_WHITE "\t   fileoff: " RESET DARK_GREY "0x%llx\n", fs->fileoff);
    printf (BOLD DARK_WHITE "\t  reserved: " RESET DARK_GREY "0x%llx\n", fs->reserved);
}