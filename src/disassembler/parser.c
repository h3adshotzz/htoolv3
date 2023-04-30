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

htool_return_t
htool_disassembler_parse_instruction (instruction_t *instr)
{
    /* Handle Mnemonic */
    char *mnemonic = A64_INSTRUCTIONS_STR[instr->type];
    if (instr->cond != -1)
        printf ("%s.%s\t", mnemonic, A64_CONDITIONS_STR[instr->cond]);
    else if (instr->spec != -1)
        printf ("%s.%s\t", mnemonic, A64_VEC_SPECIFIER_STR[instr->spec]);
    else if (instr->type == ARM64_INSTRUCTION_UNK)
        printf (RED "%s\t" RESET, mnemonic);
    else
        printf (GREEN "%s\t" RESET, mnemonic);


    /**
     *  Operands are stored as operand_t structs in an array. This will loop
     *  round that array and handle each operand so they're formatted properly.
     * 
     */
    for (int i = 0; i < instr->operands_len; i++) {
        operand_t *op = &instr->operands[i];

        /* Register */
        if (op->op_type == ARM64_OPERAND_TYPE_REGISTER) {
            char *reg;

            const char **regs;
            size_t size;

            /**
             *  There are two types of register: System and General. They are
             *  handled differently.
             */
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

        op = NULL;
check_comma:
        if (i < instr->operands_len - 1) printf (", ");
    }

    printf ("\n");
    return HTOOL_RETURN_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////

static int _concat_internal(char **dst, const char *src, va_list args){
    if(!src || !dst)
        return 0;

    size_t srclen = strlen(src), dstlen = 0;

    if(*dst)
        dstlen = strlen(*dst);

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