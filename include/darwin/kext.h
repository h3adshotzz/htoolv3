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

#ifndef __HTOOL_KEXT_H__
#define __HTOOL_KEXT_H__

#include "htool-client.h"
#include "htool-loader.h"

#include "kernel.h"

/**
 * 
 */
typedef struct kext_t
{
    macho_t *macho;
    uint64_t offset;
    uint64_t vmaddr;

    char *bundleid;
    char *version;
    char *uuid;
} kext_t;

htool_return_t
xnu_parse_kernel_extensions (xnu_t *xnu);

#endif /* __htool_kext_h__ */