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

#ifndef __HTOOL_IBOOT_H__
#define __HTOOL_IBOOT_H__

#include <stdlib.h>

#include <libhelper.h>
#include <libhelper-logger.h>
#include <libhelper-hlibc.h>

#include "htool.h"
#include "htool-loader.h"


/**
 * 
 */
typedef struct iboot_t {

    /* iBoot file propreties */
    unsigned char *data;
    uint32_t size;

} iboot_t;


iboot_t *
iboot_load (htool_binary_t *bin);

#endif /* __htool_iboot_h__ */