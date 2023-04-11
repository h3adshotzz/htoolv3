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

void create_string (instruction_t *instr)
{
    /* Handle Mnemonic */
    char *mnemonic = A64_INSTRUCTIONS_STR[instr->type];
    if (instr->cond != -1) printf ("%s.%s\t", mnemonic, A64_CONDITIONS_STR[instr->cond]);
    else if (instr->spec != -1) printf ("%s.%s\t", mnemonic, A64_VEC_SPECIFIER_STR[instr->spec]);
    else printf (GREEN "%s\t" RESET, mnemonic);


    /* Handle Operands */
    for (int i = 0; i < instr->operands_len; i++) {
        operand_t *op = &instr->operands[i];

        /* Register */
        if (op->op_type == ARM64_OPERAND_TYPE_REGISTER) {
            char *reg;

            const char **regs;
            size_t size;

            if (op->reg_type == ARM64_REGISTER_TYPE_SYSTEM) {
                reg = libarch_get_system_register (op->reg);
            } else {
                if (op->reg_type == ARM64_REGISTER_TYPE_GENERAL) {
                    regs = (op->reg_size == 64) ? A64_REGISTERS_GP_64 : A64_REGISTERS_GP_32;
                    size = (op->reg_size == 64) ? A64_REGISTERS_GP_64_LEN : A64_REGISTERS_GP_32_LEN;

                    reg = libarch_get_general_register (op->reg, regs, size);
                } else if (op->reg_type == ARM64_REGISTER_TYPE_FLOATING_POINT) {
                    regs = A64_REGISTERS_FP_128;
                    size = A64_REGISTERS_FP_128_LEN;

                    reg = libarch_get_general_register (op->reg, regs, size);
                } else {
                    reg = "unk";
                }
            }

            // Print the register
            if (op->prefix) printf ("%c", op->prefix);
            printf (BLUE "%s" RESET, reg);
            if (op->suffix) printf ("%c", op->suffix);

            goto check_comma;
        }

        /* Immediate */
        if (op->op_type == ARM64_OPERAND_TYPE_IMMEDIATE) {
            if (op->prefix) printf ("%c", op->prefix);

            if (op->imm_type == ARM64_IMMEDIATE_TYPE_SYSC)
                printf (YELLOW "c%d" RESET, op->imm_bits);
            else if (op->imm_type == ARM64_IMMEDIATE_TYPE_SYSS)
                printf (YELLOW "s%d" RESET, op->imm_bits);
            else if (instr->type == ARM64_INSTRUCTION_SYS || instr->type == ARM64_INSTRUCTION_SYSL)
                printf (YELLOW "%d" RESET, op->imm_bits);
            else {
                if (op->imm_opts == ARM64_IMMEDIATE_OPERAND_OPT_PREFER_DECIMAL) {
                    printf (YELLOW "%d" RESET, op->imm_bits);
                } else {
                    if (op->imm_type == ARM64_IMMEDIATE_TYPE_LONG || op->imm_type == ARM64_IMMEDIATE_TYPE_ULONG)
                        printf (YELLOW "0x%llx" RESET, op->imm_bits);
                    else
                        printf (YELLOW "0x%x" RESET, op->imm_bits);
                }
            }

            if (op->suffix) printf ("%c", op->suffix);
            if (op->suffix_extra) printf ("%c", op->suffix_extra);
            
            goto check_comma;
        }

        /* Shift */
        if (op->op_type == ARM64_OPERAND_TYPE_SHIFT) {
            char *shift;

            if (op->shift_type == ARM64_SHIFT_TYPE_LSL) shift = "lsl";
            else if (op->shift_type == ARM64_SHIFT_TYPE_LSR) shift = "lsr";
            else if (op->shift_type == ARM64_SHIFT_TYPE_ASR) shift = "asr";
            else if (op->shift_type == ARM64_SHIFT_TYPE_ROR) shift = "ror";
            else if (op->shift_type == ARM64_SHIFT_TYPE_ROR) shift = "ror";
            else if (op->shift_type == ARM64_SHIFT_TYPE_MSL) shift = "msl";
            else continue;

            if (op->prefix) printf ("%c", op->prefix);
            printf ("%s #%d", shift, op->shift);
            if (op->suffix) printf ("%c", op->suffix);
            goto check_comma;
        }

        /* Target */
        if (op->op_type == ARM64_OPERAND_TYPE_TARGET) {
            if (op->target) printf ("%s", op->target);
            goto check_comma;
        }

        /* PSTATE */
        if (op->op_type == ARM64_OPERAND_TYPE_PSTATE) {
            printf ("%s", A64_PSTATE_STR[op->extra]);
        }

        /* Address Translate Name */
        if (op->op_type == ARM64_OPERAND_TYPE_AT_NAME) {
            printf ("%s", A64_AT_NAMES_STR[op->extra]);
        }

        /* TLBI Ops */
        if (op->op_type == ARM64_OPERAND_TYPE_TLBI_OP) {
            printf ("%s", A64_TLBI_OPS_STR[op->extra]);
        }

        /* Prefetch Operation */
        if (op->op_type == ARM64_OPERAND_TYPE_PRFOP) {
            printf ("%s", A64_PRFOP_STR[op->extra]);
        }

        /* Memory Barrier */
        if (op->op_type == ARM64_OPERAND_TYPE_MEMORY_BARRIER) {
            printf ("%s", A64_MEM_BARRIER_CONDITIONS_STR[op->extra]);
        }

        /* Index Extend */
        if (op->op_type == ARM64_OPERAND_TYPE_INDEX_EXTEND) {
            if (op->prefix) printf ("%c", op->prefix);
            printf ("%s", A64_INDEX_EXTEND_STR[op->extra]);
            if (op->suffix) printf ("%c", op->suffix);
            if (op->extra_val) printf (" %d", op->extra_val);
        }

check_comma:
        if (i < instr->operands_len - 1) printf (", ");
    }
    printf ("\n");
}

