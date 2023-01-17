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
 * NOTE: This file will eventually be merged with libhelper, but for the sake
 *       of the university project, the ELF loader will be implemented within
 *       HTool for the time being, and moved to libhelper once the project is
 *       completed.
 * 
*/

#ifndef __HTOOL_ELF_LOADER_H__
#define __HTOOL_ELF_LOADER_H__

#define EI_NIDENT 16

#define ELF_MAGIC 0x7f454c46
#define ELF_CIGAM 0x464c457f

typedef struct __elf_header {
    unsigned char   e_ident[EI_NIDENT];
};



#endif /* __htool_elf_loader_h__ */