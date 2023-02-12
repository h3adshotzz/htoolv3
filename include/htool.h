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

#ifndef __HTOOL_VERSION_H__
#define __HTOOL_VERSION_H__

/**
 *  Exit codes
 */
typedef enum htool_return_t
{
    HTOOL_RETURN_FAILURE,
    HTOOL_RETURN_SUCCESS,
    HTOOL_RETURN_VOID
} htool_return_t;

/**
 * Error code
 */
#define HTOOL_ERROR_FILETYPE    0x000000001

/**
 * Colour codes
 */
#define WHITE           "\x1b[38;5;254m"
#define DARK_WHITE      "\x1b[38;5;251m"
#define DARK_GREY       "\x1b[38;5;243m"
#define YELLOW          "\x1b[38;5;214m"
#define DARK_YELLOW     "\x1b[38;5;94m"
#define RED             "\x1b[38;5;88m"
#define BLUE            "\x1b[38;5;32m"
#define RESET           "\x1b[0m"
#define BOLD            "\x1b[1m"


#define HTOOL_VERSION_NUMBER "3.0.0"
#define HTOOL_VERSION_TAG "DEVELOPMENT"

/**
 *  Define a Target type for HTool to both display within the help
 *  and other version information texts, and to determine platform
 *  when it comes to specific libraries that are only defined on
 *  one platform.
 *
 */
#ifdef __APPLE__
#   define BUILD_TARGET         "darwin"
#   define BUILD_TARGET_CAP     "Darwin"
#else
#   define BUILD_TARGET         "linux"
#   define BUILD_TARGET_CAP     "Linux"
#endif


/**
 *  Define an Architecture type for HTool to both display within
 *  the help and other version information texts, and to determine
 *  architecture when it comes to platform-specific operations.
 *
 */
#ifdef __x86_64__
#   define BUILD_ARCH           "x86_64"
#elif __arm__
#   define BUILD_ARCH           "arm"
#elif __arm64__
#	define BUILD_ARCH			"arm64"
#endif


/**
 *  HTool can run on Apple Silicon (ARM on Mac), however some features
 *  may not operate correctly between the two architectures (x86 & arm64).
 *  Define the macOS platform type, whether Apple Silicon or Intel.
 *
 */
#define HTOOL_PLATFORM_TYPE_APPLE_SILICON       1
#define HTOOL_PLATFORM_TYPE_INTEL_GENUINE       2

#if defined(__arm64__) && defined(__APPLE__)
#   define HTOOL_MACOS_PLATFORM_TYPE            HTOOL_PLATFORM_TYPE_APPLE_SILICON
#else
#   define HTOOL_MACOS_PLATFORM_TYPE            HTOOL_PLATFORM_TYPE_INTEL_GENUINE
#endif


#endif /* __htool_version_h__ */
