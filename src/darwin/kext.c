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
#include "darwin/kext.h"
#include "commands/macho.h"

#ifndef __APPLE__
/* Temporary, should move to libhelper */
char *
strnstr(const char *s, const char *find, size_t slen)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if (slen-- < 1 || (sc = *s++) == '\0')
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}
#endif

static macho_t *
_xnu_select_macho (xnu_t *xnu)
{
    macho_t *macho;
    if (xnu->flags & HTOOL_XNU_FLAG_FILESET_ENTRY) macho = xnu->kern;
    else macho = xnu->macho;
    return macho;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  These functions deal with parsing each individual KEXT from whatever
 *  mapping into a kext_t structure.
 */

kext_t *
xnu_parse_split_style_kext (macho_t *macho, char *load_addr_str, char *bundleid)
{
    macho_t *mem_macho;
    kext_t *kext ;
    uint64_t addr;

    /* Calculate KEXT load address */
    int rc = sscanf (load_addr_str, "%llx", &addr);
    if (!rc) {
        warningf ("[*] Unable to parse KEXT at load address: 0x%s\n", load_addr_str);
        return NULL;
    }

    /**
     *  For the Kernel's Mach-O, the segment addresses are kernel pointers, so they can be mapped
     *  into the correct area of memory. For example, an address for a segment within a kernel
     *  Mach-O would look like 0xffffff80044ec000. 
     * 
     *  This makes it annoying to find where these KEXTs are within the file. We already have the
     *  kernel virtual address of the KEXT, which is listed in the XML, and will look something
     *  like 0xffffff8004614000.
     * 
     *  To determine the "offset" of the KEXT within the file, we must first get the offset of the
     *  first segment within the kernel Mach-O. 
     */
    mach_segment_info_t *segment = (mach_segment_info_t *) mach_segment_info_search (macho->scmds, "__PRELINK_TEXT");
    mach_segment_command_64_t *seg64 = (mach_segment_command_64_t *) segment->segcmd;
    uint64_t base = seg64->fileoff;

    /**
     *  The offset of the KEXT can be calculate by taking the base offset, and subtracting the
     *  first segments virtual memory address from the virtual address of the kext. So we have
     *  something that looks like this:
     * 
     *      base            = 0x6a8000
     *      addr            = 0xfffffff0059f8180
     *      seg64->vmaddr   = 0xfffffff0059f8000
     * 
     *      offset          = (addr - seg64->vmaddr)
     *                      = 0xfffffff0059f8180 - 0xfffffff0059f8000
     *                      = 0x180
     * 
     *      KEXT            = macho->data + (base + offset)
     *                      = macho->data + (0x6a8180)
     * 
     */
    uint64_t offset = (addr - seg64->vmaddr);

    /**
     *  We can then calculate the pointer to the KEXT as macho->data + (base + offset), and
     *  then call macho_64_create_from_buffer() to create a macho_t.
     * 
     *  This isn't creating a copy of this data, it's just creating a struct that points to
     *  the correct area in the kernel Mach-O.
     */
    unsigned char *kext_data = (unsigned char *) (macho->data + (base + offset));
    mem_macho = macho_64_create_from_buffer (kext_data);

#if KEXT_DEBUG
    printf ("\n");
    printf ("          Bundle ID: %s\n", bundleid);
    printf ("  KEXT Load Address: 0x%llx\n", addr);
    printf ("Mach-O Segment Base: 0x%llx\n", base);
    printf (" Segment VM Address: 0x%llx\n", seg64->vmaddr);
    printf ("        KEXT Offset: 0x%llx\n", offset);
    printf ("          KEXT Size: %d bytes\n", mem_macho->size);
    printf ("       Final Offset: 0x%llx\n", (base + offset));
#endif

    /**
     *  Creating the KEXT struct
     */
    kext = calloc (1, sizeof (kext_t));
    memset (kext, '\0', sizeof (kext_t));

    kext->macho = mem_macho;
    kext->bundleid = bundleid;
    kext->offset = (base + offset);
    kext->vmaddr = addr;

    /* Find and set the source version of the KEXT */
    mach_source_version_command_t *svc = mach_load_command_find_source_version_command (kext->macho);
    if (svc) kext->version = mach_load_command_get_source_version_string (svc);
    else kext->version = "0.0.0";

    /* Find and set the UUID */
    mach_load_command_info_t *uuid_info = mach_load_command_find_command_by_type (kext->macho, LC_UUID);
    if (uuid_info) {
        mach_uuid_command_t *uuid = (mach_uuid_command_t *) uuid_info->lc;
        kext->uuid = mach_load_command_uuid_parse_string (uuid);
    } else {
        kext->uuid = "(null)";
    }

    /**
     *  NOTE:   There is a massive issue with the KEXTs. The segments aren't inline with the KEXTs
     *          Mach-O Header, the segments are "split" apart. Ideally, some logic needs to be
     *          implemented to reconstruct the Mach-O's, but libhelper doesn't yet support the
     *          ability to hand-craft a Mach-O. 
     */

#if KEXT_DEBUG
    printf ("       KEXT Version: %s\n", kext->version);
    printf ("          KEXT UUID: %s\n", kext->uuid);

    mach_header_t *hdr = (mach_header_t *) kext_data;
    char *magic_text;
    if (hdr->magic == MACH_CIGAM_64 || hdr->magic == MACH_MAGIC_64) magic_text = "Mach-O 64-bit";
    else magic_text = "Mach-O 32-bit";

    /* output header details */
    printf (BOLD DARK_WHITE "    Magic: " RESET DARK_GREY "0x%08x (%s)\n" RESET, hdr->magic, magic_text);
    printf (BOLD DARK_WHITE "     Type: " RESET DARK_GREY "%s\n" RESET, mach_header_get_file_type_string (hdr->filetype));
    printf (BOLD DARK_WHITE "      CPU: " RESET DARK_GREY "%s (%s)\n" RESET,
            mach_header_get_cpu_string (hdr->cputype, hdr->cpusubtype),
            mach_header_get_cpu_descriptor (hdr->cputype, hdr->cpusubtype));
    printf (BOLD DARK_WHITE "   ↳ Type: " RESET DARK_GREY "0x%08x, Subtype: 0x%08x\n" RESET, hdr->cputype, hdr->cpusubtype);
    printf (BOLD DARK_WHITE "  LoadCmd: " RESET DARK_GREY "%d\n" RESET, hdr->ncmds);
    printf (BOLD DARK_WHITE "  LC Size: " RESET DARK_GREY "%d Bytes\n" RESET, hdr->sizeofcmds);
#endif

    return kext;
}


///////////////////////////////////////////////////////////////////////////////
/**
 *  These functions deal with locating and reading the mapping for the kexts
 *  and calling the parser on each one so a list of kernel extensions can be
 *  created.
 */

htool_return_t
xnu_load_kext_list_ios9_style (xnu_t *xnu)
{
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
xnu_load_kext_list_merged_style (xnu_t *xnu)
{
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
xnu_load_kext_list_split_style (xnu_t *xnu)
{
    macho_t *macho = _xnu_select_macho (xnu);

    mach_segment_command_64_t *prelink_info_segment;
    unsigned char *xml, *xml_PrelinkExecutableLoa_str;
    int k_count = 0, k_failed = 0;
    HSList *kext_list = NULL;

    mach_segment_info_t *base_segment = (mach_segment_info_t *) h_slist_nth_data (macho->scmds, 0);
    mach_section_64_t *base_section = (mach_section_64_t *) base_segment->segcmd;
    uint64_t base_address = base_section->addr;

    /**
     *  The __PRELINK_INFO segment contains an XML that maps the Kernel Extensions in the
     *  Mach-O. If we search for this segment and then check it's valid before trying to
     *  parse the XML inside it.
     * 
     *  This is a bit of a crappy way to parse XML, but it saves writing an XML parsing
     *  or adding another library. This should be quite quick.
     * 
     */
    prelink_info_segment = mach_segment_command_64_from_info (mach_segment_info_search (macho->scmds, "__PRELINK_INFO"));
    if (prelink_info_segment) {
        printf ("[*] Found Segment: __PRELINK_INFO\n");

        /**
         *  Copy the contents of the __PRELINK_INFO segment into the xml string.
         */
        xml = malloc (prelink_info_segment->filesize);
        memcpy (xml, macho->data + prelink_info_segment->fileoff, prelink_info_segment->filesize);

        /**
         *  Check that the "PrelinkExecutableLoa" string exists in the XML. This is the
         *  start of the dictionary that contains the KEXT mapping.
         */
        xml_PrelinkExecutableLoa_str = strstr (xml, "PrelinkExecutableLoa");
        if (xml_PrelinkExecutableLoa_str) {

            char kext_name[2560], load_addr[24];
            unsigned char *load_addr_ptr, *kext_name_ptr;
            unsigned char *prelink_addr;

            /* Look for the first CFBundleName key */
            kext_name_ptr = strstr (xml, "CFBundleName</key>");

            /**
             *  While the kext_name_ptr still has a value, keep trying to advance to the
             *  next occurrence of CFBundleName.
             */
            while (kext_name_ptr) {

                /* Reset the xml pointer to the first occurrence of </string> */
                xml = strstr (kext_name_ptr, "</string>");

                /* Set the prelink_addr to the _PrelinkExecutableLoadAddr */
                prelink_addr = strstr (kext_name_ptr, "_PrelinkExecutableLoadAddr");
                if (!prelink_addr) {
                    warningf ("[*] Cannot determine Kernel Extension Load Address (%d)\n", k_count);
                    continue;
                }

                /* Find the start of the KEXT load address */
                load_addr_ptr = strstr (prelink_addr, "0x");

                /* Initialise 256 bytes in the kext_name to avoid something going wrong */
                memset (kext_name, '\0', 256);

                /* Fix for "ID=" */
                unsigned char *idFix = NULL;
                if (!(idFix = strnstr (kext_name_ptr, "ID=\"", xml - kext_name_ptr)))
                    idFix = kext_name_ptr;
                else
                    idFix = strstr (idFix + 5, "\">") + 2;

                /* Copy idFix to kext_name */
                strncpy (kext_name, idFix, xml - idFix);

                /* Set and copy bytes */
                memset (load_addr, '\0', 24);
                strncpy (load_addr, load_addr_ptr, 18);

                /* Advance the xml past the kext */
                xml += 10;

                /* Look for the next CFBundleIdentifier */
                kext_name_ptr = strstr (xml, "CFBundleIdentifier");
                if (kext_name_ptr) {

                    /* Look for the end of the string */
                    xml = strstr (kext_name_ptr, "</string>");
                    memset (kext_name, '\0', 256);

                    /* Reset the idFix */
                    if (!(idFix = strnstr (kext_name_ptr, "ID=\"", xml - kext_name_ptr)))
                        idFix = kext_name_ptr + 32;
                    else
                        idFix = strstr (idFix + 5, "\">") + 2;

                    /* Copy some bytes to the kext name */
                    strncpy (kext_name, idFix, xml - idFix);
                }

                /* Parse the individual kext and add it to the list */
                kext_t *kext = xnu_parse_split_style_kext (macho, load_addr, kext_name);
                kext_list = h_slist_append (kext_list, kext);
                
                /* Look for the next CFBundleName */
                kext_name_ptr = strstr (xml, "CFBundleName</key>");
                k_count++;
            }
            printf (ANSI_COLOR_GREEN "[*] Successfully parsed Kernel Extensions (%d)\n" RESET, h_slist_length (kext_list));
        }
    }
    return HTOOL_RETURN_SUCCESS;
}

htool_return_t
xnu_load_kext_list_fileset_style (xnu_t *xnu)
{
    debugf ("fileset size: %d\n", h_slist_length(xnu->macho->fileset));
    for (int i = 0; i < h_slist_length (xnu->macho->fileset); i++) {
        mach_fileset_entry_info_t *entry = (mach_fileset_entry_info_t *) h_slist_nth_data (xnu->macho->fileset, i);
        
        kext_t *kext = calloc (1, sizeof (kext_t));
        kext->macho = entry->macho;
        kext->bundleid = entry->entry_id;

        printf ("\n--------\nKEXT Bundle ID: %s (%d bytes)\n", kext->bundleid, kext->macho->size);
        htool_print_macho_header_from_struct (kext->macho->header);

        FILE *fp = fopen (kext->bundleid, "w+");
        fwrite (kext->macho->data, kext->macho->size, 1, fp);
        fclose (fp);
    }

    return HTOOL_RETURN_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

htool_return_t
xnu_parse_kernel_extensions (xnu_t *xnu)
{
    debugf ("parsing kernel extensions\n");
    macho_t *kern = xnu->macho;

    if (xnu->type == XNU_KERNEL_TYPE_IOS_IOS9) {
        printf ("XNU_KERNEL_TYPE_IOS_IOS9\n");
        xnu_load_kext_list_split_style (xnu);
    } else if (xnu->type == XNU_KERNEL_TYPE_IOS_SPLIT) {
        printf ("XNU_KERNEL_TYPE_IOS_SPLIT\n");
        xnu_load_kext_list_split_style (xnu);
    } else if (xnu->type == XNU_KERNEL_TYPE_IOS_MERGED) {
        printf ("XNU_KERNEL_TYPE_IOS_MERGED\n");
    } else if (xnu->type == XNU_KERNEL_TYPE_IOS_FILESET) {
        printf ("XNU_KERNEL_TYPE_IOS_FILESET\n");
        xnu_load_kext_list_fileset_style (xnu);
    } else {
        errorf ("ded\n");
        return HTOOL_RETURN_FAILURE;
    }


    return HTOOL_RETURN_SUCCESS;
}
