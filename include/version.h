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

#ifdef __cplusplus
extern "C" {
#endif

#define HTOOL_VERSION_NUMBER "2.0.0"
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
