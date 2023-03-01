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

#include "htool.h"

#include "commands/analyse.h"

#include "iboot/iboot.h"

#include "darwin/darwin.h"
#include "darwin/kernel.h"
#include "darwin/kext.h"


htool_return_t
htool_analyse_kernel (htool_binary_t *bin)
{
    debugf ("file_type_kernel\n");
    xnu_t *xnu = xnu_kernel_load_kernel_cache (bin);
    bin->firmware = (void *) xnu;

    if (xnu->type == XNU_KERNEL_TYPE_IOS_FILESET_EXTRACTED) {
        printf ("Cannot parse KEXTs of an extracted Fileset Kernel\n");
        return HTOOL_RETURN_FAILURE;
    }

    xnu_parse_kernel_extensions (xnu);

    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_analyse_kext (htool_binary_t *bin)
{
    /**
     *  This isn't one of the specified requirements for the project, but it might
     *  be something useful to implement in the future. Ideally the functionality
     *  to reconstruct the KEXT with the correct section information would be good.
     * 
     *  As it stands, the KEXT format used in the FILESET kernels can probably be
     *  reconstructed, since the segments are still together, they're just not
     *  within the area of the KEXT Mach-O. The other ones idk about but since they
     *  are old it's not a big deal if they're not supported.
     */
    printf ("analyse_kext\n");
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_analyse_iboot (htool_binary_t *bin)
{
    debugf ("file_type_iboot\n");
    iboot_t *iboot = iboot_load (bin);
    bin->firmware = (void *) iboot;



    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_generic_analyse (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;

    printf (BOLD RED "[*] Analysing file:" RESET DARK_GREY " %s\n" RESET, client->filename);

    if (HTOOL_CLIENT_CHECK_FLAG (bin->flags, HTOOL_BINARY_FIRMWARETYPE_IBOOT)) htool_analyse_iboot (client->bin);
    else if (HTOOL_CLIENT_CHECK_FLAG (bin->flags, HTOOL_BINARY_FIRMWARETYPE_KERNEL)) htool_analyse_kernel (client->bin);
    else if (HTOOL_CLIENT_CHECK_FLAG (bin->flags, HTOOL_BINARY_FIRMWARETYPE_KEXT)) htool_analyse_kext (client->bin);

    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_analyse_list_all (htool_client_t *client)
{
    /**
     *  Calling -l or --list-all on a kernel will list out each embedded Kernel Extension/KEXT
     *  contained within the file. These have already been parsed when the file was loaded, so
     *  it's as simple as printing out each kext_t from a HSList.
     */
    if (client->bin->flags & HTOOL_BINARY_FIRMWARETYPE_KERNEL) {
        xnu_t *xnu = (xnu_t *) client->bin->firmware;
        uint32_t k_size = h_slist_length (xnu->kexts);
        
        if (!k_size) goto no_embedded_bin;

        printf ("[*] KEXT List:\n");
        printf (BOLD DARK_YELLOW "  %-12s%-10s\n", "Offset", "Bundle ID" RESET);
        for (int i = 0; i < k_size; i++) {
            kext_t *kext = (kext_t *) h_slist_nth_data (xnu->kexts, i);
            printf (BOLD DARK_WHITE "  0x%-10llx" RESET DARK_GREY "%s\n" RESET,
                kext->offset, kext->name);
        }
        return HTOOL_RETURN_SUCCESS;
    
    } else if (client->bin->flags & HTOOL_BINARY_FIRMWARETYPE_IBOOT) {

        /**
         * 
         */

    }

no_embedded_bin:
    printf (YELLOW "[*] Firmware file contains zero embedded binaries.\n");
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_analyse_extract (htool_client_t *client)
{
    char *name = client->extract;
    if (client->bin->flags & HTOOL_BINARY_FIRMWARETYPE_KERNEL) {
        xnu_t *xnu = (xnu_t *) client->bin->firmware;
        uint32_t k_size = h_slist_length (xnu->kexts);

        if (!k_size) goto no_embedded_bin;

        printf ("[*] Searching binary for %s\n", name);
        for (int i = 0; i < k_size; i++) {
            kext_t *kext = (kext_t *) h_slist_nth_data (xnu->kexts, i);
            if (strcmp (kext->name, name)) continue;

            printf ("[*] Extracting KEXT\n");

            FILE *fp = fopen (kext->name, "w+");
            fwrite (kext->macho->data, kext->macho->size, 1, fp);
            fclose (fp);

            return HTOOL_RETURN_SUCCESS;
        }
        printf (YELLOW "[*] Could not find embedded firmware with given name\n");
        return HTOOL_RETURN_FAILURE;
    }

no_embedded_bin:
    printf (YELLOW "[*] Firmware file contains zero embedded binaries.\n");
    return HTOOL_RETURN_SUCCESS;
}