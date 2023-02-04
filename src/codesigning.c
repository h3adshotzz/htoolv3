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

#include <time.h>

#include "htool.h"
#include "commands/macho.h"
#include "commands/codesigning.h"

/**
 * NOTE:    https://redmaple.tech/blogs/macho-files/#:~:text=Mach%2DO%20format.-,Apple%20Code%20Signing,and%20origin%20of%20the%20file.
 *          https://developer.apple.com/forums/thread/702351
 *          https://papers.put.as/papers/macosx/2017/Day4Session2.pdf
 *          https://medium.com/csit-tech-blog/demystifying-ios-code-signature-309d52c2ff1d
*/

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

#define SWAP_INT(a)  ( ((a) << 24) | \
		      (((a) << 8) & 0x00ff0000) | \
		      (((a) >> 8) & 0x0000ff00) | \
	 ((unsigned int)(a) >> 24) )

//////////////////////////////////////////////////////////////////

static char *
_fetch_code_signing_magic_name (uint32_t magic)
{
    char *ret; 
    switch (magic) {
        case kSecCodeMagicRequirement:
            ret = "kSecCodeMagicRequirement";
            break;
        case kSecCodeMagicRequirementSet:
            ret = "kSecCodeMagicRequirementSet";
            break;
        case kSecCodeMagicCodeDirectory:
            ret = "kSecCodeMagicCodeDirectory";
            break;
        case kSecCodeMagicEmbeddedSignature:
            ret = "kSecCodeMagicEmbeddedSignature";
            break;
        case kSecCodeMagicEmbeddedSignatureOld:
            ret = "kSecCodeMagicEmbeddedSignatureOld";
            break;
        case kSecCodeMagicDetachedSignature:
            ret = "kSecCodeMagicDetachedSignature";
            break;
        case kSecCodeMagicEntitlement:
            ret = "kSecCodeMagicEntitlement";
            break;
        case kSecCodeMagicDEREntitlement:
            ret = "kSecCodeMagicDEREntitlement";
            break;
        case kSecCodeMagicBlobWrapper:
            ret = "kSecCodeMagicBlobWrapper";
            break;

        default:
            ret = "unknown_type";
            break;
    }
    return ret;
}

