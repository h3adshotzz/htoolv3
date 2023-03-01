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

#ifndef __HTOOL_MACH_TRAPS_H__
#define __HTOOL_MACH_TRAPS_H__

#include <libhelper.h>
#include <libhelper-macho.h>

#include "htool-loader.h"
#include "darwin.h"
#include "kernel.h"

htool_return_t
mach_parse_trap_table (xnu_t *xnu);


#endif /* __htool_mach_traps_h__ */