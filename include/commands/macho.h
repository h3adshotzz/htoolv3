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


htool_return_t
htool_print_header (htool_client_t *client);
void
htool_print_load_commands (htool_client_t *client);


macho_t *
htool_binary_select_arch (htool_binary_t *bin, char *arch_name);

void
htool_print_macho_header_from_struct (mach_header_t *hdr);
void
htool_print_fat_header_from_struct (fat_info_t *info, int expand);
void
htool_print_dylib_command (HSList *dylibs, int dylib_count);
void
htool_print_sub_framework_command (macho_t *macho, mach_load_command_info_t *info, void *lc_raw);


#endif /* __htool_command_macho_h__ */