htool_return_t
htool_print_code_signature (htool_client_t *client)
{
    htool_binary_t *bin = client->bin;
    int is_verbose = (client->opts & HTOOL_CLIENT_MACHO_OPT_VERBOSE);

    /**
     *  First, check if the file is a FAT and has --arch unset. If this is the
     *  case, print out an error as we cannot print load commands of a FAT if
     *  --arch is not set.
     */
    if (htool_macho_check_fat (client)) {
        
        /* print an error as --arch is not set */
        errorf ("htool_print_load_commands: Cannot print Shared libraries of a FAT archive. Please run with --arch=\n\n");
        
        /* if the --header option has been used, don't print the header again */
        if (!(client->opts & HTOOL_CLIENT_MACHO_OPT_HEADER)) {
            printf (BOLD RED "FAT Header:\n" RESET);
            htool_print_fat_header_from_struct (bin->fat_info, 1);    
        }
        
        /* there's nothing that can be done now, so exit */
        exit (EXIT_FAILURE);
    }

    /**
     *  Load the correct Mach-O. Either the only one in the macho_list, or
     *  the one specified by --arch.
     */
    macho_t *macho = calloc (1, sizeof (macho_t));
    if (htool_macho_select_arch (client, &macho) == SELECT_MACHO_ARCH_FAIL_NO_ARCH) {
        errorf ("htool_print_load_commands: Could not load architecture from FAT archive: %s\n", client->arch);
        htool_print_fat_header_from_struct (bin->fat_info, 1);

        exit (EXIT_FAILURE);
    }

    /**
     *  The function to fetch the LC_CODE_SIGNATURE load command returns it as an
     *  info struct, from which we can then get the generic load command.
     * 
     *  The LC_CODE_SIGNATURE Load Commands use the linkedit_data structure, so
     *  it can be loaded into that struct. It's useful here to make a copy of the
     *  code signature command.
     */
    mach_load_command_info_t *info = mach_load_command_find_command_by_type (macho, LC_CODE_SIGNATURE);
    mach_linkedit_data_command_t *cs_cmd = (mach_linkedit_data_command_t *) info->lc;

    void *raw = macho_load_bytes (macho, cs_cmd->datasize, cs_cmd->dataoff);

    printf ("\n" BOLD RED "Code Signature (%s): \n" RESET, mach_header_get_cpu_string (macho->header->cputype, macho->header->cpusubtype));

    /**
     *  The Code Signature section begins with a "SuperBlob" which has a `magic`,
     *  `length` (of the whole section), and a `count`. Following the `count` is an
     *  array of "BlobIndex's" which each point to a particular part of the code
     *  signature. e.g. CodeDictionary.
     * 
     *  The byte-order is also the wrong way round when loading from the file. The
     *  order of the bytes needs to be swapped using SWAP_INT.
     */
    CS_SuperBlob *blob = (CS_SuperBlob *) raw;
    blob->magic = SWAP_INT(blob->magic);
    blob->length = SWAP_INT(blob->length);
    blob->count = SWAP_INT(blob->count);

    printf (BOLD DARK_WHITE "Blob: " RESET DARK_GREY "Embedded signature of %d bytes, and %d blobs:\n" RESET,
        blob->length, blob->count);

    
    /**
     *  Iterate through the array of BlobIndex's. Each index has an offset at which
     *  a particular part of the code signature, like CodeDictionary, lies. The `magic`
     *  once identified can be used to print the data in the proper format.
     * 
     *  The base of the index's is directly after the SuperBlob, so sizeof(CS_SuperBlob)
     *  bytes off the base of the code signature section.
     */
    uint32_t offset = sizeof (CS_SuperBlob);
    for (int i = 0; i < blob->count; i++) {

        /**
         *  Create the BlobIndex struct at the current offset, again the bytes are in
         *  the wrong order so they need to be flipped.
         */
        CS_BlobIndex *index = (CS_BlobIndex *) (raw + offset);
        index->type = SWAP_INT(index->type);
        index->offset = SWAP_INT(index->offset);

        /**
         *  Now we want to determine what the offset from the current BlobIndex is
         *  actually pointing to, so grabbing the value at index->offset, and, again
         *  swapping the bytes. 
         */
        uint32_t magic = SWAP_INT(*(uint32_t *) (raw + index->offset));

        printf (BOLD DARK_WHITE "   Blob " RESET DARK_GREY "%d:" BOLD DARK_WHITE " Type: " RESET DARK_GREY "%d 0x%x:",
            i, index->type, index->offset);

        /**
         *  Check the magic against the currently identified code signature component
         *  types, and call the function necessary to print the data out in a nice way.
         */
        if (magic == kSecCodeMagicCodeDirectory) {
            htool_code_signature_print_code_directory (raw, index->offset, is_verbose);
        } else if (magic == kSecCodeMagicBlobWrapper) {
            htool_code_signature_print_cms_blob (raw, index->offset, is_verbose);
        } else if (magic == kSecCodeMagicRequirementSet) {
            htool_code_signature_print_requirements_set (raw, index->offset, is_verbose);
        } else if (magic == kSecCodeMagicDEREntitlement) {
            htool_code_signature_print_entitlements_der (raw, index->offset, is_verbose);
        } else if (magic == kSecCodeMagicEntitlement) {
            htool_code_signature_print_entitlements (raw, index->offset, is_verbose);
        } else {
            printf ("\n\t");
            warningf ("Code Signature blob 0x%08x not implemented\n", magic);
        }


        /* Increment the offset to the next BlobIndex */
        offset += sizeof (CS_BlobIndex);
    }

    return HTOOL_RETURN_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

#define SWAP_INT2(a)        (a = SWAP_INT(a))

htool_return_t
htool_code_signature_print_code_directory (void *raw, uint32_t offset, int verbose)
{
    const char *base = (const char *) (raw + offset);
    CS_CodeDirectory *code_directory = (CS_CodeDirectory *) base;

    /* Flipping all the values */
    code_directory->magic = SWAP_INT(code_directory->magic);
    code_directory->length = SWAP_INT(code_directory->length);
    code_directory->version = SWAP_INT(code_directory->version);
    code_directory->flags = SWAP_INT(code_directory->flags);
    code_directory->hashOffset = SWAP_INT(code_directory->hashOffset);
    code_directory->identOffset = SWAP_INT(code_directory->identOffset);

    code_directory->nSpecialSlots = SWAP_INT(code_directory->nSpecialSlots);
    code_directory->nCodeSlots = SWAP_INT(code_directory->nCodeSlots);
    code_directory->codeLimit = SWAP_INT(code_directory->codeLimit);

    /**
     *  Fetch the code directory identity string, usually the bundle ID of
     *  the application, and CD Hash.
     */
    char *ident = (char *) (base + code_directory->identOffset);
    char *cd_hash = "(unknown)";        /* can't work out how this is stored yet */

    /**
     *  Print the details of the CodeDirectory
     */
    printf (" Code Directory (%d bytes)\n", code_directory->length);

    printf (BOLD DARK_WHITE "\tVersion:     " RESET DARK_GREY "    %05x\n", code_directory->version);
    printf (BOLD DARK_WHITE "\tFlags:       " RESET DARK_GREY "    %d\n", code_directory->flags);
    
    printf (BOLD DARK_WHITE "\tPlatform:        ");
    if (code_directory->platform) printf (RESET DARK_GREY "Platform Binary\n" RESET);
    else printf (RESET DARK_GREY "none\n" RESET);

    printf (BOLD DARK_WHITE "\tCodeLimit:   " RESET DARK_GREY "    %d\n", code_directory->codeLimit);
    printf (BOLD DARK_WHITE "\tIdentifier:  " RESET DARK_GREY "    %s\n", ident);

    printf (BOLD DARK_WHITE "\tExecutable Segment:\n" RESET);
    printf (BOLD DARK_WHITE "\t   Base: " RESET DARK_GREY "        0x%08x\n", code_directory->execSegBase);
    printf (BOLD DARK_WHITE "\t   Limit: " RESET DARK_GREY "       0x%08x\n", code_directory->execSegLimit);
    printf (BOLD DARK_WHITE "\t   Flags: " RESET DARK_GREY "       0x%08x\n", code_directory->execSegFlags);

    printf ("\n");
    printf (BOLD DARK_WHITE "\tCDHash: " RESET DARK_GREY "         %s\n", cd_hash);
    printf (BOLD DARK_WHITE "\tHashes: " RESET DARK_GREY);
    printf (DARK_GREY "         %d code (%dK pages), %d special (0x%x)\n", code_directory->nCodeSlots, code_directory->pageSize, code_directory->nSpecialSlots, code_directory->hashOffset);


    printf (BOLD DARK_WHITE "\tPage Size: " RESET DARK_GREY "      %dK\n", code_directory->pageSize);
    printf (BOLD DARK_WHITE "\tCode Slots: " RESET DARK_GREY "     %d\n", code_directory->nCodeSlots);
    printf (BOLD DARK_WHITE "\tSpecial Slots: " RESET DARK_GREY "  %d\n", code_directory->nSpecialSlots);



    uint32_t hash_base = code_directory->hashOffset - (code_directory->nSpecialSlots * code_directory->hashSize);
    uint32_t hash_total = code_directory->nSpecialSlots + code_directory->nCodeSlots;
    
    /**
     * Print the special slots with their description
     */
    for (int i = code_directory->nSpecialSlots; i > 0; i--) {
        printf (BOLD DARK_WHITE "\t   Slot #%d: " RESET, i);

        if (i == kSpecialSlotCodeDirectory) printf (DARK_GREY "Code Directory:");
        else if (i == kSpecialSlotInfoSlot) printf (DARK_GREY "Bound Info.plist:");
        else if (i == kSpecialSlotRequirements) printf (DARK_GREY "Internal Requirements:");
        else if (i == kSpecialSlotResourceDir) printf (DARK_GREY "Resource Directory (_CodeResources):");
        else if (i == kSpecialSlotApplication) printf (DARK_GREY "Application Specific:");
        else if (i == kSpecialSlotEntitlements) printf (DARK_GREY "Entitlements:");
        else if (i == kSpecialSlotEntitlementsDer) printf (DARK_GREY "Entitlements DER:");
        else if (i == kSpecialSlotLaunchConstraintSelf) printf (DARK_GREY "Launch Constraint - Self:");
        else if (i == kSpecialSlotLaunchConstraintParent) printf (DARK_GREY "Launch Constraint - Parent:");
        else if (i == kSpecialSlotLaunchConstraintResponsible) printf (DARK_GREY "Launch Constraint - Responsible:");
        else printf (DARK_GREY "Unknown:\t");
        printf ("\n\t\t    ");
        
        unsigned char *hash = (unsigned char *) (base + hash_base) + (i * code_directory->hashSize);
        for (int j = 0; j < code_directory->hashSize; j++) printf (DARK_GREY "%02x", hash[j]);
        printf ("\n");
    }

    if (verbose) {
        printf (BOLD BLUE "\tVerbose Hash Info:\n" RESET);
        int pos = 0 - code_directory->nSpecialSlots;
        for (int i = 0; i < hash_total; i++) {
            int cur = pos + i;
            if (cur >= 0 && cur < 10) printf (BOLD DARK_WHITE "\t    %d: " RESET, cur);
            else printf (BOLD DARK_WHITE "\t   %d: " RESET, cur);

            /**
             *  Rethink how these are formatted, because at the moment its too messy and 
             *  doesn't give enough info.
             */

            unsigned char *hash = (unsigned char *) (base + hash_base) + (i * code_directory->hashSize);
            for (int j = 0; j < code_directory->hashSize; j++) printf (DARK_GREY "%02x", hash[j]);
            printf ("\n");
        }
    }
}

htool_return_t
htool_code_signature_print_cms_blob (void *raw, uint32_t offset, int verbose)
{  
    const char *base = (const char *) (raw + offset);
    CS_GenericBlob *blob = (CS_GenericBlob *) base;

    /* swap the values of the blob */
    blob->magic = SWAP_INT (blob->magic);
    blob->length = SWAP_INT (blob->length);

    printf (RESET " Blob Wrapper (%d bytes) CMS RFC3852 Signature\n", blob->length);

    /**
     *  Iterate through the blob until we match the byte sequence for either the
     *  CA or CN strings. The CA (Certificate Authority) should typically come
     *  before the CN.
     */
    for (int i = 0; i < blob->length; i++) {

        /**
         *  The format of the CMS blob is not officially documented by Apple, so this
         *  part of HTool has been developed by looking at the resources listed above,
         *  reverse engineering JTool (which was only helpful in finding the magics),
         *  and trial-and-error, which has resulted in almost everything being identified.
         * 
         *  CMS Blobs appear to contain four strings in an undocumented structure:
         * 
         *    1. The first four bytes are the "magic", they are 06 03 55 04, and I have
         *       created `kCertificateAuthorityString` to make this nicer to read.
         * 
         *    2. The byte following the magic is the "type", this identifies whether the
         *       string is the CA, CN, Organisation or Country. 
         * 
         *    3. The next byte is unknown.
         * 
         *    4. The seventh byte is the length of the string.
         * 
         *    5. Everything from the eighth byte to `n` is the string.
         * 
         *  I've tried to package this format into a C struct, as CMS_String, but as
         *  I still don't know what the unknown sixth byte is, it makes things tricky.
         */
        
        /**
         *  Create a CMS_String from the current pointer, and check if the magic matches.
         */
        CMS_String *cms_string = (CMS_String *) (base + i + 8);
        if (cms_string->magic == kCertificateAuthorityString) {
            
            /**
             *  Everything is printed the same, as it has the same structure. The only difference
             *  is that the different types require different labels. The actual string can be
             *  allocated and copied here.
             */
            unsigned char *str = malloc (cms_string->size);
            memcpy (str, (base + i + 8) + 7, cms_string->size);

            /**
             *  Now, we can check the string type and output the correct label. The Organisation
             *  and country can be printed in the future if we want.
             */
            if (cms_string->type == kCertificateAuthorityStringCAType) {
                printf (BOLD DARK_WHITE "\tCA:" RESET DARK_GREY" %s", str);
                if (cms_string->size < 20) printf ("\t\t");
            } else if (cms_string->type == kCertificateAuthorityStringCNType) {
                printf (BOLD DARK_WHITE "\tCN:" RESET DARK_GREY " %s\n", str);
            }

            str = NULL;
            free (str);        
        }

    }
}

htool_return_t
htool_code_signature_print_requirements_set (void *raw, uint32_t offset, int verbose)
{
    const char *base = (const char *) (raw + offset);

    /**
     *  The Code Signing Requirement Set blob contains a length and count, the count being
     *  the amount of Items within the set. As usual, the bytes need to be swapped.
     */
    CS_RequirementSet *set = (CS_RequirementSet *) base;
    SWAP_INT2(set->magic);
    SWAP_INT2(set->len);
    SWAP_INT2(set->count);

    printf (RESET " Requirement Set (%d bytes) with %d requirement:\n", set->len, set->count);

    /**
     *  The items begin at offset 12 (from base), with a sequence of CS_RequirementSetItem's
     *  which each define an offset, and a string type. 
     * 
     *  We'll loop through each item and print the value of the string it points too.
     */
    uint32_t item_offset = 12;
    for (int i = 0; i < set->count; i++) {
        CS_RequirementSetItem *item = (CS_RequirementSetItem *) (base + item_offset);
        SWAP_INT2(item->offset);
        SWAP_INT2(item->type);

        /**
         *  Fetch the data at base + item->offset. The CS_Requirement structure is not fully
         *  worked out (by me at least). The first four bytes are the magic number, followed
         *  by the size, three unknown values, the length of the string, and then the actual
         *  string.
         * 
         *  NOTE: It might be worth verifying the magic values throughout this function, since
         *        we're loading values from offsets.
         */
        CS_Requirement *string = (CS_Requirement *) (base + item->offset);
        SWAP_INT2(string->len);
        SWAP_INT2(string->size);

        char *str = malloc (string->len);
        memcpy (str, ((base + item->offset) + 24), string->len);

        char *type = "";
        switch (item->type) {
            case kRequirementTypeHost:
                type = "Host";
                break;
            case kRequirementTypeGuest:
                type = "Guest";
                break;
            case kRequirementTypeDesignated:
                type = "Designated";
                break;
            case kRequirementTypeLibrary:
                type = "Library";
                break;
            case kRequirementTypePlugIn:
                type = "Plug-In";
                break;
            case kRequirementTypeUnknown:
            default:
                type = "(unknown)";
                break;
        }

        printf ("\t%d: " DARK_GREY "%s Requirement (@0x%x, %d bytes): " BOLD DARK_WHITE "Identity: " RESET "%s, %s\n", i, type, item->offset, string->size, str, "n/a");
    }

}

htool_return_t
htool_code_signature_print_entitlements_der (void *raw, uint32_t offset, int verbose)
{
    const char *base = (const char *) (raw + offset);

    /**
     *  Libhelper does have a ASN.1/DER Decoder, however I don't see much of a point
     *  in decoding this, we'll just decode the XML one instead - it's the same data. 
     */
    uint32_t size = SWAP_INT(*(uint32_t *) (base + 4));
    printf (RESET " Entitlements (DER-Encoded, %d bytes)\n", size);
}

htool_return_t
htool_code_signature_print_entitlements (void *raw, uint32_t offset, int verbose)
{
    const char *base = (const char *) (raw + offset);

    /**
     *  To reduce this commnad being too messy, the entitlements XML can be printed
     *  using another option, --sig.  
     */
    uint32_t size = SWAP_INT(*(uint32_t *) (base + 4));
    printf (RESET " Entitlements (XML, %d bytes)\n", size);
    printf (BLUE "\tUse --sig to view XML-encoded Entitlements information\n" RESET);
}