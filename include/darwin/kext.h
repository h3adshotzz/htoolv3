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

#define MIN(a, b)               (((a) < (b)) ? (a) : (b))

/***********************************************************************
* XNU Kernel Extension loading and parsing.
***********************************************************************/

/**
 *  \brief      kext_t contains the raw and parsed data of an XNU Kernel 
 *              Extension, specifcally iOS Kernel Extensions. Kernel 
 *              Extensions work slightly differently on macOS, mainly because
 *              they are packaged into the kernel image.
 * 
 *      Looking at a Kernel Extension for macOS, there are full __TEXT and
 *      __DATA segments. It looks like a normal binary, but is still identified
 *      as a Mach Kernel Extension Bundle
 * 
 *      On iOS, there is only the __TEXT_EXEC segment present, and inside
 *      that only the __text section. This is probably something to do with
 *      the fact on iOS you cannot build kernel extensions, so they are
 *      trusted from the outset. There is no signature information in the
 *      Mach header, unlike macOS.
 * 
 */

typedef enum kext_type_t
{
    KERNEL_EXTENSION_FLAG_SPLIT_KEXT,
    KERNEL_EXTENSION_FLAG_MERGED_KEXT,
    KERNEL_EXTENSION_FLAG_FILESET_KEXT,
} kext_type_t;

typedef struct kext_t
{
    /* KEXT struct and location */
    macho_t         *macho;         /* KEXT Mach-O */
    uint64_t         offset;        /* Offset of KEXT in Kernel file */

    /* KEXT Identification */
    kext_type_t     type;
    char            *name;
    char            *version;
    char            *uuid;

    /* Split type properties */
    uint64_t         vmaddr;


    /* Merged type properties */
    uint64_t         kext_table;
    uint64_t         info_table;
    uint64_t         __text_vmaddr;
    uint64_t         kernel_ptr;


    /* Fileset type properties */


} kext_t;



/**
 * \brief       Macro for untagging an Arm MTE pointer.
 */
#define UNTAG_PTR(a)            ((a) | UINT64_C(0xffff000000000000))

#define KMOD_MAX_NAME       (64)

/**
 *  \brief      Partial kmod info structure from XNU headers.
 * 
 *              In the XNU source code, this is defined as `kmod_info`. 
 *              The next_addr proprety is actually a pointer to the next 
 *              kmod struct, and this acts as a statically-linked list. 
 *  
 *              This structure is layed out in __PRELINK_INFO.__kmod_info 
 *              segment. It maps out all of the Kext's contained within the 
 *              cache. We can't use the list functionality because we'd need 
 *              to define the entire struct and that just isn't required for 
 *              what we're doing.
 * 
 */
typedef struct partial_kmod_info_64_t {

    /* From xnu/osfmk/mach/kmod.h */ 
    uint64_t        next_addr;                  // Next kmod 
    int32_t         info_version;               // Structure version
    uint32_t        id;                         // Unknown
    char            name[KMOD_MAX_NAME];        // Kext Name
    char            version[KMOD_MAX_NAME];     // Kext Version

    /* Rest of the data is not relevant */

} partial_kmod_info_64_t;


htool_return_t
xnu_parse_kernel_extensions (xnu_t *xnu);

#endif /* __htool_kext_h__ */