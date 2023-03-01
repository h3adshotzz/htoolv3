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

#ifndef __HTOOL_COMMAND_MACHO_H__
#define __HTOOL_COMMAND_MACHO_H__

#include <libhelper-logger.h>
#include <libhelper-macho.h>
#include <libhelper.h>

#include "htool-client.h"
#include "htool.h"


/***********************************************************************
*                  HTool Mach-O Handler Functions
************************************************************************/

/**
 * \brief       Return values for select_macho_arch
 */
enum {
    SELECT_MACHO_ARCH_FAIL = 0,
    SELECT_MACHO_ARCH_FAIL_NO_ARCH,
    SELECT_MACHO_ARCH_IS_FAT,
    SELECT_MACHO_ARCH_IS_MACHO,
};

int
htool_macho_select_arch (htool_client_t *client, macho_t **macho);

htool_return_t
htool_macho_check_fat (htool_client_t *client);

htool_return_t
macho_search_section (macho_t *macho, mach_section_64_t **ptr, char *segment, char *section);

/***********************************************************************
*                   HTool Mach-O Print Functions
************************************************************************/

/**
 *  \brief      Print the Mach-O header of the file loaded into the client
 *              structure. If the file is a FAT archive, check the --arch
 *              flag and, if that's not set, print the FAT header. For the
 *              header print, no error is shown if no --arch is specified.
 * 
 *  \param client       HTool Client instance.
 */
htool_return_t
htool_print_header (htool_client_t *client);

/**
 *  \brief      Print the Mach-O Load Commands of the file loaded into the
 *              client structure. If the file is a FAT archive, but the
 *              --arch flag isn't set, print the FAT header again and an
 *              error.
 * 
 *  \param client       HTool Client instance.
 */
htool_return_t
htool_print_load_commands (htool_client_t *client);

/**
 *  \brief      Print the Mach-O Shared Libraries of the file loaded into 
 *              the client structure. If the file is a FAT archive, but the
 *              --arch flag isn't set, print the FAT header again and an
 *              error.
 * 
 *  \param client       HTool Client instance.
 */
htool_return_t
htool_print_shared_libraries (htool_client_t *client);

/**
 *  \brief      Print the Mach-O Static Symbols of the file loaded into the
 *              client structure. If the file is a FAT archive, but the
 *              --arch flag isn't set, print the FAT header again and an
 *              error.
 * 
 *  \param client       HTool Client instance.
 */
htool_return_t
htool_print_static_symbols (htool_client_t *client);

/**
 *  \brief      Print the Mach-O Code Signature of the file loaded into the
 *              client structure. If the file is a FAT archive, but the
 *              --arch flag isn't set, print the FAT header again and an
 *              error.
 * 
 *  \param client       HTool Client instance.
 */
htool_return_t
htool_print_code_signature (htool_client_t *client);

/**
 *  \brief      Select the specified architecture from the, presumably,
 *              FAT archive loaded in the given HTool binary, and return
 *              an appropriate return code.
 * 
 *  \param bin          HTool binary with loaded FAT archive.
 *  \param arch_name    Name of architecture to search FAT archive for.
 * 
 *  \returns    Returns the macho_t for the selected architecture, or NULL.
 */
macho_t *
htool_binary_select_arch (htool_binary_t *bin, char *arch_name);


/***********************************************************************
*                HTool Load Command Print Functions
************************************************************************/

/**
 *  \brief      Print a Mach-O Header struct in a colour-coded format.
 * 
 *  \param hdr      mach_header_t to print.
 */
void
htool_print_macho_header_from_struct (mach_header_t *hdr);

/**
 *  \brief      Print a FAT Header struct in a colour-coded format.
 * 
 *  \param info     FAT archive info struct.
 *  \param expand   Whether or not to print details of each architecture
 *                  contained in the FAT archive.
 */
void
htool_print_fat_header_from_struct (fat_info_t *info, int expand);

/**
 *  \brief      Print a Mach-O DYLIB Load Command in a colour-coded
 *              format.
 * 
 *  \param dylibs       List of Dynamic Library load commands to print.
 *  \param dylib_count  Number of dylibs to print. 
 */
void
htool_print_dylib_command (HSList *dylibs, int dylib_count);

/**
 *  \brief      Print a Mach-O Sub Framework Load Command in a colour
 *              -coded format.
 * 
 *  \param macho    Mach-O struct containing the Load Command.
 *  \param info     Load Command information struct/wrapper.
 *  \param lc_raw   Pointer to the base of the load command within the
 *                  Mach-O in memory.
 */
