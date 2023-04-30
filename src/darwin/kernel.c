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

#include "darwin/kernel.h"

/***********************************************************************
* XNU Cache version, build style, and other properties.
***********************************************************************/

xnu_kernel_type_t 
xnu_kernel_fetch_type (xnu_t *xnu)
{
    mach_load_command_info_t    *lc_info;
    mach_build_version_info_t   *build;
    mach_segment_info_t         *seg_inf;
    macho_t                     *tmp_macho;

    /**
     *  The Fileset Mach-O's do not have the LC_BUILD_VERSION load command properly
     *  populated, so if the given kernel is an MH_FILESET, we need to first extract
     *  the "com.apple.kernel" entry Mach-O, from which we can determine build platform
     *  and architecture.
     * 
     *  The Fileset kernels do not have the KEXTs embedded in the kernel image, instead
     *  they are individual fileset entries in the master Mach-O. 
     */
    tmp_macho = xnu->macho;
    if (xnu->macho->header->filetype == MACH_TYPE_FILESET) {
        for (int i = 0; i < h_slist_length (xnu->macho->fileset); i++) {
            mach_fileset_entry_info_t *info = (mach_fileset_entry_info_t *) h_slist_nth_data (xnu->macho->fileset, i);
            char *entry_name = mach_load_command_load_string (xnu->macho, info->cmd->cmdsize, 
                sizeof (mach_fileset_entry_command_t), info->offset, info->cmd->entry_id.offset);

            if (!strcmp (entry_name, "com.apple.kernel")) {
                xnu->kern = info->macho;
                xnu->flags |= HTOOL_XNU_FLAG_FILESET_ENTRY;
                tmp_macho = xnu->kern;
                break;
            }
        }
    }

    /**
     *  We need to determine the platform type for the Kernel. This
     *  is specified in the LC_BUILD_VERSION load command, so we can fetch that
     *  from the Mach-O and check the platform name.
     * 
     *  However, if there is no build command, we may have a pre-iOS12 kernel,
     *  which can be verified by checking if the __BOOTDATA segment exists, as
     *  pre-iOS12 kernels do not have a __BOOTDATA segment.
     */
    lc_info = mach_load_command_find_command_by_type (tmp_macho, LC_BUILD_VERSION);
    if (!lc_info) goto kernel_ios_detect;

    /**
     *  We now have a pointer to the actual kernel binary mach-o which we can now use to
     *  determine the platform/SoC that the kernel is for. This is not entirely reliable
     *  as, on Mac's, the Pro/Max/Ultra SoC's use the same kernel image, usually showing
     *  the SoC as the Mn Pro. Afaik, there's nothing yet that can be done to determine
     *  which one it is.
     */
    build = mach_load_command_build_version_info ((mach_build_version_command_t *) lc_info->lc, lc_info->offset, tmp_macho);
    if (!strcmp (build->platform, "macOS")) {

        /* Check if the kernel is x86 */
        if (tmp_macho->header->cputype == CPU_TYPE_X86_64)
            return XNU_KERNEL_TYPE_MACOS_X86_64;

        /**
         *  The DTK does not, afaik, receive any support from newer versions of macOS, however
         *  it is probably useful to keep some sort of detection for it in HTool. It appears
         *  that DTK kernels are the only ones to have a __PPLTRAMP segment, so we can use that
         *  to detect it.
         */
        mach_section_64_t *__PPLTRAMP__text = mach_section_64_search (tmp_macho->scmds, "__PPLTRAMP", "__text");
        if (__PPLTRAMP__text) return XNU_KERNEL_TYPE_MACOS_ARM64_DTK;
        else return XNU_KERNEL_TYPE_MACOS_ARM64;
    } else {

kernel_ios_detect:
        /**
         *  We need to return the type of the Mach-O passed to HTool, not that of the kernel
         *  fileset entry. The MH_FILESET Mach-O will have a __TEXT segment with no sections,
         *  so if that's the case, and the correct xnu->flag is set, it's probably a Fileset
         *  image.
         */
        seg_inf = mach_segment_info_search (xnu->macho->scmds, "__TEXT");
        mach_segment_command_64_t *__TEXT = (mach_segment_command_64_t *) seg_inf->segcmd;
        if (HTOOL_CLIENT_CHECK_FLAG(xnu->flags, HTOOL_XNU_FLAG_FILESET_ENTRY) && __TEXT->nsects == 0)
            return XNU_KERNEL_TYPE_IOS_FILESET;

        /**
         *  Next is to check if the kernel is extracted from an MH_FILESET Mach-O. This can
         *  be done by checking the size of the __PRELINK_INFO segment. As of iOS 16, the
         *  com.apple.kernel fileset will still have this segment, but there won't be anything
         *  in it, whereas with other kernel formats this segment contains information regarding
         *  the location of all the embedded KEXT Mach-O's.
         */
        seg_inf = mach_segment_info_search (tmp_macho->scmds, "__PRELINK_INFO");
        mach_segment_command_64_t *__PRELINK_INFO = (mach_segment_command_64_t *) seg_inf->segcmd;
        if (__PRELINK_INFO)
            if (__PRELINK_INFO->vmsize == 0)
                return XNU_KERNEL_TYPE_IOS_FILESET_EXTRACTED;

        /**
         *  The split-style kernels, from iOS 10 - 11, should contain a __PLK_TEXT_EXEC.__text
         *  segment with a size larger than 0, so if we can detect that it's likely this is an
         *  iOS 10-11 split-style kernel.
         */
        mach_section_64_t *__PLK_TEXT_EXEC__text = mach_section_64_search (tmp_macho->scmds, "__PLK_TEXT_EXEC", "__text");
        if (__PLK_TEXT_EXEC__text)
            if (__PLK_TEXT_EXEC__text->size)
                return XNU_KERNEL_TYPE_IOS_SPLIT;

        /**
         *  The merged-style kernels merge all the KEXT __TEXT sections together, the __DATA
         *  sections together, etc, and are then mapped out in the __PRELINK_INFO.__kmod_info
         *  section, so if this exists then we have an iOS 12-15 merged-style kernel.
         */
        mach_section_64_t *__PRELINK_INFO__kmod_info = mach_section_64_search (tmp_macho->scmds, "__PRELINK_INFO", "__kmod_info");
        if (__PRELINK_INFO__kmod_info)
            return XNU_KERNEL_TYPE_IOS_MERGED;

        /**
         *  If the Mach-O has an __TEXT_EXEC segment, then we have an iOS 9 kernel. iOS 9 kernels
         *  are the only ones which have the __TEXT_EXEC segments.
         */
        mach_segment_info_t *__TEXT_EXEC = mach_segment_info_search (tmp_macho->scmds, "__TEXT_EXEC");
        if (!__TEXT_EXEC)
            return XNU_KERNEL_TYPE_IOS_IOS9;

        /* probably either an odd beta format, or a new style all-together */
        return XNU_KERNEL_TYPE_UNKNOWN_IOS;
    }

    return XNU_KERNEL_TYPE_UNKNOWN_PLATFORM;
}

