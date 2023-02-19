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
#include "darwin/kernel.h"
#include "htool-loader.h"


char *darwin_get_device_from_string (char *string)
{
    darwin_device_t *device = malloc (sizeof(darwin_device_t));
    uint32_t size;
    char *ret;

    StringList *socs = strsplit (string, "/");

    for (int i = 0; i < DARWIN_DEVICE_LIST_LEN; i++) {
        device = &device_list[i];

        /**
         *  NOTE: We cannot determine the board type as some devices share the
         *      same SoC or Platform identifier. 
         */
        for (int i = 0; i < socs->count; i++) {
            char *a = socs->ptrs[i];
            //printf ("checking: %s, %s.%s.%s\n", a, device->soc, device->board, device->platform);
            if ( !strncasecmp (device->soc, string, strlen (string)) ||
                 !strncasecmp (device->board, string, strlen (string)) ||
                 !strncasecmp (device->platform, string, strlen (string))) {

                size = strlen (string) + strlen (device->soc) + 5;
                ret = malloc (size);

                // E.g. "M1/A14 Bionic (T8101)"
                snprintf (ret, size, "%s (%s)", string, device->soc);
                return ret;
            }
        }
    }
    
    // E.g. "<string> (Unknown)"
    size = strlen (string) + 11;
    ret = malloc (size);

    snprintf (ret, size, "%s (Unknown)", string);
    return ret;
}

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

htool_return_t
darwin_detect_firmware_component_iboot (htool_binary_t *bin)
{
    char *needle, *result;

    needle = "iBoot for";
    result = (char *) bh_memmem ((unsigned char *) bin->data, bin->size, (unsigned char *) needle, strlen (needle));
    if (!result)
        goto darwin_abort;

    needle = "Apple Mobile Device";
    result = (char *) bh_memmem ((unsigned char *) bin->data, bin->size, (unsigned char *) needle, strlen (needle));
    if (!result)
        goto darwin_abort;

    needle = "Apple Secure Boot";
    result = (char *) bh_memmem ((unsigned char *) bin->data, bin->size, (unsigned char *) needle, strlen (needle));
    if (!result)
        goto darwin_abort;

    return HTOOL_RETURN_SUCCESS;

darwin_abort:
    return HTOOL_RETURN_FAILURE;
}