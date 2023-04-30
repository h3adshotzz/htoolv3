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

#ifndef __HTOOL_SEP_H__
#define __HTOOL_SEP_H__

#include "htool.h"
#include "htool-loader.h"

/* at offset 0x10f8 (pointer to it stored right after "legion2") */
typedef struct sep_data_hdr_64_t {
    uint8_t kernel_uuid[16];
    uint64_t unknown0;
    uint64_t kernel_base_paddr;
    uint64_t kernel_max_paddr;
    uint64_t app_images_base_paddr;
    uint64_t app_images_max_paddr;
    uint64_t paddr_max; /* size of SEP firmware image */
    uint64_t unknown1;
    uint64_t unknown2;
    uint64_t unknown3;
    uint64_t init_base_paddr;
    uint64_t unknown4;
    uint64_t unknown5;
    uint64_t unknown6;
    uint64_t unknown7;
    uint64_t unknown8;
    uint64_t unknown9;
    char init_name[16];
    uint8_t init_uuid[16];
    uint64_t unknown10;
    uint64_t unknown11;
    uint64_t n_apps;
} sep_data_hdr_64_t;

typedef struct sepapp_64_t {
    uint64_t phys_text;
    uint64_t size_text;
    uint64_t phys_data;
    uint64_t size_data;
    uint64_t virt;
    uint64_t entry;
    uint64_t unknown4;
    uint64_t unknown5;
    uint64_t unknown6;
    uint32_t minus_one;
    uint32_t unknown7;
    char app_name[16];
    uint8_t app_uuid[16];
    uint64_t unknown8;
} sepapp_64_t;

//////////////////////////////////////////////////////////////////

typedef enum sep_type_t
{
    SEP_FIRMWARE_TYPE_ROM = 0,
    SEP_FIRMWARE_TYPE_OS_32,
    SEP_FIRMWARE_TYPE_OS_32_COMP,
    SEP_FIRMWARE_TYPE_OS_64,
} sep_type_t;

typedef struct sep_app_t
{
    uint32_t offset;
    uint32_t size;
    unsigned char *data;

    char *name;
    char *version;
} sep_app_t;

typedef struct sep_t
{
    /* SEP Firmware raw data */
    unsigned char   *data;
    uint32_t         size;
    sep_type_t       type;

    /* SEP ROM version */
    char            *rom_version;
    char            *rom_builder;

    /* SEP Bootloader */
    uint32_t         bootloader_offset;
    uint32_t         bootloader_size;
    char            *bootloader_version;

    /* SEP Kernel */
    char            *kernel_version;
    uint32_t         kernel_offset;
    uint32_t         kernel_size;
    unsigned char   *kernel; // macho_32_t

    /* Applications */
    HSList          *apps;
} sep_t;

htool_return_t
detect_sep_firmware (htool_binary_t *bin);
sep_t *
parse_sep_firmware (htool_binary_t *bin);

#endif /* __htool_sep_h__ */