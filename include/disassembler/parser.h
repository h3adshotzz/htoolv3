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

#ifndef __HTOOL_DISASSEMBLER_PARSER_H__
#define __HTOOL_DISASSEMBLER_PARSER_H__

#include "htool.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <instruction.h>
#include <register.h>
#include <libarch.h>

#include <libhelper-hlibc.h>
#include <libhelper-macho.h>

#define SWAP_INT(a)     ( ((a) << 24) | \
                        (((a) << 8) & 0x00ff0000) | \
                        (((a) >> 8) & 0x0000ff00) | \
                        ((unsigned int)(a) >> 24) )

/**
 * \brief       Structure to represent an inline symbol, whether that be
 *              a function name or the start location of a section.
 */
typedef struct inline_symbol_t {
    uint64_t    virt_addr;
    char       *name;
    char       *type;       // method, section
} inline_symbol_t;


/**
 * \brief       Disassemble a given `instruction_t` and output it to the
 *              terminal, colour-coded and formatted. 
 * 
 * \param   instr   Instruction to parse and print.
 * 
 * \return      Success or Failure based on the result of the parsing.
 */
htool_return_t
htool_disassembler_parse_instruction (instruction_t *instr);


#endif /* __htool_disassembler_parser_h__ */