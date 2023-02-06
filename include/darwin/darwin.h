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
 *  NOTE:   This header file contains all the definitions needed to detect and 
 *          handle different versions of Darwin XNU firmware files up to the latest
 *          iOS version as-of March-ish 2023.   
 * 
 */

#ifndef __HTOOL_DARWIN_H__
#define __HTOOL_DARWIN_H__

#include <libhelper.h>

#include "htool-client.h"
#include "htool-loader.h"
#include "htool.h"



/***********************************************************************
* Darwin Component Detection.
*
*   These functions handle detecting different Darwin components such
*   as the Kernel, iBoot, SecureROM and SEP.
***********************************************************************/

/**
 * 
 */
typedef enum darwin_component_type_t
{
    DARWIN_COMPONENT_TYPE_UNKNOWN,
    DARWIN_COMPONENT_TYPE_IBOOT,
    DARWIN_COMPONENT_TYPE_KERNEL,
    DARWIN_COMPONENT_TYPE_SEPOS,
    DARWIN_COMPONENT_TYPE_IMG4
} darwin_component_type_t;

/**
 * 
 */
htool_return_t
darwin_detect_firmware_component_kernel (htool_binary_t *bin);

#endif /* __htool_darwin_h__ */