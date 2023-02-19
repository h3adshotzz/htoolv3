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
#include "darwin/darwin.h"
#include "darwin/kernel.h"
#include "darwin/kext.h"

void htool_hexdump_formatted_print_byte_stream (char *mem, uint32_t size)
{
    uint32_t offset = 0;
    int lines = size / 16;
    int pos = 0;  //pos

    for (int i = 0; i < lines; i++) {

        printf ("%08x  ", offset);

        uint8_t ln[16];

        for (int j = 0; j < 16; j++) {
            uint8_t byte = (uint8_t) mem[pos];
            printf ("%02x ", byte);

            if (j == 7) printf (" ");

            pos++;
            ln[j] = byte;
        }

        printf ("  |");

        for (int k = 0; k < 16; k++) {

            if (ln[k] < 0x20 || ln[k] > 0x7e) {
                printf (".");
            } else {
                printf ("%c", (char) ln[k]);
            }

        }

        printf ("|\n");

        offset += 0x10;
    }

    printf ("\n");
}

void htool_hexdump_single_line (char *mem, uint32_t size)
{
    for (int a = 0; a < size; a++) printf ("%02x ", (uint8_t) mem[a]);
}


htool_return_t
htool_analyse_kernel (htool_binary_t *bin)
{
    printf ("file_type_kernel\n");
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
htool_analyse_iboot (htool_binary_t *bin)
{
    printf ("file_type_iboot\n");
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
htool_generic_analyse (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;

    printf (BOLD RED "[*] Analysing file:" RESET DARK_GREY " %s\n" RESET, client->filename);

    if (bin->flags & HTOOL_BINARY_FIRMWARETYPE_KERNEL) htool_analyse_kernel (client->bin);
    else if (bin->flags & HTOOL_BINARY_FIRMWARETYPE_IBOOT) htool_analyse_iboot (client->bin);

    return HTOOL_RETURN_SUCCESS;
}