void
htool_print_sub_framework_command (macho_t *macho, mach_load_command_info_t *info, void *lc_raw);

/**
 *  \brief      Print a Mach-O Prebound Dylib Load Command in a colour
 *              -coded format.
 * 
 *  \param macho    Mach-O struct containing the Load Command.
 *  \param info     Load Command information struct/wrapper.
 */
void
htool_print_prebound_dylib_command (macho_t *macho, mach_load_command_info_t *info);

/**
 *  \brief      Print a Mach-O Dynamic Linker Load Command in a colour
 *              -coded format.
 * 
 *  \param macho    Mach-O struct containing the Load Command.
 *  \param info     Load Command information struct/wrapper.
 */
void
htool_print_dylinker_command (macho_t *macho, mach_load_command_info_t *info);

/**
 *  \brief      Print a Mach-O Linkedit Data Load Command in a colour-
 *              coded format.
 * 
 *  \param cmd      Pointer to the base of the load command within the
 *                  Mach-O in memory.  
 */
void
htool_print_linkedit_data_command (void *cmd);

/**
 *  \brief      Print a Mach-O DYLD Information Load Command in a colour-
 *              coded format.
 * 
 *  \param cmd      Pointer to the base of the load command within the
 *                  Mach-O in memory.  
 */
void
htool_print_dylid_info_command (void *cmd);

/**
 *  \brief      Print a Mach-O Thread State Load Command in a colour-
 *              coded format.
 * 
 *  \param cmd      Pointer to the base of the load command within the
 *                  Mach-O in memory.  
 */
void
htool_print_thread_state_command (void *lc_raw);

/**
 *  \brief      Print a Mach-O Synbol Table Load Command in a colour-
 *              coded format.
 * 
 *  \param lc_raw   Pointer to the base of the load command within the
 *                  Mach-O in memory.  
 */
void
htool_print_symtab_command (void *lc_raw);

/**
 *  \brief      Print a Mach-O Dynamic Symbol Table Load Command in a 
 *              colour-coded format.
 * 
 *  \param lc_raw   Pointer to the base of the load command within the
 *                  Mach-O in memory.  
 */
void
htool_print_dysymtab_command (void *lc_raw);

/**
 *  \brief      Print a Mach-O UUID Load Command in a colour-coded 
 *              format.
 * 
 *  \param lc_raw   Pointer to the base of the load command within the
 *                  Mach-O in memory.  
 */
void
htool_print_uuid_command (void *lc_raw);

/**
 *  \brief      Print a Mach-O RPATH Load Command in a colour-coded
 *              format.
 *
 *  \param macho    Mach-O struct containing the Load Command.
 *  \param info     Load Command information struct/wrapper.
 *  \param lc_raw   Pointer to the base of the load command within the
 *                  Mach-O in memory.
 */
void
htool_print_rpath_command (macho_t *macho, mach_load_command_info_t *info, void *lc_raw);

/**
 *  \brief      Print a Mach-O Source Version Load Command in a colour
 *              -coded format.
 * 
 *  \param lc_raw   Pointer to the base of the load command within the
 *                  Mach-O in memory.  
 */
void
htool_print_source_version_command (void *lc_raw);

/**
 *  \brief      Print a Mach-O Build Version Load Command in a colour
 *              -coded format.
 * 
 *  \param macho    Mach-O struct containing the Load Command.
 *  \param offset   Offset of the load commnad in the Mach-O file.
 *  \param lc_raw   Pointer to the base of the load command within the
 *                  Mach-O in memory.   
 */
void
htool_print_build_version_command (macho_t *macho, uint32_t offset, void *lc_raw);

/**
 *  \brief      Print a Mach-O Entry Point Load Command in a colour-coded 
 *              format.
 * 
 *  \param lc_raw   Pointer to the base of the load command within the
 *                  Mach-O in memory.  
 */
void
htool_print_entry_point_command (void *lc_raw);

/**
 *  \brief      Print a Mach-O Fileset Entry Load Command in a colour
 *              -coded format.
 * 
 *  \param macho    Mach-O struct containing the Load Command.
 *  \param offset   Offset of the load commnad in the Mach-O file.
 *  \param lc_raw   Pointer to the base of the load command within the
 *                  Mach-O in memory.   
 */
void
htool_print_fileset_entry_command (macho_t *macho, uint32_t offset, void *lc_raw);


///////////////////////////////////////////////////////////////////////////////

/**
 * NOTE:    The following definitions are ripped from the cctools project from Apple.
 * 
 *  MOVE TO A NEW HEADER UNDER include/commands/macho/symbols.h
 */

