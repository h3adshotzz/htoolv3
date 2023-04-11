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

#include "commands/macho.h"

static htool_return_t
_print_symbol (htool_client_t *client, macho_t *macho, nlist *sym, char *name);

htool_return_t
htool_print_static_symbols (htool_client_t *client)
{
    macho_t *macho = (macho_t *) h_slist_nth_data (client->bin->macho_list, 0);
    
    mach_load_command_info_t *info = mach_load_command_find_command_by_type (macho, LC_SYMTAB);
    mach_symtab_command_t *table = (mach_symtab_command_t *) info->lc;

    if (!table) {
        warningf ("Cannot load symbol table: ...\n");
        return HTOOL_RETURN_FAILURE;
    }

    printf (BOLD RED "Symbols:\n" RESET);

    if (!table->nsyms) {
        printf (BLUE "  No Symbol Information\n" RESET, client->filename);
        return HTOOL_RETURN_FAILURE;
    }

    uint32_t offset = table->symoff;
    uint32_t nlist_size = sizeof (nlist);
    for (int i = 0; i < table->nsyms; i++) {
 
        /* Find the symbols `nlist`*/
        nlist *curr = (nlist *) macho_load_bytes (macho, nlist_size, offset);
        char *sym_name = mach_symbol_table_find_symbol_name (macho, curr, table);

        _print_symbol (client, macho, curr, sym_name);

        offset += nlist_size;
    }

    return HTOOL_RETURN_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

static char *
find_symbol_stab (unsigned char n_type)
{
    const struct stabnames *p;
    static char prbuf[32];

    for (p = stabnames; p->name; p++)
        if (p->n_type == n_type)
            return (p->name);

    sprintf (prbuf, "%02x", (unsigned int) n_type);
    return prbuf;
}

htool_return_t
_print_symbol (htool_client_t *client, macho_t *macho, nlist *sym, char *name)
{
    /* If the symbol has no name, return */
    if (!strcmp (name, LIBHELPER_MACHO_SYMBOL_NO_NAME))
        return HTOOL_RETURN_VOID;

    /* Determine the type of symbol */
    mach_section_64_t *sect64;
    unsigned char c = sym->n_type;

    if (sym->n_type & N_STAB) {

        /* Debugger symbol. If the client flag -sym-dbg is set, print it */
        if (client->opts & HTOOL_CLIENT_MACHO_OPT_SYMDBG) c = '-';
        else return HTOOL_RETURN_VOID;

        /* Check if the symbol belongs to a section_64 */
        if (sym->n_sect) sect64 = mach_section_find_at_index (macho->scmds, sym->n_sect);

        goto SYMBOL_OUTPUT;
    }

    /**
     * Switch through the different symbol types.
     */
    uint32_t type = sym->n_type & N_TYPE;
    switch (type) {

        /* Not Defined */
        case N_UNDF:
            c = 'u';
            if (sym->n_value != 0) c = 'c';
            break;

        /* */
        case N_ABS:
            c = 'a';
            break;

        /* Symbol is defined in segment */
        case N_SECT:

            /* Check if the symbol belongs to either TEXT/DATA or DATA._bss */
            sect64 = mach_section_find_at_index (macho->scmds, sym->n_sect);
            char *sect64_name = sect64->sectname;

            /* Set symbol type character */
            if (!strcmp (sect64_name, "__text")) c = 't';
            else if (!strcmp (sect64_name, "__data")) c = 'd';
            else if (!strcmp (sect64_name, "__bss")) c = 'b';
            else c = 's';

            break;

        /* Prebound */
        case N_PBUD:
            c = 'p';
            break;

        /* Indirect */
        case N_INDR:
            c = 'i';
            break;

        /* Default, nothing */
        default:
            break;
    }

SYMBOL_OUTPUT:

    /* Print the address of the symbol */
    if (sym->n_value != 0)
        printf (BOLD DARK_WHITE "0x%016llx" RESET, sym->n_value);
    else
        printf ("                  ");

    /* Print the type of the symbol */
    if (c != 'b') c = toupper (c);
    printf (BOLD DARK_YELLOW "  %c  " RESET, c);

    /* Check if the section should be printed too */
    if (client->opts & HTOOL_CLIENT_MACHO_OPT_SYMSECT && sect64 && sym->n_sect)
        printf ("(%s.%s)\t", sect64->segname, sect64->sectname);

    /* Show debug information for symbols */
    if (client->opts & HTOOL_CLIENT_MACHO_OPT_SYMDBG && (sym->n_type & N_STAB))
        printf ("%s  ", find_symbol_stab (sym->n_type));

    
    /* Print the name of the symbol */
    printf (DARK_GREY "%s\n" RESET, name);

    return HTOOL_RETURN_VOID;
}