//===----------------------------------------------------------------------===//
//
//                         === The HTool Project ===
//
//  This  document  is the property of "Is This On?" It is considered to be
//  confidential and proprietary and may not be, in any form, reproduced or
//  transmitted, in whole or in part, without express permission of Is This
//  On?.
//
//  Copyright (C) 2023, Harry Moulton - Is This On? Holdings Ltd
//
//  Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

#include "htool.h"

#include <assert.h>
#include <libarch.h>

#include <arm64/arm64-common.h>
#include <arm64/arm64-conditions.h>
#include <arm64/arm64-instructions.h>
#include <arm64/arm64-prefetch-ops.h>
#include <arm64/arm64-pstate.h>
#include <arm64/arm64-registers.h>
#include <arm64/arm64-tlbi-ops.h>
#include <arm64/arm64-translation.h>
#include <arm64/arm64-vector-specifiers.h>
#include <arm64/arm64-index-extend.h>

#include <instruction.h>
#include <register.h>
#include <utils.h>

#include "disassembler/parser.h"
#include "commands/disassembler.h"
#include "commands/macho.h"
#include "commands/macho.h"

#include "hashmap.h"

HTOOL_PRIVATE
int
hashmap_compare (const void *a, const void *b, const void *udata)
{
    const inline_symbol_t *aa = a;
    const inline_symbol_t *bb = b;
    return aa->virt_addr == bb->virt_addr;
}

HTOOL_PRIVATE
uint64_t
hashmap_hash (const void *item)
{
    const inline_symbol_t *sym = item;
    uint8_t bytes[8];
    *(uint64_t *) bytes = sym->virt_addr;
    return SIP64 (bytes, sizeof (bytes), 0, 0);
}

///////////////////////////////////////////////////////////////////////////////

HTOOL_PRIVATE
uint64_t
find_offset_for_virtual_address (macho_t *macho, uint64_t vmaddr)
{
    for (int i = 0; i < h_slist_length (macho->scmds); i++) {
        mach_segment_info_t *info = (mach_segment_info_t *) h_slist_nth_data (macho->scmds, i);
        mach_segment_command_64_t *seg = info->segcmd;

        if (vmaddr >= seg->vmaddr && vmaddr < (seg->vmaddr + seg->vmsize))
            return seg->fileoff + (vmaddr - seg->vmaddr);
    }
    return 0;
}

HTOOL_PRIVATE
uint64_t
find_macho_entry_point_virtual_address (macho_t *macho)
{
    mach_segment_info_t *pzinfo = (mach_segment_info_t *) h_slist_nth_data (macho->scmds, 0);
    mach_segment_command_64_t *__pagezero = pzinfo->segcmd;
    if (strcmp (__pagezero->segname, "__PAGEZERO")) return 0;

    mach_load_command_info_t *info = mach_load_command_find_command_by_type (macho, LC_MAIN);
    if (!info) return 0;

    mach_entry_point_command_t *entry = info->lc;
    return (__pagezero->vmaddr + __pagezero->vmsize) + entry->entryoff;
}

HTOOL_PRIVATE
mach_section_64_t *
find_macho_executable_section (macho_t *macho)
{
    char **section_table[][2] = {
        { "__TEXT", "__text" },
        { "__TEXT_EXEC", "__text" },
    };
    for (int i = 0; i < (sizeof (section_table) / sizeof (char*)); i++) {
        mach_section_64_t *sect = mach_section_64_search (macho->scmds, section_table[i][0], section_table[i][1]);
        if (sect) 
            return sect;
    }
    return NULL;
}

HTOOL_PRIVATE
struct hashmap *
fetch_macho_inline_symbol_hashmap (macho_t *macho)
{
    /* Create the hashmap */
    struct hashmap *map = hashmap_new (sizeof (inline_symbol_t), 0, 0, 0,
            hashmap_hash, hashmap_compare, NULL, NULL);

    /* Add all the sections first */
    for (int i = 0; i < h_slist_length (macho->scmds); i++) {
        mach_segment_info_t *info = (mach_segment_info_t *) h_slist_nth_data (macho->scmds, i);
        for (int j = 0; j < h_slist_length (info->sections); j++) {
            mach_section_64_t *sect = (mach_section_64_t *) h_slist_nth_data (info->sections, j);

            uint32_t len = strlen (sect->segname) + strlen (sect->sectname) + 2;
            char *name = calloc (1, len);
            snprintf (name, len, "%s.%s\0", sect->segname, sect->sectname);

            hashmap_set (map, &(inline_symbol_t){ .name=name, .type="section", .virt_addr=sect->addr });
            //printf ("name: %s\n", name);
        }
    }

    /* Do not add symbols for Fileset-style kernels */
    if (macho->header->filetype == MACH_TYPE_FILESET)
        return map;

    /* Find the symbol table */
    mach_load_command_info_t *info = mach_load_command_find_command_by_type (macho, LC_SYMTAB);
    if (!info) return NULL;

    mach_symtab_command_t *table = (mach_symtab_command_t *) info->lc;

    assert (table);

    uint32_t offset = table->symoff;
    uint32_t nlist_size = sizeof (nlist);
    for (int i = 0; i < table->nsyms; i++) {

        /* Find the current symbols 'nlist' */
        nlist *curr = (nlist *) macho_load_bytes (macho, nlist_size, offset);
        char *name = mach_symbol_table_find_symbol_name (macho, curr, table);
        
        /* Only add the symbols if it has a name */
        if (strcmp (name, LIBHELPER_MACHO_SYMBOL_NO_NAME) && curr->n_value)
            hashmap_set (map, &(inline_symbol_t){ .name=name, .type="method", .virt_addr=curr->n_value });

        offset += nlist_size;
    }

    return map;
}


