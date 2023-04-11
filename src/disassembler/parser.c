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

#include "disassembler/parser.h"


int 
mstrappend2 (char **dst, const char *src, ...);


HSList *
htool_parse_macho_section_symbols (macho_t *macho)
{
    HSList *ret = NULL;
    int sect_count = 0;

    for (int i = 0; i < h_slist_length (macho->scmds); i++) {
        mach_segment_info_t *info = h_slist_nth_data (macho->scmds, i);
        mach_segment_command_64_t *seg = mach_segment_command_64_from_info (info);
        for (int j = 0; j < h_slist_length (info->sections); j++) {
            mach_section_64_t *sect = (mach_section_64_t *) h_slist_nth_data (info->sections, j);

            section_symbol_t *sym = (section_symbol_t *) calloc (1, sizeof (section_symbol_t));
            sym->index = sect_count;
            sym->virt_addr = sect->addr;
            sym->size = sect->size;
            sym->perms = mach_segment_read_vm_protection (seg->maxprot);

            mstrappend2 (&sym->name, "section.%d.%s.%s", sect_count, sect->segname, sect->sectname);
            assert (sym->name);
            
            ret = h_slist_append (ret, sym);
            sect_count += 1;
        }
    }

    return ret;
}

HSList *
htool_parse_macho_function_symbols (macho_t *macho)
{
    HSList *ret = NULL;
    mach_load_command_info_t *info = mach_load_command_find_command_by_type (macho, LC_SYMTAB);
    mach_symtab_command_t *table = (mach_symtab_command_t *) info->lc;

    assert (table);
    assert (table->nsyms);

    uint32_t offset = table->symoff;
    uint32_t nlist_size = sizeof (nlist);
    for (int i = 0; i < table->nsyms; i++) {

        /* Find the current symbols 'nlist' */
        nlist *curr = (nlist *) macho_load_bytes (macho, nlist_size, offset);

        /* Create an inline symbol struct for this symobl and add to the list */
        inline_symbol_t *sym = (inline_symbol_t *) calloc (1, sizeof (inline_symbol_t));
        sym->name = mach_symbol_table_find_symbol_name (macho, curr, table);
        sym->virt_addr = curr->n_value;

        /* Only add the symbol if it has a name */
        if (strcmp (sym->name, LIBHELPER_MACHO_SYMBOL_NO_NAME))
            ret = h_slist_append (ret, sym);
        
        offset += nlist_size;
    }
    return ret;
}


///////////////////////////////////////////////////////////////////////////////

HTOOL_PRIVATE
htool_return_t
_htool_disass_parse_instruction_mnemonic (instruction_t *instr)
{
    /**
     *  
     */
}

htool_return_t
htool_disassembler_parse_instruction (instruction_t **instr)
{
    



    return HTOOL_RETURN_SUCCESS;
}

static int _concat_internal(char **dst, const char *src, va_list args){
    if(!src || !dst)
        return 0;

    size_t srclen = strlen(src), dstlen = 0;

    if(*dst)
        dstlen = strlen(*dst);

    /* Back up args before it gets used. Client calls va_end
     * on the parameter themselves when calling vconcat.
     */
    va_list args1;
    va_copy(args1, args);

    size_t total = srclen + dstlen + vsnprintf(NULL, 0, src, args) + 1;

    char *dst1 = malloc(total);

    if(!(*dst))
        *dst1 = '\0';
    else{
        strncpy(dst1, *dst, dstlen + 1);
        free(*dst);
        *dst = NULL;
    }

    int w = vsnprintf(dst1 + dstlen, total, src, args1);

    va_end(args1);

    *dst = realloc(dst1, strlen(dst1) + 1);

    return w;
}

int 
mstrappend2 (char **dst, const char *src, ...)
{
    va_list args;
    va_start(args, src);

    int w = _concat_internal(dst, src, args);

    va_end(args);

    return w;
}