char *
xnu_kernel_type_get_string (xnu_kernel_type_t type)
{
    switch (type) {

        /* iOS Kernel Types */
        case XNU_KERNEL_TYPE_IOS_IOS9:
            return "Legacy (iOS 9)";
        case XNU_KERNEL_TYPE_IOS_SPLIT:
            return "Split (iOS 10-11)";
        case XNU_KERNEL_TYPE_IOS_MERGED:
            return "Merged (iOS 12-15)";
        case XNU_KERNEL_TYPE_IOS_FILESET:
            return "Fileset (iOS 16)";
        case XNU_KERNEL_TYPE_IOS_FILESET_EXTRACTED:
            return "Fileset Extracted (iOS 16)";
            
        /* macOS Kernel Types */
        case XNU_KERNEL_TYPE_MACOS_X86_64:
            return "Intel-Genuine (x86_64)";
        case XNU_KERNEL_TYPE_MACOS_ARM64:
            return "Apple-Silicon (arm64)";
        case XNU_KERNEL_TYPE_MACOS_ARM64_DTK:
            return "Developer-Transition-Kit (arm64)";

        /* Unknown Kernel Types */
        case XNU_KERNEL_TYPE_UNKNOWN_PLATFORM:
            return "Unknown-Platform";
        case XNU_KERNEL_TYPE_UNKNOWN_IOS:
            return "Unknown-iOS";
        case XNU_KERNEL_TYPE_UNKNOWN_MACOS:
            return "Unknown-macOS";
    }
}

///////////////////////////////////////////////////////////////////////////////

char *
xnu_kernel_soc_string (char *buffer)
{
    darwin_device_t *dev;
    HSList *matches = NULL;

    /* Iterate through the entire device database until a platform match */
    for (int i = 0; i < DARWIN_DEVICE_LIST_LEN; i++) {
        dev = &device_list[i];
        if (!strcmp (buffer, dev->platform)) {
            matches = h_slist_append (matches, dev);
        }
    }

    darwin_device_t *ret = (darwin_device_t *) h_slist_nth_data (matches, 0);
    return ret->platform;
}

///////////////////////////////////////////////////////////////////////////////

