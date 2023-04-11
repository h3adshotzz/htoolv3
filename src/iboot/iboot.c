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

#include <libhelper-lzfse.h>
#include <libhelper-file.h>

#include "iboot/iboot.h"

char *iboot_find_version_string (iboot_t *iboot)
{
    unsigned char *needle = "iBoot-";
    return (char *) bh_memmem (iboot->data, iboot->size, needle, strlen (needle));
}

char *
iboot_payload_get_type_string (payload_type_t type)
{
    switch (type) {
        case IBOOT_EMBEDDED_IMAGE_TYPE_LZFSE:
            return "LZFSE";
        case IBOOT_EMBEDDED_IMAGE_TYPE_PMU:
            return "PMU";
        case IBOOT_EMBEDDED_IMAGE_TYPE_RAW:
            return "RAW";
        default:
            return "Unknown";
    }
}

darwin_device_t *
iboot_find_device_type (iboot_t *iboot)
{
    char *tmp = bh_memmem (iboot->data, iboot->size, "iBoot for", 9) + 10;
    char *version_string = calloc (1, strlen (tmp));
    memcpy (version_string, tmp, strlen (tmp));

    return strtok (version_string, ",");
}

iboot_payload_t *
iboot_search_power_management_firmware (iboot_t *iboot)
{
    iboot_payload_t *payload = calloc (1, sizeof (iboot_payload_t));

    /* Calculate the true iBoot base */
    uint64_t iboot_base = *(uint64_t *) (iboot->data + iboot->base);
    uint32_t pmu_fw_top = 0;
    uint64_t pmu_magic = 0x6800481200000000;
    unsigned char *pmu_text = bh_memmem (iboot->data, iboot->size, &pmu_magic, sizeof (pmu_magic));
    
    if (pmu_text) {
        pmu_text += 4;

        /**
         *  `pmu_base` points to the start of the __TEXT section, we need to count back until we
         *  reach the true base of the pmu firmware
         */
        unsigned char *pmu_base = pmu_text;
        while (pmu_fw_top != 0x20000101) {
            pmu_fw_top = *(uint32_t *) pmu_base;
            pmu_base -= 0x4;
        }
        uint64_t iboot_addr_pmu_fw = (pmu_base - iboot->data) + iboot_base;

        unsigned char *pmu_fw = bh_memmem (iboot->data, iboot->size, &iboot_addr_pmu_fw, sizeof (iboot_addr_pmu_fw));
        if (pmu_fw) {
            uint32_t pmu_fw_size = 0;
            if ( *(uint64_t *) pmu_fw == *(uint64_t *) (pmu_fw + 8) )
                pmu_fw_size = ((pmu14_t *) pmu_fw)->pmu_sz;
            else
                pmu_fw_size = ((pmu13_t *) pmu_fw)->pmu_sz;

            payload->start = iboot_addr_pmu_fw;
            payload->size = pmu_fw_size;
            payload->end = payload->start + payload->size;
            payload->name = bh_memmem (pmu_base, pmu_fw_size, &(uint64_t){0x554D50656C707041}, sizeof (uint64_t));
            payload->type = IBOOT_EMBEDDED_IMAGE_TYPE_PMU;
        }
    }

    return (payload) ? payload : NULL;
}