///////////////////////////////////////////////////////////////////////////////

htool_return_t
htool_disassemble (unsigned char *data, uint32_t size, uint64_t base_address)
{
    for (int i = 0; i < size; i++) {
        /* Get the next opcode */
        uint32_t opcode = *(uint32_t *) (data + (i * 4));
        instruction_t *in = libarch_instruction_create (opcode, base_address);
        libarch_disass (&in);

        printf (GREEN "   0x%016llx    " RESET "%08x\t", in->addr, SWAP_INT (in->opcode));
        htool_disassembler_parse_instruction (in);

        base_address += 4;
    }
}

htool_return_t
htool_disassemble_with_symbols (unsigned char *data, uint32_t size, uint64_t base_address, struct hashmap *inline_symbols)
{
    for (int i = 0; i < size; i++) {
        /* Get the next opcode */
        uint32_t opcode = *(uint32_t *) (data + (i * 4));
        instruction_t *in = libarch_instruction_create (opcode, base_address);
        libarch_disass (&in);

        /* Match a symbol */
        size_t iter = 0;
        void *item;
        while (hashmap_iter (inline_symbols, &iter, &item)) {
            const inline_symbol_t *func = item;
            if (func->virt_addr == in->addr) {
                printf (BLUE "   ;-- %s: %s:\n" RESET, func->type, func->name);
            }
        }

        printf (GREEN "   0x%016llx    " RESET "%08x\t", in->addr, SWAP_INT (in->opcode));
        htool_disassembler_parse_instruction (in);

        base_address += 4;
    }
}

htool_return_t
htool_disassemble_binary_quick (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;
    macho_t *macho;

    /**
     *  There are two types of files that can be disassembled: Mach-O 64-bit and
     *  RAW binaries.
     * 
     *  If the file is a RAW binary, it's disassembly starts at 0x0, unless otherwise
     *  specified with the --base-addr option.
     * 
     *  If the file is a Mach-O, it's base address depends on a few things:
     *   - If a specific address is given with --base-addr.
     *   - If the file is a Kernel, or some other known type that doesn't have a
     *     __TEXT and __PAGEZERO segment.
     *   - Whether the LC_MAIN load command exists.
     * 
     *  The first step is to determine the base address if the --base-addr option
     *  isn't set.
     */
    unsigned char *data;
    uint64_t base_addr;
    uint32_t size = 32;


    /**
     *  First check if the file is a Mach-O.
     */
    if (HTOOL_CLIENT_CHECK_FLAG(bin->flags, HTOOL_BINARY_FILETYPE_MACHO64) || HTOOL_CLIENT_CHECK_FLAG(bin->flags, HTOOL_BINARY_FILETYPE_FAT)) {

        /*  Fetch the best macho_t */
        macho = (macho_t *) calloc (1, sizeof (macho_t));
        htool_macho_select_arch (client, &macho);
        assert (macho);

        /**
         *  If the base address has already been specified, nothing needs to be
         *  done with the Mach-O.
         */
        if (client->opts & HTOOL_CLIENT_DISASS_OPT_BASE_ADDRESS) {
            base_addr = client->base_address;
            data = macho->data + find_offset_for_virtual_address (macho, base_addr);
    
        } else {

            /**
             *  If the LC_MAIN load command exists, we can start the disassembly from
             *  the location specified by it. However, we need the end of the __PAGEZERO
             *  segment as the base.
             * 
             */
            base_addr = find_macho_entry_point_virtual_address (macho);
            if (!base_addr) {
                /* If that didn't work, look for the base of the __TEXT segment */
                mach_section_64_t *sect = find_macho_executable_section (macho);
                assert (sect);
                if (!sect) return HTOOL_RETURN_FAILURE;

                /* Set the data and base_addr */
                data = macho->data + sect->offset;
                base_addr = sect->addr;
            }

        }

    } else {

        /**
         *  File is presumably a RAW binary, but the base address needs to be
         *  verified again, otherwise it's set to 0x0.
         */
        if (client->opts & HTOOL_CLIENT_DISASS_OPT_BASE_ADDRESS) base_addr = client->base_address;
        else base_addr = 0x0;

        data = bin->data + base_addr;
    }

    /**
     *  Determine the amount of bytes to disassemble from the base address.
     * 
     */
    if (client->opts & HTOOL_CLIENT_DISASS_OPT_STOP_ADDRESS) size = ((client->stop_address - client->base_address) / 4) + 1;
    else if (client->opts & HTOOL_CLIENT_DISASS_OPT_COUNT) size = client->size;
    debugf ("start: 0x%llx, end: 0x%llx, size: %d\n", base_addr, base_addr + size, size);

    /**
     *  Fetch a list of all inline functions and sections, so they can be printed when outputting
     *  the instructions.
     */
    struct hashmap *inline_symbols;
    if (HTOOL_CLIENT_CHECK_FLAG(bin->flags, HTOOL_BINARY_FILETYPE_MACHO64)) {
        inline_symbols = fetch_macho_inline_symbol_hashmap (macho);
        htool_disassemble_with_symbols (data, size, base_addr, inline_symbols);
    } else {
        htool_disassemble (data, size, base_addr);
    }

    return HTOOL_RETURN_SUCCESS;
}