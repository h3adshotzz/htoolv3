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

#include <libhelper.h>
#include <libhelper-image4.h>

#include "commands/macho.h"
#include "darwin/darwin.h"
#include "htool-loader.h"

htool_return_t
darwin_detect_firmware_component_kernel (htool_binary_t *bin)
{
    /**
     *  We're assuming that as this function is meant to be called by the loader
     *  parsing function, that the binary passed should have at least one mach-o
     *  within it.
     * 
     *  We can determine if a given binary is a Kernel if two conditions are met:
     *      * The Mach-O has a __PRELINK_INFO segment.
     *      * The "Darwin Kernel Version" string exists within the Mach-O.
     * 
     *  If both of these conditions are met, we can probably assume it's a Mach-O.
     */
    macho_t *kern_macho = (macho_t *) h_slist_nth_data (bin->macho_list, 0);
    mach_segment_info_t *kern_prelink_info = mach_segment_info_search (kern_macho->scmds, "__PRELINK_INFO");

    if (!kern_prelink_info) return HTOOL_RETURN_FAILURE;

    /**
     *  The bh_memmem function, from libhelper, is a fast way of searching a buffer
     *  for a given string, or 'needle'. If it doesn't exist, then return a failure
     *  code.
     */
    char *needle = "Darwin Kernel Version ";
    unsigned char *uname = (unsigned char *) bh_memmem ((unsigned char *) bin->data, bin->size,
        (unsigned char *) needle, strlen (needle));

    return (uname) ? HTOOL_RETURN_SUCCESS : HTOOL_RETURN_FAILURE;
}