HSList *
iboot_search_lzfse_embedded_images (iboot_t *iboot)
{
    /**
     *  Some iBoot images contain a number of lzfse-compressed firmware images for 
     *  functionality such as PMU (Power Management Unit) and SMC (System Management 
     *  Controller).
     */
    HSList *list = NULL;
    unsigned char *newbase = calloc (1, iboot->size);
    memcpy (newbase, iboot->data, iboot->size);
    uint64_t newbase_ptr = OFFSET(newbase);

    while (newbase != NULL) {
        iboot_payload_t *payload = calloc (1, sizeof (iboot_payload_t));
        uint32_t img_offset = 0, img_size = 0;

        /**
         *  Look for the start of an LZFSE-encoded block by searching for LZFSE_BLOB_START,
         *  setting the `newbase`, and the start offset.
         */
        newbase = bh_memmem (newbase, iboot->size, LZFSE_BLOB_START, strlen (LZFSE_BLOB_START));
        uint64_t start = OFFSET (newbase);

        /**
         *  Look for the end of an LZFSE-encoded block by searching for LZFSE_BLOB_END,
         *  setting `tmp` and the end offset.
         */
        unsigned char *tmp = bh_memmem (newbase, iboot->size, LZFSE_BLOB_END, strlen (LZFSE_BLOB_END));
        uint64_t end = OFFSET (tmp);

        /* Set the image offset and size */
        img_offset = start - newbase_ptr;
        img_size = end - start;

        /* If the `img_offset` is larger than the size of iboot, there was an issue */
        if (img_offset > iboot->size) break;

        /* Set the uncompressed properties of the payload */
        payload->start = start;
        payload->end = end;
        payload->size = img_size + 4;

        /**
         *  The next step is to decompress the firmware image and create an iboot_payload_t
         *  struct that can be added to `list` and returned.
         * 
         *  We'll create a decompressed_blob and decompressed_size and do a memcpy from
         *  iboot->data as this data needs to be modified, and iboot->data is read-only.
         */
        unsigned char *decompressed_blob;
        uint32_t decompressed_size;

        decompressed_size = (payload->size - 4) * 4;
        decompressed_blob = calloc (1, decompressed_size);

        /* Call the lzfse decoder and check the result */
        decompressed_size = lzfse_decode_buffer ((uint8_t *) decompressed_blob, decompressed_size,
                        (uint8_t *) newbase, payload->size, NULL);
        if (!decompressed_size) {
            errorf ("decompressed_size: %d\n", decompressed_size);
        }

        /* Start setting some of the payload properties */
        payload->decomp_size = decompressed_size;
        payload->decomp = calloc (1, payload->decomp_size);
        memcpy (payload->decomp, decompressed_blob, decompressed_size);

        /* Fetch the name of the firmware */
        payload->name = bh_memmem (payload->decomp, payload->decomp_size, "Apple", 5);
        if (!payload->name) payload->name = "n/a";

        /* Determine the architecture */
        uint32_t arch = *(uint32_t *) decompressed_blob;
        if (arch == 0xEA000006) payload->arch = IBOOT_EMBEDDED_PAYLOAD_ARCH_ARM32;
        else if (arch == 0x14000081) payload->arch = IBOOT_EMBEDDED_PAYLOAD_ARCH_ARM64;
        else payload->arch = IBOOT_EMBEDDED_PAYLOAD_ARCH_UNKNOWN;

        list = h_slist_append (list, payload);

        /* reset `newbase` to the end of the last image found */
        newbase = tmp;
    }

    free (newbase);
    return list;
}

iboot_t *
iboot_load (htool_binary_t *bin)
{
    iboot_t *iboot;
    
    /* Set the initial values for the iboot struct */
    iboot = calloc (1, sizeof (iboot_t));
    iboot->data = bin->data;
    iboot->size = bin->size;

    /* Fetch information from the file regarding iBoot device, version and iOS version */
    char *dev = iboot_find_device_type (iboot);
    iboot->iboot_version = iboot_find_version_string (iboot);
    iboot->device = darwin_get_device_from_string (dev);
    iboot->ios_version = "n/a";

    /* Set iBoot base address depending on version */
    char *major = iboot->data + 0x286;
    uint32_t base_addr_offset = 0x318;
    if (atoi (major) > 5540) base_addr_offset = 0x300;
    iboot->base = base_addr_offset;

    if (!iboot->ios_version || !iboot->device) {
        htool_error_throw (HTOOL_ERROR_GENERAL, "Could not determine iBoot version or device.");
        return NULL;
    }
    ci_logf ("iBoot successfully loaded\n");

    printf (ANSI_COLOR_GREEN "[*]" RESET ANSI_COLOR_GREEN " Detected iBoot (iBoot)\n" RESET);
    printf ( BOLD DARK_WHITE "%siBoot Version:   " RESET DARK_GREY "%s\n" RESET, "    ", iboot->iboot_version);
    printf ( BOLD DARK_WHITE "%siOS Version:     " RESET DARK_GREY "%s\n" RESET, "    ", iboot->ios_version);
    printf ( BOLD DARK_WHITE "%sDevice:          " RESET DARK_GREY "%s\n" RESET, "    ", iboot->device);

    /* Search and fetch for embedded lzfse payloads within the iBoot binary */
    iboot->payloads = iboot_search_lzfse_embedded_images (iboot);

    /* Search for the Power Management firmware */
    iboot_payload_t *pmu_firmware = iboot_search_power_management_firmware (iboot);
    if (pmu_firmware) iboot->payloads = h_slist_append (iboot->payloads, pmu_firmware);

    printf (ANSI_COLOR_GREEN "[*] Successfully parsed Embedded Firmware (%d)\n" RESET, h_slist_length (iboot->payloads));

    return iboot;
}