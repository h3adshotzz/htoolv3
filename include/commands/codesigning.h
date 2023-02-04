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

/**
 *  NOTE:   Some definitions within this header file are taken from the XNU
 *          kernel header under osfmk/kern/cs_blob.h. The XNU version these
 *          were taken from is xnu-8792.41.9.
 * 
 *          The license with which XNU is released permits the use of these
 *          definitions in this project. Some additional explanation of the
 *          definitions are provided, as Apple like to be vague and cryptic.
 * 
*/

#ifndef __HTOOL_COMMAND_MACHO_CODE_SIGNING_H__
#define __HTOOL_COMMAND_MACHO_CODE_SIGNING_H__

#include <stdlib.h>

/**
 *  \brief  Definitions for a range of Code Signing properties and identifiers.
 *          Some of these are taken from Apple's header files, and other are 
 *          identified by either me or one of the blog posts I found on this
 *          subject, as much of the code-signing format, with exception of the
 *          code directory, is undocumented.
 * 
 */
enum {

    /* Code Signing Magic Numbers */
    kSecCodeMagicRequirement            = 0xfade0c00,	/* single requirement */
    kSecCodeMagicRequirementSet         = 0xfade0c01,	/* requirement set */

    kSecCodeMagicCodeDirectory          = 0xfade0c02,	/* CodeDirectory */
    kSecCodeMagicEmbeddedSignature      = 0xfade0cc0,   /* single-architecture embedded signature */
    kSecCodeMagicEmbeddedSignatureOld   = 0xface0b02,   /* XXX */
    kSecCodeMagicDetachedSignature      = 0xfade0cc1,   /* detached multi-architecture signature */

    kSecCodeMagicEntitlement            = 0xfade7171,	/* entitlement blob */
    kSecCodeMagicDEREntitlement         = 0xfade7172,   /* DER encoded entitlements */

    kSecCodeMagicBlobWrapper            = 0xfade0b01,   /* blob wrapper */

    /* Other Magic Numbers */
    kCertificateAuthorityString         = 0x04550306,

    kCertificateAuthorityStringCAType   = 0x0b,
    kCertificateAuthorityStringCNType   = 0x03,
    kCertificateAuthorityStringCountry  = 0x06,
    kCertificateAuthorityStringOrg      = 0x0a,

    /* Special Slot Identifiers */
    kSpecialSlotCodeDirectory                   = 0,
    kSpecialSlotInfoSlot                        = 1,
    kSpecialSlotRequirements                    = 2,
    kSpecialSlotResourceDir                     = 3,
    kSpecialSlotApplication                     = 4,
    kSpecialSlotEntitlements                    = 5,
    kSpecialSlotEntitlementsDer                 = 6,
    kSpecialSlotLaunchConstraintSelf            = 7,
    kSpecialSlotLaunchConstraintParent          = 8,
    kSpecialSlotLaunchConstraintResponsible     = 9,

    /* Requirement types */
    kRequirementTypeHost                        = 1,
    kRequirementTypeGuest                       = 2,
    kRequirementTypeDesignated                  = 3,
    kRequirementTypeLibrary                     = 4,
    kRequirementTypePlugIn                      = 5,
    kRequirementTypeUnknown                     = 0,

};

/**
 *  \brief  The structure for, as far as I can tell, the weird string format
 *          that Apple have used for the strings embedded in the Code Directory.
 * 
 *          This could possibly be something from Foundation.h, but idk.
 * 
 *          The first 4 bytes are the magic value, I've identified kCertificateAuthorityString
 *          as being one of them. This is then followed by a 1-byte type value, 
 *          then an unknown value, followed by the size and the pointer to the
 *          actual data.
 */
typedef struct __CMSString {
    uint32_t        magic;
    uint8_t         type;
    uint8_t         unk;
    uint8_t         size;
    void            *data;
} CMS_String;

/**
 *  \brief  The Blob Index points to a particular blob and identifies it's type.
 */
typedef struct __BlobIndex {
    uint32_t        type;
    uint32_t        offset;
} CS_BlobIndex;

/**
 *  \brief  The Super Blob of the embedded signature contains a list of blob's
 *          which hold information such as the Code Directory, Requirements and
 *          Entitlements. 
 */
typedef struct __SuperBlob {
    uint32_t        magic;
    uint32_t        length;
    uint32_t        count;
    CS_BlobIndex    index[];
} CS_SuperBlob;

/**
 *  \brief  The Generic Blob is a general structure format that can apply to a
 *          number of different blob types, particularly the CMS blobs.
*/
typedef struct __GenericBlob {
    uint32_t        magic;
    uint32_t        length;
    char            data[];
} CS_GenericBlob;


/**
 *  \brief  The Requirement is an entry within the Requirement Set, pointed to by
 *          an Item. Much of this structure has been created from what I have
 *          identified in the hexdata of Mach-O's and reverse engineering Apple's
 *          tools and JTool, this is why there are a number of unknown properties.
 */
typedef struct __Requirement {
    uint32_t        magic;

    uint32_t        size;
    uint32_t        unk2;
    uint32_t        unk3;
    uint32_t        unk4;

    uint32_t        len;
    char            *string;
} CS_Requirement;

/**
 *  \brief  The Requirement Set Item, like the BlobIndex, points to a particular
 *          Requirement within the blob and identifies it's type.
 */
typedef struct __RequirementSetItem {
    uint32_t        type;
    uint32_t        offset;
} CS_RequirementSetItem;

/**
 *  \brief  The Requirement Set contains a list of all the requirement items contained
 *          within the Code Signature.
 */
