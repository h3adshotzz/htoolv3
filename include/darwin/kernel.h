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

#ifndef __HTOOL_KERNEL_H__
#define __HTOOL_KERNEL_H__

#include <libhelper.h>
#include <libhelper-macho.h>

#include "darwin.h"
#include "htool-loader.h"


/***********************************************************************
* XNU Kernelcache Loading.
*
*   These functions handle the identifcation and loading of an XNU
*   Kernelcache using the provided definitions aboves.
***********************************************************************/

/**
 *  \brief <explain the types here quickly>
 */
typedef enum xnu_kernel_type_t {

    /* Unknown Kernel Types */
    XNU_KERNEL_TYPE_UNKNOWN_PLATFORM = 0,
    XNU_KERNEL_TYPE_UNKNOWN_IOS,
    XNU_KERNEL_TYPE_UNKNOWN_MACOS,

    /* iOS Kernel Types */
    XNU_KERNEL_TYPE_IOS_IOS9,
    XNU_KERNEL_TYPE_IOS_SPLIT,
    XNU_KERNEL_TYPE_IOS_MERGED,
    XNU_KERNEL_TYPE_IOS_FILESET,
    XNU_KERNEL_TYPE_IOS_FILESET_EXTRACTED,

    /* macOS Kernel Types */
    XNU_KERNEL_TYPE_MACOS_X86_64,
    XNU_KERNEL_TYPE_MACOS_ARM64,
    XNU_KERNEL_TYPE_MACOS_ARM64_DTK

} xnu_kernel_type_t;

typedef struct xnu_version_t
{
    char        *darwin_vers;           //  Darwin Version  e.g. 19.2.0  
    char        *xnu_vers;              //  XNU Version     e.g. xnu-6153.60.66~39
    char        *build_time;            //  Build Time:     e.g. Mon Nov  4 17:46:45 PST 2019
    char        *device_type;           //  Device Type:    e.g. A13 (T8030)
    char        *cache_style;           //  Cache Style:    e.g. Merged (New-Style)
} xnu_version_t;

#define HTOOL_XNU_FLAG_FILESET_ENTRY        (1 << 1)

/**
 * 
 */
typedef struct xnu_t 
{
    /* Mach-O and associated KEXTs */
    macho_t                 *macho;
    macho_t                 *kern;      /* only if HTOOL_XNU_FLAG_FILESET_ENTRY is set */
    HSList                  *kexts;

    /* Parsed kernel properties */
    HSList                  *mach_traps;

    /* Non-string types */
    xnu_kernel_type_t       type;
    xnu_version_t          *version;

    uint32_t                flags;

} xnu_t;


xnu_t *
xnu_kernel_load_kernel_cache (htool_binary_t *bin);

macho_t *
xnu_select_macho (xnu_t *xnu);

xnu_kernel_type_t xnu_kernel_fetch_type (xnu_t *xnu);
char *
xnu_kernel_type_get_string (xnu_kernel_type_t type);
char *
xnu_kernel_soc_string (char *buffer);

#endif /* __htool_kernel_h__ */