char *
xnu_search_needle_haystack (unsigned char *needle, uint32_t needle_len, unsigned char *haystack, uint32_t size, char *split, uint32_t adjust)
{
    char *result;
    char *ret;
    
    result = (char *) bh_memmem (haystack, size, needle, needle_len) + adjust;
    if (!result) return NULL;

    if (split) {
        if ((ret = (char *) strsplit (result, split)->ptrs[0]))
            return ret;
    }
    
    return result;
}

char *
xnu_find_uname_string (unsigned char *data, uint32_t size)
{
    char *needle = "Darwin Kernel Version ";
    return xnu_search_needle_haystack (needle, strlen (needle), data, size, NULL, 0);   
}

char *
xnu_find_darwin_version (unsigned char *data, uint32_t size)
{
    char *needle = "Version ";
    return xnu_search_needle_haystack (needle, 6, data, size, ":", 8);
}

char *
xnu_find_xnu_version (unsigned char *data, uint32_t size)
{
    char *needle = "; root: ";
    return xnu_search_needle_haystack (needle, 6, data, size, "/", 7);
}

char *
xnu_find_build_time (unsigned char *data, uint32_t size)
{
    char *needle = xnu_find_darwin_version (data, size);
    return xnu_search_needle_haystack (needle, 6, data, size, ";", 8);
}

char *
xnu_find_device_type (unsigned char *data, uint32_t size)
{
    char *device, *brand, *ret;

    /* Grab the part of the string after "ARM64_" */
    device = (char *) strstr ((char *) data, "ARM64_");

    /**
     *  If the device does not have a value, it's possible this is an x86
     *  kernel, and therefore would use the x86_64 tag.
     */
    if (!device) {
        if ((char *) strstr ((char *) data, "X86_64"))
            return "x86_64";
    }

    /* Load the cpu brand string from the device string */
    brand = xnu_kernel_soc_string (device + 6);

    /* Make sure the brand has a value, return NULL otherwise */
    if (!brand) return NULL;

    //debug
    ret = darwin_get_device_from_string (brand);
    return (ret) ? ret : NULL;
}  

///////////////////////////////////////////////////////////////////////////////

void xnu_version_info_print (xnu_t *xnu, char *padding)
{
    xnu_version_t *version = xnu->version;
    printf ( BOLD DARK_WHITE "%sDarwin Version:  " RESET DARK_GREY "%s\n" RESET, padding, version->darwin_vers);
    printf ( BOLD DARK_WHITE "%sXNU Version:     " RESET DARK_GREY "%s\n" RESET, padding, version->xnu_vers);
    printf ( BOLD DARK_WHITE "%sCompile Time:    " RESET DARK_GREY "%s\n" RESET, padding, version->build_time);
    printf ( BOLD DARK_WHITE "%sDevice Type:     " RESET DARK_GREY "%s\n" RESET, padding, version->device_type);
    printf ( BOLD DARK_WHITE "%sXNU Type:        " RESET DARK_GREY "%s\n" RESET, padding, version->cache_style);

    if (xnu->flags & HTOOL_XNU_FLAG_FILESET_ENTRY)
        printf ( BOLD DARK_WHITE "%sFileset:         " RESET DARK_GREY "Yes\n" RESET, padding);
}

xnu_t *
xnu_kernel_load_kernel_cache (htool_binary_t *bin)
{
    xnu_t *xnu;

    /**
     *  Currently, there isn't a kernel that ships in a FAT format, so we can
     *  just assume that there is only one macho_t in the binary.
     */
    xnu = calloc (1, sizeof (xnu_t));
    xnu->macho = (macho_t *) h_slist_nth_data (bin->macho_list, 0);
    xnu->type = xnu_kernel_fetch_type (xnu);

    /**
     *  Fetch the `uname` string from the provided kernel binary as this gives us
     *  most of the information needed to determine a range of properties of the
     *  kernel.
     */    
    unsigned char *uname = (unsigned char *) xnu_find_uname_string (bin->data, bin->size);
    uint32_t u_len = strlen ((char *) uname);

    /**
     *  Find the version of the given kernel mach-o.
     */
    xnu_version_t *version = calloc (1, sizeof (xnu_version_t));
    version->darwin_vers = xnu_find_darwin_version (uname, u_len);
    version->xnu_vers = xnu_find_xnu_version (uname, u_len);
    version->build_time = xnu_find_build_time (uname, u_len);
    version->device_type = xnu_find_device_type (uname, u_len);
    version->cache_style = xnu_kernel_type_get_string (xnu->type);

    xnu->version = version;
    printf (ANSI_COLOR_GREEN "[*]" RESET ANSI_COLOR_GREEN " Detected Kernelcache\n" RESET);
    xnu_version_info_print (xnu, "   ");

    return xnu;
}