struct stabnames {
    unsigned char n_type;
    char *name;
};

/*
 * Symbolic debugger symbols.  The comments give the conventional use for
 * 
 * 	.stabs "n_name", n_type, n_sect, n_desc, n_value
 *
 * where n_type is the defined constant and not listed in the comment.  Other
 * fields not listed are zero. n_sect is the section ordinal the entry is
 * refering to.
 */
#define	N_GSYM	0x20	/* global symbol: name,,NO_SECT,type,0 */
#define	N_FNAME	0x22	/* procedure name (f77 kludge): name,,NO_SECT,0,0 */
#define	N_FUN	0x24	/* procedure: name,,n_sect,linenumber,address */
#define	N_STSYM	0x26	/* static symbol: name,,n_sect,type,address */
#define	N_LCSYM	0x28	/* .lcomm symbol: name,,n_sect,type,address */
#define N_BNSYM 0x2e	/* begin nsect sym: 0,,n_sect,0,address */
#define N_AST	0x32	/* AST file path: name,,NO_SECT,0,0 */
#define N_OPT	0x3c	/* emitted with gcc2_compiled and in gcc source */
#define	N_RSYM	0x40	/* register sym: name,,NO_SECT,type,register */
#define	N_SLINE	0x44	/* src line: 0,,n_sect,linenumber,address */
#define N_ENSYM 0x4e	/* end nsect sym: 0,,n_sect,0,address */
#define	N_SSYM	0x60	/* structure elt: name,,NO_SECT,type,struct_offset */
#define	N_SO	0x64	/* source file name: name,,n_sect,0,address */
#define	N_OSO	0x66	/* object file name: name,,0,0,st_mtime */
#define	N_LSYM	0x80	/* local sym: name,,NO_SECT,type,offset */
#define N_BINCL	0x82	/* include file beginning: name,,NO_SECT,0,sum */
#define	N_SOL	0x84	/* #included file name: name,,n_sect,0,address */
#define	N_PARAMS  0x86	/* compiler parameters: name,,NO_SECT,0,0 */
#define	N_VERSION 0x88	/* compiler version: name,,NO_SECT,0,0 */
#define	N_OLEVEL  0x8A	/* compiler -O level: name,,NO_SECT,0,0 */
#define	N_PSYM	0xa0	/* parameter: name,,NO_SECT,type,offset */
#define N_EINCL	0xa2	/* include file end: name,,NO_SECT,0,0 */
#define	N_ENTRY	0xa4	/* alternate entry: name,,n_sect,linenumber,address */
#define	N_LBRAC	0xc0	/* left bracket: 0,,NO_SECT,nesting level,address */
#define N_EXCL	0xc2	/* deleted include file: name,,NO_SECT,0,sum */
#define	N_RBRAC	0xe0	/* right bracket: 0,,NO_SECT,nesting level,address */
#define	N_BCOMM	0xe2	/* begin common: name,,NO_SECT,0,0 */
#define	N_ECOMM	0xe4	/* end common: name,,n_sect,0,0 */
#define	N_ECOML	0xe8	/* end common (local name): 0,,n_sect,0,address */
#define	N_LENG	0xfe	/* second stab entry with length information */

static const struct stabnames stabnames[] = {
    { N_GSYM,  "GSYM" },
    { N_FNAME, "FNAME" },
    { N_FUN,   "FUN" },
    { N_STSYM, "STSYM" },
    { N_LCSYM, "LCSYM" },
    { N_BNSYM, "BNSYM" },
    { N_OPT,   "OPT" },
    { N_RSYM,  "RSYM" },
    { N_SLINE, "SLINE" },
    { N_ENSYM, "ENSYM" },
    { N_SSYM,  "SSYM" },
    { N_SO,    "SO" },
    { N_OSO,   "OSO" },
    { N_LSYM,  "LSYM" },
    { N_BINCL, "BINCL" },
    { N_SOL,   "SOL" },
    { N_PARAMS,"PARAM" },
    { N_VERSION,"VERS" },
    { N_OLEVEL,"OLEV" },
    { N_PSYM,  "PSYM" },
    { N_EINCL, "EINCL" },
    { N_ENTRY, "ENTRY" },
    { N_LBRAC, "LBRAC" },
    { N_EXCL,  "EXCL" },
    { N_RBRAC, "RBRAC" },
    { N_BCOMM, "BCOMM" },
    { N_ECOMM, "ECOMM" },
    { N_ECOML, "ECOML" },
    { N_LENG,  "LENG" },
    { 0, 0 }
};


#endif /* __htool_command_macho_h__ */