typedef struct __RequirementSet {
    uint32_t        magic;
    uint32_t        len;
    uint32_t        count;

    CS_RequirementSetItem     *items;
} CS_RequirementSet;


/**
 *  \brief  Code Directory structure versions
 */
enum {
    CS_CODEDIRECTORY_VERSION_1 = 0x20100,
    CS_CODEDIRECTORY_VERSION_2 = 0x20200,
    CS_CODEDIRECTORY_VERSION_3 = 0x20300,
    CS_CODEDIRECTORY_VERSION_4 = 0x20400,
    CS_CODEDIRECTORY_VERSION_5 = 0x20500,
    CS_CODEDIRECTORY_VERSION_6 = 0x20600,
};

/**
 *  \brief  C form of the Code Directory structure, taken from cs_blobs.h within
 *          the XNU source code.
 */
typedef struct __CodeDirectory {
    
    /* Common fields across all versions */
    uint32_t        magic;                      /* magic number (CSMAGIC_CODEDIRECTORY) */
    uint32_t        length;                     /* total length of CodeDirectory blob */
    uint32_t        version;                    /* compatibility version */
    uint32_t        flags;                      /* setup and mode flags */
    uint32_t        hashOffset;                 /* offset of hash slot element at index zero */
    uint32_t        identOffset;                /* offset of identifier string */
    uint32_t        nSpecialSlots;              /* number of special hash slots */
    uint32_t        nCodeSlots;                 /* number of ordinary (code) hash slots */
    uint32_t        codeLimit;                  /* limit to main image signature range */
    uint8_t         hashSize;                   /* size of each hash in bytes */
    uint8_t         hashType;                   /* type of hash (cdHashType* constants) */
    uint8_t         platform;                   /* platform identifier; zero if not platform binary */
    uint8_t         pageSize;                   /* log2(page size in bytes); 0 => infinite */
    uint32_t        spare2;                     /* unused (must be zero) */

    char            end_earliest[0];

    /* Version 0x20100 */
    uint32_t        scatterOffset;              /* offset of optional scatter vector */
    char            end_withScatter[0];

    /* Version 0x20200 */
    uint32_t        teamOffset;                 /* offset of optional team identifier */
    char            end_withTeam[0];

    /* Version 0x20300 */
    uint32_t        spare3;                     /* unused (must be zero) */
    uint64_t        codeLimit64;                /* limit to main image signature range, 64 bits */
    char            end_withCodeLimit64[0];

    /* Version 0x20400 */
    uint64_t        execSegBase;                /* offset of executable segment */
    uint64_t        execSegLimit;               /* limit of executable segment */
    uint64_t        execSegFlags;               /* executable segment flags */
    char            end_withExecSeg[0];
    
    /* Version 0x20500 */
    uint32_t        runtime;
    uint32_t        preEncryptOffset;
    char            end_withPreEncryptOffset[0];

    /* Version 0x20600 */
    uint8_t         linkageHashType;
    uint8_t         linkageTruncated;
    uint16_t        spare4;
    uint32_t        linkageOffset;
    uint32_t        linkageSize;
    char            end_withLinkage[0];
} CS_CodeDirectory;



/**
 *  \brief      Function to locate and print the Code Directory in a nicely formatted way.
 * 
 *  \param raw      Pointer to the base of the code signature section.
 *  \param offset   Offset from the base of the code signature section.
 *  \param verbose  Option whether to print verbose information.
 * 
 *  \returns    A htool_return_t value as to whether the print was successful or not.
 */
htool_return_t
htool_code_signature_print_code_directory (void *raw, uint32_t offset, int verbose);

/**
 *  \brief      Function to locate and print the CMS Blob in a nicely formatted way.
 * 
 *  \param raw      Pointer to the base of the code signature section.
 *  \param offset   Offset from the base of the code signature section.
 *  \param verbose  Option whether to print verbose information.
 * 
 *  \returns    A htool_return_t value as to whether the print was successful or not.
 */
htool_return_t
htool_code_signature_print_cms_blob (void *raw, uint32_t offset, int verbose);

/**
 *  \brief      Function to locate and print the Requirements Set Blob in a nicely 
 *              formatted way.
 * 
 *  \param raw      Pointer to the base of the code signature section.
 *  \param offset   Offset from the base of the code signature section.
 *  \param verbose  Option whether to print verbose information.
 * 
 *  \returns    A htool_return_t value as to whether the print was successful or not.
 */
htool_return_t
htool_code_signature_print_requirements_set (void *raw, uint32_t offset, int verbose);

/**
 *  \brief      Function to locate and print the Entitlements DER Blob in a nicely 
 *              formatted way.
 * 
 *  \param raw      Pointer to the base of the code signature section.
 *  \param offset   Offset from the base of the code signature section.
 *  \param verbose  Option whether to print verbose information.
 * 
 *  \returns    A htool_return_t value as to whether the print was successful or not.
 */
htool_return_t
htool_code_signature_print_entitlements_der (void *raw, uint32_t offset, int verbose);

/**
 *  \brief      Function to locate and print the Entitlements Blob in a nicely formatted 
 *              way.
 * 
 *  \param raw      Pointer to the base of the code signature section.
 *  \param offset   Offset from the base of the code signature section.
 *  \param verbose  Option whether to print verbose information.
 * 
 *  \returns    A htool_return_t value as to whether the print was successful or not.
 */
htool_return_t
htool_code_signature_print_entitlements (void *raw, uint32_t offset, int verbose);


#endif /* __htool_command_macho_code_signing_h__ */