HTOOL_PRIVATE
mach_section_64_t *
_macho_find_executable_section (macho_t *macho)
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
uint64_t
_macho_find_offset_for_vm_address (macho_t *macho, uint64_t vmaddr)
{
    for (int i = 0; i < h_slist_length (macho->scmds); i++) {
        mach_segment_info_t *info = (mach_segment_info_t *) h_slist_nth_data (macho->scmds, i);
        mach_segment_command_64_t *seg = info->segcmd;

        if (vmaddr > seg->vmaddr && vmaddr < (seg->vmaddr + seg->vmsize))
            return seg->fileoff + (vmaddr - seg->vmaddr);
    }
    return 0;
}

HTOOL_PRIVATE
int inline_function_compare (const void *a, const void *b, const void *udata)
{
    const function_t *aa = a;
    const function_t *bb = b;
    return aa->virt_addr == bb->virt_addr;
}

HTOOL_PRIVATE
uint64_t inline_function_hash (const void *item)
{
    const function_t *func = item;
    uint8_t bytes[8];
    *(uint64_t *) bytes = func->virt_addr;
    return SIP64 (bytes, sizeof (bytes), 0, 0);
}

HTOOL_PRIVATE
struct hashmap *
_macho_fetch_function_symbol_hashmap (macho_t *macho)
{
    /* Create the hashmap */
    struct hashmap *map = hashmap_new (sizeof (function_t), 0, 0, 0,
            inline_function_hash, inline_function_compare, NULL, NULL);

    /* Find the symbol table */
    mach_load_command_info_t *info = mach_load_command_find_command_by_type (macho, LC_SYMTAB);
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
            hashmap_set (map, &(function_t){ .name=name, .virt_addr=curr->n_value });

        offset += nlist_size;
    }

    return map;
}

///////////////////////////////////////////////////////////////////////////////


htool_return_t
htool_disassemble_binary_quick (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;
    macho_t *macho;

    /**
     *  This function will run a quick disassembly of the provided binary, with minimal
     *  code annotations. What I mean here is that this option will only show where
     *  functions begin and end, but will not annotate on branches, where sections are
     *  or anything else added in the future.
     *  
     *  The first step is to work out the base address of where we need to be working
     *  from. This can be set with CLI flags, or, if left blank, we will look for the
     *  __TEXT.__text or __TEXT_EXEC.__text sections and use that as the base. If the
     *  file is not a Mach-O, we'll just disassemble it from 0x0.
     * 
     */
    unsigned char *data;
    uint64_t base_addr;
    uint32_t size = 32;

    
    if (bin->flags & HTOOL_BINARY_FILETYPE_MACHO64) {
        debugf ("macho-64\n");

        /* Fetch the appropriate Mach-O */
        macho = (macho_t *) calloc (1, sizeof (macho_t));
        htool_macho_select_arch (client, &macho);
        assert (macho);

        /**
         *  If the base/start address has been specified by the user, we don't need to
         *  find the executable section, only the base section to work out the correct
         *  offset.
         * 
         */
        if (client->opts & HTOOL_CLIENT_DISASS_OPT_START_ADDRESS) {
            base_addr = client->start_address;
            uint64_t offset = _macho_find_offset_for_vm_address (macho, base_addr);
            data = macho->data + offset;
            printf ("%x\n", _macho_find_offset_for_vm_address (macho, base_addr));
        
        } else {
            /* Try to find an executable __text section */
            mach_section_64_t *sect = _macho_find_executable_section (macho);
            assert (sect);
            if (!sect) return HTOOL_RETURN_FAILURE;

            /* Set the data and base_addr */
            data = macho->data + sect->offset;
            base_addr = sect->addr;
        }
    } else {
    
        /**
         *  This file is assumed to be a raw binary, but again we need to check if the
         *  start address was given
         */
        if (client->opts & HTOOL_CLIENT_DISASS_OPT_START_ADDRESS) {
            base_addr = client->start_address;
        } else {
            base_addr = 0;
        }
        data = bin->data + base_addr;
    }


    /**
     *  With quick disassembly, we still need to get a list of symbols (if the file
     *  is a Mach-O and contains symbols).
     * 
     */
    struct hashmap *inline_functions;
    if (bin->flags & HTOOL_BINARY_FILETYPE_MACHO64)
        inline_functions = _macho_fetch_function_symbol_hashmap (macho);

    /**
     * 
     */
    for (int i = 0; i < size; i++) {
        
        /* Get the next opcode */
        uint32_t opcode = *(uint32_t *) (data + (i * 4));
        instruction_t *in = libarch_instruction_create (opcode, base_addr);
        libarch_disass (&in);

        /* Does this instruction match a symbol? */
        if (bin->flags & HTOOL_BINARY_FILETYPE_MACHO64) {
            size_t iter = 0;
            void *item;
            while (hashmap_iter (inline_functions, &iter, &item)) {
                const function_t *func = item;
                if (func->virt_addr == in->addr) {
                    printf (BLUE "\n   ;-- method: %s:\n" RESET, func->name);
                    break;
                }
            }
        }

        printf (GREEN "   0x%016llx    " RESET "%08x\t", in->addr, SWAP_INT (in->opcode));
        create_string (in);

        base_addr += 4;
    }



}