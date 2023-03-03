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

#ifndef __HTOOL_IBOOT_H__
#define __HTOOL_IBOOT_H__

#include <stdlib.h>

#include <libhelper.h>
#include <libhelper-logger.h>
#include <libhelper-hlibc.h>

#include "htool.h"
#include "htool-error.h"
#include "htool-loader.h"

#include "darwin/darwin.h"

/***********************************************************************
* LZFSE Parsing and Information.
***********************************************************************/

struct lzfse_compressed_header {
    uint32_t        magic;
    uint32_t        raw_bytes;
    uint32_t        payload_bytes;
};

#define LZFSE_EMBEDDED_INFO_SIZE        sizeof(struct lzfse_embedded_info)
#define LZFSE_COMPRESSED_HEADER_SIZE    sizeof(struct lzfse_compressed_header)

#define LZFSE_BLOB_START            "bvx2"
#define LZFSE_BLOB_END              "bvx$"

struct lzfse_embedded_info          *lzfse_embedded_info_create ();
struct lzfse_compressed_header      *lzfse_compressed_header_create ();


/***********************************************************************
* iBoot Parsing and Information.
***********************************************************************/

#define OFFSET(x)                   (uint64_t) ((uint64_t *) x)

typedef struct pmu13_t
{
    uint64_t pmu_fw;
    uint32_t pmu_sz;
    uint32_t pmu_uuid_offset;
} pmu13_t;

typedef struct pmu14_t
{
    uint64_t pmu_fw;
    uint64_t pmu_fw_;
    uint64_t pmu_fw_end;
    uint64_t unk;
    uint32_t pmu_sz;
    uint32_t pmu_uuid_offset;
} pmu14_t;

typedef enum payload_arch_t
{
    IBOOT_EMBEDDED_PAYLOAD_ARCH_ARM64,
    IBOOT_EMBEDDED_PAYLOAD_ARCH_ARM32,
    IBOOT_EMBEDDED_PAYLOAD_ARCH_UNKNOWN,
} payload_arch_t;

typedef enum payload_type_t
{
    IBOOT_EMBEDDED_IMAGE_TYPE_LZFSE,
    IBOOT_EMBEDDED_IMAGE_TYPE_PMU,
    IBOOT_EMBEDDED_IMAGE_TYPE_RAW,
} payload_type_t;

typedef struct iboot_payload_t
{
    uint32_t        start;
    uint32_t        end;
    uint32_t        size;

    payload_arch_t      arch;
    payload_type_t      type;

    /* only used if type == IBOOT_EMBEDDED_IMAGE_TYPE_LZFSE */
    unsigned char       *decomp;
    uint32_t             decomp_size;

    char *name;

} iboot_payload_t;

typedef struct iboot_t {

    /* iBoot file propreties */
    unsigned char *data;
    uint32_t size;
    uint32_t base;

    /* iBoot version */
    char *iboot_version;
    char *ios_version;
    char *device;

    /* Embedded Firmware */
    HSList *payloads;


} iboot_t;


iboot_t *
iboot_load (htool_binary_t *bin);

char *
iboot_payload_get_type_string (payload_type_t type);

#endif /* __htool_iboot_h__ */