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

#include "secure_enclave/sep.h"
#include "iboot/iboot.h"

#include "darwin/darwin.h"
#include "darwin/kernel.h"
#include "darwin/kext.h"


htool_return_t
htool_analyse_kernel (htool_binary_t *bin)
{
    xnu_t *xnu = xnu_kernel_load_kernel_cache (bin);
    bin->firmware = (void *) xnu;

    if (xnu->type == XNU_KERNEL_TYPE_IOS_FILESET_EXTRACTED) {
        htool_error_throw (HTOOL_ERROR_FILETYPE, "Cannot parse KEXTs of an extracted Fileset Kernel\n");
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
    htool_error_throw (HTOOL_ERROR_NOT_IMPLEMENTED, "Parsing KEXTs is not yet implemented.\n");
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_analyse_iboot (htool_binary_t *bin)
{
    iboot_t *iboot = iboot_load (bin);
    bin->firmware = (void *) iboot;
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_analyse_sep (htool_binary_t *bin)
{
    sep_t *sep = parse_sep_firmware (bin);
    bin->firmware = (void *) sep;
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_generic_analyse (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;
    htool_return_t ret;

    printf (BOLD RED "[*] Analysing file:" RESET DARK_GREY " %s\n" RESET, client->filename);

    if (HTOOL_CLIENT_CHECK_FLAG (bin->flags, HTOOL_BINARY_FIRMWARETYPE_IBOOT)) ret = htool_analyse_iboot (client->bin);
    else if (HTOOL_CLIENT_CHECK_FLAG (bin->flags, HTOOL_BINARY_FIRMWARETYPE_KERNEL)) ret = htool_analyse_kernel (client->bin);
    else if (HTOOL_CLIENT_CHECK_FLAG (bin->flags, HTOOL_BINARY_FIRMWARETYPE_KEXT)) ret = htool_analyse_kext (client->bin);
    else if (HTOOL_CLIENT_CHECK_FLAG (bin->flags, HTOOL_BINARY_FIRMWARETYPE_SEP)) ret = htool_analyse_sep (client->bin);

    return ret;
}

htool_return_t
htool_analyse_list_all (htool_client_t *client)
{
    if (HTOOL_CLIENT_CHECK_FLAG (client->bin->flags, HTOOL_BINARY_FIRMWARETYPE_IBOOT)) {

        iboot_t *iboot = (iboot_t *) client->bin->firmware;
        uint32_t len = h_slist_length (iboot->payloads);
        if (!len) goto no_embedded_bin;

        printf (ANSI_COLOR_GREEN "[*]" RESET ANSI_COLOR_GREEN " Embedded firmware list:\n" RESET);
        printf (BOLD DARK_YELLOW "  %-12s%-10s%-10s\n", "Offset", "Type", "Name" RESET);
        for (int i = 0; i < len; i++) {
            iboot_payload_t *payload = (iboot_payload_t *) h_slist_nth_data (iboot->payloads, i);
            printf (BOLD DARK_WHITE "  0x%-10llx" RESET DARK_GREY "%-10s%s\n" RESET,
                payload->start, iboot_payload_get_type_string (payload->type), payload->name);
        }
        return HTOOL_RETURN_SUCCESS;

    } else if (HTOOL_CLIENT_CHECK_FLAG (client->bin->flags, HTOOL_BINARY_FIRMWARETYPE_KERNEL)) {

        /**
         *  Calling -l or --list-all on a kernel will list out each embedded Kernel Extension/KEXT
         *  contained within the file. These have already been parsed when the file was loaded, so
         *  it's as simple as printing out each kext_t from a HSList.
         */
        xnu_t *xnu = (xnu_t *) client->bin->firmware;
        uint32_t k_size = h_slist_length (xnu->kexts);
        
        if (!k_size) goto no_embedded_bin;

        printf (ANSI_COLOR_GREEN "[*]" RESET ANSI_COLOR_GREEN " KEXT List:\n" RESET);
        printf (BOLD DARK_YELLOW "  %-12s%-10s\n", "Offset", "Bundle ID" RESET);
        for (int i = 0; i < k_size; i++) {
            kext_t *kext = (kext_t *) h_slist_nth_data (xnu->kexts, i);
            printf (BOLD DARK_WHITE "  0x%-10llx" RESET DARK_GREY "%s\n" RESET,
                kext->offset, kext->name);
        }
        return HTOOL_RETURN_SUCCESS;
    
    } else if (HTOOL_CLIENT_CHECK_FLAG (client->bin->flags, HTOOL_BINARY_FIRMWARETYPE_SEP)) {

        sep_t *sep = (sep_t *) client->bin->firmware;
        if (sep->type != SEP_FIRMWARE_TYPE_OS_32) return HTOOL_RETURN_FAILURE;

        uint32_t len = h_slist_length (sep->apps);

        if (!len) goto no_embedded_bin;

        printf (ANSI_COLOR_GREEN "[*]" RESET ANSI_COLOR_GREEN " SEP App List:\n" RESET);
        printf (BOLD DARK_YELLOW "  %-12s%-10s%-25s%s\n", "Offset", "Size", "Name", "Version" RESET);
        for (int i = 0; i < len; i++) {
            sep_app_t *app = (kext_t *) h_slist_nth_data (sep->apps, i);
            printf (BOLD DARK_WHITE "  0x%-10llx" RESET DARK_GREY "%-10d%-24s (%s)\n" RESET,
                app->offset, app->size, app->name, app->version);
        }
        return HTOOL_RETURN_SUCCESS;
    } else {
        htool_error_throw (HTOOL_ERROR_FILETYPE, "Filetype not supported.");
    }

no_embedded_bin:
    printf (YELLOW "[*] Firmware file contains zero embedded binaries.\n");
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_analyse_extract (htool_client_t *client)
{
    char *name = client->extract;

    if (HTOOL_CLIENT_CHECK_FLAG (client->bin->flags, HTOOL_BINARY_FIRMWARETYPE_IBOOT)) {

        iboot_t *iboot = (iboot_t *) client->bin->firmware;
        uint32_t len = h_slist_length (iboot->payloads);

        if (!len) goto no_embedded_bin;

        printf (ANSI_COLOR_GREEN "[*] Searching binary for %s\n" RESET, name);
        for (int i = 0; i < len; i++) {
            iboot_payload_t *payload = (iboot_payload_t *) h_slist_nth_data (iboot->payloads, i);
            if (strcmp (payload->name, name)) continue;

            printf (ANSI_COLOR_GREEN "[*] Extracting Embedded payload:\n" RESET);

            uint32_t payload_size;
            unsigned char *payload_data;

            if (payload->type == IBOOT_EMBEDDED_IMAGE_TYPE_LZFSE) {
                payload_data = payload->decomp;
                payload_size = payload->decomp_size;
            } else {
                payload_data = (unsigned char *) (iboot->data + payload->start);
                payload_size = payload->size;
            }

            FILE *fp = fopen (payload->name, "w+");
            fwrite (payload_data, payload_size, 1, fp);
            fclose (fp);

            return HTOOL_RETURN_SUCCESS;
        }
        printf (YELLOW "[*] Could not find embedded firmware with given name\n");
        return HTOOL_RETURN_FAILURE;


    } else if (HTOOL_CLIENT_CHECK_FLAG (client->bin->flags, HTOOL_BINARY_FIRMWARETYPE_KERNEL)) {
        xnu_t *xnu = (xnu_t *) client->bin->firmware;
        uint32_t k_size = h_slist_length (xnu->kexts);

        if (!k_size) goto no_embedded_bin;

        printf ("[*] Searching binary for %s\n", name);
        for (int i = 0; i < k_size; i++) {
            kext_t *kext = (kext_t *) h_slist_nth_data (xnu->kexts, i);
            if (strcmp (kext->name, name)) continue;

            printf ("[*] Extracting KEXT:\n");

            FILE *fp = fopen (kext->name, "w+");
            fwrite (kext->macho->data, kext->macho->size, 1, fp);
            fclose (fp);

            return HTOOL_RETURN_SUCCESS;
        }
        printf (YELLOW "[*] Could not find embedded firmware with given name\n");
        return HTOOL_RETURN_FAILURE;

    } else if (HTOOL_CLIENT_CHECK_FLAG (client->bin->flags, HTOOL_BINARY_FIRMWARETYPE_SEP)) {

        /**
         *  Individual SEP Apps can't be extracted, they all have to be split and written
         *  to their own files. A SEPROM can't be split at all.
         * 
         *  First check that it's not a SEPROM
         */
        sep_t *sep = (sep_t *) client->bin->firmware;
        if (sep->type != SEP_FIRMWARE_TYPE_OS_32) {
            printf (YELLOW "[*] Cannot split a SEP ROM file, 32-bit Compressed SEPOS, or 64-bit SEPOS.\n" RESET);
            return HTOOL_RETURN_FAILURE;
        }

        /* First extract the bootloader */
        printf ("[*] Extracting SEPOS Bootloader...\n");
        unsigned char *bootloader = calloc (1, sizeof (sep->bootloader_size));
        memcpy (bootloader, sep->data + sep->bootloader_offset, sep->bootloader_size);

        FILE *fp = fopen ("sepos_bootloader", "w+");
        fwrite (bootloader, sep->bootloader_size, 1, fp);
        fclose (fp);

        /* Extract the Kernel */
        printf ("[*] Extracting SEPOS Kernel...\n");
        unsigned char *kernel = calloc (1, sizeof (sep->kernel_size));
        memcpy (kernel, sep->data + sep->kernel_offset, sep->kernel_size);

        fp = fopen ("sepos_kernel", "w+");
        fwrite (kernel, sep->kernel_size, 1, fp);
        fclose (fp);

        /* Extract the applications */
        for (int i = 0; i < h_slist_length (sep->apps); i++) {
            sep_app_t *app = (sep_app_t *) h_slist_nth_data (sep->apps, i);

            char *name;
            if (!strcmp (app->name, "Unknown")) asprintf (&name, "sepos_app%d", i);
            else asprintf (&name, "sepos_app%d_%s", i, app->name);

            printf ("[*] Extracting SEPOS App: %s...\n", name);

            unsigned char *data = calloc (1, app->size);
            memcpy (data, sep->data + app->offset, app->size);

            fp = fopen (name, "w+");
            fwrite (data, app->size, 1, fp);
            fclose (fp);
            
            name = NULL;
            free (name);
        }

        return HTOOL_RETURN_SUCCESS;
    }

no_embedded_bin:
    printf (YELLOW "[*] Firmware file contains zero embedded binaries.\n");
    return HTOOL_RETURN_SUCCESS;
}