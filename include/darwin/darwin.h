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

/**
 *  NOTE:   This header file contains all the definitions needed to detect and 
 *          handle different versions of Darwin XNU firmware files up to the latest
 *          iOS version as-of March-ish 2023.   
 * 
 */

#ifndef __HTOOL_DARWIN_H__
#define __HTOOL_DARWIN_H__

#include <libhelper.h>

#include "htool-client.h"
#include "htool-loader.h"
#include "htool.h"


/***********************************************************************
* Apple Device Information.
*
*   All 64-bit devices are basically defined here. The aim is that either
*   the SoC (e.g. T8010), the board identifier (D10AP) or the Apple Chip
*   Brand name (e.g. A14 Bionic).
*
***********************************************************************/

/**
 *  Rather than defining "unknown" three times, define a single one here.
 */
#define DARWIN_PROPERTY_UNKNOWN             "Unknown"


/**
 *  All 64-bit iDevice Platform identifiers, e.g. T8101, are defined here.
 *  They are separated between device type, with older chips higher up the
 *  list.
 *
 */
// A-Series chips
#define DARWIN_PLATFORM_S5L8960             "S5L8960"           // Apple A7 (APL0698)
#define DARWIN_PLATFORM_S5L8965             "S5L8965"           // Apple A7 (APL5698)
#define DARWIN_PLATFORM_T7000               "T7000"
#define DARWIN_PLATFORM_T7001               "T7001"
#define DARWIN_PLATFORM_S8000               "S8000"
#define DARWIN_PLATFORM_S8003               "S8003"
#define DARWIN_PLATFORM_S8001               "S8001"
#define DARWIN_PLATFORM_T8010               "T8010"
#define DARWIN_PLATFORM_T8011               "T8011"
#define DARWIN_PLATFORM_T8015               "T8015"
#define DARWIN_PLATFORM_T8020               "T8020"             // Appears in DTK Kernel
#define DARWIN_PLATFORM_T8027               "T8027"
#define DARWIN_PLATFORM_T8030               "T8030"
#define DARWIN_PLATFORM_T8101               "T8101"             // Occasionally appears in M1 kernels
#define DARWIN_PLATFORM_T8110               "T8110"
#define DARWIN_PLATFORM_T8120               "T8120"

// M-Series chips
#define DARWIN_PLATFORM_T8103               "T8103"
#define DARWIN_PLATFORM_T6000               "T6000"
#define DARWIN_PLATFORM_T6001               "T6001"
#define DARWIN_PLATFORM_T6002               "T6002"
#define DARWIN_PLATFORM_T6021               "T6021"
#define DARWIN_PLATFORM_T6020               "T6020"
#define DARWIN_PLATFORM_T8112               "T8112"
#define DARWIN_PLATFORM_VMAPPLE2            "vmapple2"          // macOS Virtual Machine

// S- & T-Series chips
#define DARWIN_PLATFORM_S7002               "S7002"
#define DARWIN_PLATFORM_T8002               "T8002"
#define DARWIN_PLATFORM_T8004               "T8004"
#define DARWIN_PLATFORM_T8006               "T8006"
#define DARWIN_PLATFORM_T8301               "T8301"
#define DARWIN_PLATFORM_T8012               "T8012"

// H-Serise chips
#define DARWIN_PLATFORM_T2002               "T2002"
#define DARWIN_PLATFORM_T2016               "T2016"

/*
 *  All Board Identiifers for all iOS-based devices. #defines are grouped with
 *  other device launches at that time, with iPhone's, iPad's, iPod's, HomePod's,
 *  Apple Watch's, iBridge's etc grouped separately. These are listed from new to
 *  old.
 *
 */
#define DARWIN_BOARD_D74AP                  "D74AP"             // iPhone 14 Pro Max
#define DARWIN_BOARD_D73AP                  "D73AP"             // iPhone 14 Pro
#define DARWIN_BOARD_D28AP                  "D28AP"             // iPhone 14 Plus
#define DARWIN_BOARD_D27AP                  "D27AP"             // iPhone 14
#define DARWIN_BOARD_D49AP                  "D49AP"             // iPhone SE (3rd Generation)
#define DARWIN_BOARD_D64AP                  "D64AP"             // iPhone 13 Pro Max
#define DARWIN_BOARD_D63AP                  "D63AP"             // iPhone 13 Pro
#define DARWIN_BOARD_D17AP                  "D17AP"             // iPhone 13
#define DARWIN_BOARD_D16AP                  "D16AP"             // iPhone 13 Mini

#define DARWIN_BOARD_D54PAP                 "D54PAP"            // iPhone 12 Pro Max
#define DARWIN_BOARD_D53PAP                 "D53PAP"            // iPhone 12 Pro
#define DARWIN_BOARD_D53GAP                 "D53GAP"            // iPhone 12
#define DARWIN_BOARD_D52GAP                 "D52GAP"            // iPhone 12 Mini

#define DARWIN_BOARD_D79AP                  "D79AP"             // iPhone SE (2nd Generation) 2020

#define DARWIN_BOARD_D431AP                 "D431AP"            // iPhone 11 Pro Max
#define DARWIN_BOARD_D421AP                 "D421AP"            // iPhone 11 Pro
#define DARWIN_BOARD_N104AP                 "N104AP"            // iPhone 11

#define DARWIN_BOARD_D331AP                 "D331AP"            // iPhone XS Max (iPhone11,4)
#define DARWIN_BOARD_D331PAP                "D331PAP"           // iPhone XS Max (iPhone11,6)
#define DARWIN_BOARD_D321AP                 "D321AP"            // iPhone XS
#define DARWIN_BOARD_N841AP                 "N841AP"            // iPhone XR

#define DARWIN_BOARD_D221AP                 "D221AP"            // iPhone X (iPhone10,6)
#define DARWIN_BOARD_D22AP                  "D22AP"             // iPhone X (iPhone10,3)
#define DARWIN_BOARD_D21AP                  "D21AP"             // iPhone 8 Plus (Global) (iPhone10,2)
#define DARWIN_BOARD_D211AP                 "D211AP"            // iPhone 8 Plus (GSM) (iPhone10,5)
#define DARWIN_BOARD_D201AP                 "D201AP"            // iPhone 8 (GSM) (iPhone10,4)
#define DARWIN_BOARD_D20AP                  "D20AP"             // iPhone 8 (Global) (iPhone10,1)

#define DARWIN_BOARD_D111AP                 "D111AP"            // iPhone 7 Plus (iPhone9,4)
#define DARWIN_BOARD_D11AP                  "D11AP"             // iPhone 7 Plus (iPhone9,2)
#define DARWIN_BOARD_D101AP                 "D101AP"            // iPhone 7 (iPhone9,3)
#define DARWIN_BOARD_D10AP                  "D10AP"             // iPhone 7 (iPhone9,1)

#define DARWIN_BOARD_N69UAP                 "N69UAP"            // iPhone SE (1st Generation) (Samsung)
#define DARWIN_BOARD_N69AP                  "N69AP"             // iPhone SE (1st Generation) (TSMC)

#define DARWIN_BOARD_N66AP                  "N66AP"             // iPhone 6s Plus (Samsung)
#define DARWIN_BOARD_N66MAP                 "N66MAP"            // iPhone 6s Plus (TSMC)
#define DARWIN_BOARD_N71AP                  "N71AP"             // iPhone 6s (Samsung)
#define DARWIN_BOARD_N71MAP                 "N71MAP"            // iPhone 6s (TSMC)

#define DARWIN_BOARD_N56AP                  "N56AP"             // iPhone 6 Plus
#define DARWIN_BOARD_N61AP                  "N61AP"             // iPhone 6

#define DARWIN_BOARD_N53AP                  "N53AP"             // iPhone 5s
#define DARWIN_BOARD_N51AP                  "N51AP"             // iPhone 5s


/**
 *  All board identifiers for all macOS devices. Again, #defines are grouped with
 *  other products released at the same time.
 */
#define DARWIN_BOARD_MACOS_J273AAP          "J273AAP"           // Developer Transition Kit (DTK) (A12Z)

#define DARWIN_BOARD_MACOS_J313AP           "J313AP"            // Macbook Air (Late 2020) (M1)
#define DARWIN_BOARD_MACOS_J293AP           "J293AP"            // Macbook Pro (Late 2020) (M1)
#define DARWIN_BOARD_MACOS_J274AP           "J274AP"            // Mac Mini (Late 2020) (M1)
#define DARWIN_BOARD_MACOS_J456AP           "J456AP"            // iMac 24-inch (Two Ports) (2021) (M1)
#define DARWIN_BOARD_MACOS_J457AP           "J457AP"            // iMac 24-inch (Four Ports) (2021) (M1)

#define DARWIN_BOARD_MACOS_J314CAP          "J314cAP"           // Macbook Pro 14-inch (2021) (M1 Max)
#define DARWIN_BOARD_MACOS_J316CAP          "J316cAP"           // Macbook Pro 16-inch (2021) (M1 Max)
#define DARWIN_BOARD_MACOS_J314SAP          "J314sAP"           // Macbook Pro 14-inch (2021) (M1 Pro)
#define DARWIN_BOARD_MACOS_J316SAP          "J316sAP"           // Macbook Pro 16-inch (2021) (M1 Pro)

#define DARWIN_BOARD_MACOS_J375CAP          "J375cAP"           // Mac Studio (M1 Max)
#define DARWIN_BOARD_MACOS_J375DAP          "J375dAP"           // Mac Studio (M1 Ultra)

#define DARWIN_BOARD_MACOS_VMA2MACOSAP      "VMA2MACOSAP"       // Apple Virtual Machine 1
#define DARWIN_BOARD_MACOS_J413AP           "J413AP"            // Macbook Air (2022) (M2)
#define DARWIN_BOARD_MACOS_J493AP           "J493AP"            // Macbook Pro 13-inch (2022) (M2)
#define DARWIN_BOARD_MACOS_J473AP           "J473AP"            // Mac Mini (2023) M2

#define DARWIN_BOARD_MACOS_J414CAP          "J414cAP"           // Macbook Pro 14-inch (2023) (M2 Max)
#define DARWIN_BOARD_MACOS_J416CAP          "J416cAP"           // Macbook Pro 16-inch (2023) (M2 Max)
#define DARWIN_BOARD_MACOS_J414SAP          "J414sAP"           // Macbook Pro 14-inch (2023) (M2 Pro)
#define DARWIN_BOARD_MACOS_J416SAP          "J416sAP"           // Macbook Pro 16-inch (2023) (M2 Pro)

#define DARWIN_BOARD_MACOS_J474SAP          "J474sAP"           // Mac Mini (2023) (M2 Pro)


/**
 *  Apple-Branded SoC (System-on-Chips).
 *
 */
#define DARWIN_SOC_GENERIC_ARM              "Generic Arm"

#define DARWIN_SOC_M2_MAX                   "M2 Max"
#define DARWIN_SOC_M2_PRO                   "M2 Pro"
#define DARWIN_SOC_M2                       "M2"

#define DARWIN_SOC_M1_ULTRA                 "M1 Ultra"
#define DARWIN_SOC_M1_MAX                   "M1 Max"
#define DARWIN_SOC_M1_PRO                   "M1 Pro"
#define DARWIN_SOC_M1                       "M1"

#define DARWIN_SOC_A12Z                     "A12Z"
#define DARWIN_SOC_A12X_BIONIC              "A12X Bionic"

#define DARWIN_SOC_A16_BIONIC               "A16 Bionic"
#define DARWIN_SOC_A15_BIONIC               "A15 Bionic"
#define DARWIN_SOC_A14_BIONIC               "A14 Bionic"
#define DARWIN_SOC_A13_BIONIC               "A13 Bionic"
#define DARWIN_SOC_A12_BIONIC               "A12 Bionic"
#define DARWIN_SOC_A11                      "A11"
#define DARWIN_SOC_A10                      "A10"
#define DARWIN_SOC_A9                       "A9"
#define DARWIN_SOC_A8                       "A8"
#define DARWIN_SOC_A7                       "A7"


/**
 *  Defines a Darwin-based device with an identifier, platform, board
 *  and soc. The only thing not included here is the marketing name for
 *  devices. E.g. we use iPhone13,4 instead of iPhone 12 Po Max.
 * 
 */
struct darwin_device_properties {
    char        *platform;
    char        *board;
    char        *soc;

    int          type;
    char        *identifier;
};
typedef struct darwin_device_properties     darwin_device_t;

#define DARWIN_DEVICE_TYPE_MOBILE           1
#define DARWIN_DEVICE_TYPE_MAC              2

/**
 *  List of all defined 64-bit iDevice's completely supported by HTool.
 *  macOS and other *OS devices are listed separately.
 * 
 */
static darwin_device_t device_list[] = 
{
    /* ==== MACOS DEVICES ==== */

    /* M2 Macbook Pro 2023 */
    { DARWIN_PLATFORM_T6021, DARWIN_BOARD_MACOS_J414CAP, DARWIN_SOC_M2_MAX, DARWIN_DEVICE_TYPE_MAC, "Mac14,5" },
    { DARWIN_PLATFORM_T6021, DARWIN_BOARD_MACOS_J416CAP, DARWIN_SOC_M2_MAX, DARWIN_DEVICE_TYPE_MAC, "Mac14,6" },
    { DARWIN_PLATFORM_T6020, DARWIN_BOARD_MACOS_J414SAP, DARWIN_SOC_M2_PRO, DARWIN_DEVICE_TYPE_MAC, "Mac14,9" },
    { DARWIN_PLATFORM_T6020, DARWIN_BOARD_MACOS_J416SAP, DARWIN_SOC_M2_PRO, DARWIN_DEVICE_TYPE_MAC, "Mac14,10" },

    /* M2 Mac mini 2023 */
    { DARWIN_PLATFORM_T6020, DARWIN_BOARD_MACOS_J473AP, DARWIN_SOC_M2_PRO, DARWIN_DEVICE_TYPE_MAC, "Mac14,12" },
    { DARWIN_PLATFORM_T8112, DARWIN_BOARD_MACOS_J473AP, DARWIN_SOC_M2, DARWIN_DEVICE_TYPE_MAC, "Mac14,3" },

    /* M2 Macbook Air & Macbook Pro 2022 */
    { DARWIN_PLATFORM_T8112, DARWIN_BOARD_MACOS_J493AP, DARWIN_SOC_M2, DARWIN_DEVICE_TYPE_MAC, "Mac14,7" },
    { DARWIN_PLATFORM_T8112, DARWIN_BOARD_MACOS_J413AP, DARWIN_SOC_M2, DARWIN_DEVICE_TYPE_MAC, "Mac14,2" },

    /* Apple Virtual Machine 1 */
    { DARWIN_PLATFORM_VMAPPLE2, DARWIN_BOARD_MACOS_VMA2MACOSAP, DARWIN_SOC_GENERIC_ARM, DARWIN_DEVICE_TYPE_MAC, "VirtualMac2,1" },

    /* M1 Mac Studio */
    { DARWIN_PLATFORM_T6001, DARWIN_BOARD_MACOS_J375CAP, DARWIN_SOC_M1_MAX, DARWIN_DEVICE_TYPE_MAC, "Mac13,1" },
    { DARWIN_PLATFORM_T6002, DARWIN_BOARD_MACOS_J375DAP, DARWIN_SOC_M1_ULTRA, DARWIN_DEVICE_TYPE_MAC, "Mac13,2" },

    /* M1 Macbook Pro 2021 */
    { DARWIN_PLATFORM_T6000, DARWIN_BOARD_MACOS_J316SAP, DARWIN_SOC_M1_PRO, DARWIN_DEVICE_TYPE_MAC, "MacBookPro18,1" },
    { DARWIN_PLATFORM_T6000, DARWIN_BOARD_MACOS_J314SAP, DARWIN_SOC_M1_PRO, DARWIN_DEVICE_TYPE_MAC, "MacBookPro18,3" },
    { DARWIN_PLATFORM_T6001, DARWIN_BOARD_MACOS_J316CAP, DARWIN_SOC_M1_MAX, DARWIN_DEVICE_TYPE_MAC, "MacBookPro18,2" },
    { DARWIN_PLATFORM_T6001, DARWIN_BOARD_MACOS_J314CAP, DARWIN_SOC_M1_MAX, DARWIN_DEVICE_TYPE_MAC, "MacBookPro18,1" },

    /* M1 iMac 2021 */
    { DARWIN_PLATFORM_T8103, DARWIN_BOARD_MACOS_J457AP, DARWIN_SOC_M1,    DARWIN_DEVICE_TYPE_MAC, "iMac21,2" },
    { DARWIN_PLATFORM_T8103, DARWIN_BOARD_MACOS_J456AP, DARWIN_SOC_M1,    DARWIN_DEVICE_TYPE_MAC, "iMac21,1" },

    /* M1 Mac Mini 2020 */
    { DARWIN_PLATFORM_T8103, DARWIN_BOARD_MACOS_J274AP,  DARWIN_SOC_M1,    DARWIN_DEVICE_TYPE_MAC, "MacMini9,1" },

    /* M1 Macbook Air & Macbook Pro 2020 */
    { DARWIN_PLATFORM_T8103, DARWIN_BOARD_MACOS_J313AP,  DARWIN_SOC_M1,   DARWIN_DEVICE_TYPE_MAC, "MacBookAir10,1" },
    { DARWIN_PLATFORM_T8103, DARWIN_BOARD_MACOS_J293AP,  DARWIN_SOC_M1,   DARWIN_DEVICE_TYPE_MAC, "MacBookPro17,1" },

    /* Developer Transition Kit */
    { DARWIN_PLATFORM_T8020, DARWIN_BOARD_MACOS_J313AP,  DARWIN_SOC_A12Z, DARWIN_DEVICE_TYPE_MAC, "ADP3,2" },

    /* ==== IOS DEVICES ==== */

    /* iPhone 14, 14 Plus, 14 Pro, 14 Pro Max */
    { DARWIN_PLATFORM_T8120, DARWIN_BOARD_D74AP,   DARWIN_SOC_A16_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone15,3 "},
    { DARWIN_PLATFORM_T8120, DARWIN_BOARD_D73AP,   DARWIN_SOC_A16_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone15,2" },
    { DARWIN_PLATFORM_T8120, DARWIN_BOARD_D28AP,   DARWIN_SOC_A16_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone14,8" },
    { DARWIN_PLATFORM_T8120, DARWIN_BOARD_D27AP,   DARWIN_SOC_A16_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone14,7 "},

    /* iPhone SE (3rd Generation) */
    { DARWIN_PLATFORM_T8110, DARWIN_BOARD_D49AP,   DARWIN_SOC_A15_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone14,6 "},

    /* iPhone 13 Pro, 13 Pro Max, 13 & 13 Mini */
    { DARWIN_PLATFORM_T8110, DARWIN_BOARD_D17AP,   DARWIN_SOC_A15_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone14,5" },
    { DARWIN_PLATFORM_T8110, DARWIN_BOARD_D16AP,   DARWIN_SOC_A15_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone14,4" },
    { DARWIN_PLATFORM_T8110, DARWIN_BOARD_D64AP,   DARWIN_SOC_A15_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone14,3" },
    { DARWIN_PLATFORM_T8110, DARWIN_BOARD_D63AP,   DARWIN_SOC_A15_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone14,2" },

    /* iPhone 12 Pro, 12 Pro Max, 12 & 12 Mini */
    { DARWIN_PLATFORM_T8101, DARWIN_BOARD_D54PAP,  DARWIN_SOC_A14_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone13,4" },
    { DARWIN_PLATFORM_T8101, DARWIN_BOARD_D53PAP,  DARWIN_SOC_A14_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone13,3" },
    { DARWIN_PLATFORM_T8101, DARWIN_BOARD_D53GAP,  DARWIN_SOC_A14_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone13,2" },
    { DARWIN_PLATFORM_T8101, DARWIN_BOARD_D52GAP,  DARWIN_SOC_A14_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone13,1" },

    /* iPhone SE (2nd Generation) 2020 */
    { DARWIN_PLATFORM_T8030, DARWIN_BOARD_D79AP,   DARWIN_SOC_A13_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone12,8" },

    /* iPhone 11 Pro Max, 11 Pro & 11 */
    { DARWIN_PLATFORM_T8030, DARWIN_BOARD_D431AP,  DARWIN_SOC_A13_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone12,5" },
    { DARWIN_PLATFORM_T8030, DARWIN_BOARD_D421AP,  DARWIN_SOC_A13_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone12,3" },
    { DARWIN_PLATFORM_T8030, DARWIN_BOARD_N104AP,  DARWIN_SOC_A13_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone12,1" },

    /* iPhone XS Max, XS & XR */
    { DARWIN_PLATFORM_T8020, DARWIN_BOARD_D331PAP, DARWIN_SOC_A12_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone11,6" },
    { DARWIN_PLATFORM_T8020, DARWIN_BOARD_D331AP,  DARWIN_SOC_A12_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone11,4" },
    { DARWIN_PLATFORM_T8020, DARWIN_BOARD_D321AP,  DARWIN_SOC_A12_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone11,2" },
    { DARWIN_PLATFORM_T8020, DARWIN_BOARD_N841AP,  DARWIN_SOC_A12_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone11,8" },

    /* iPhone X*/
    { DARWIN_PLATFORM_T8015, DARWIN_BOARD_D221AP,  DARWIN_SOC_A11, DARWIN_DEVICE_TYPE_MOBILE, "iPhone10,6" },
    { DARWIN_PLATFORM_T8015, DARWIN_BOARD_D22AP,   DARWIN_SOC_A11, DARWIN_DEVICE_TYPE_MOBILE, "iPhone10,3" },

    /* iPhone 8 Plus & 8 */
    { DARWIN_PLATFORM_T8015, DARWIN_BOARD_D211AP,  DARWIN_SOC_A11, DARWIN_DEVICE_TYPE_MOBILE, "iPhone10,2" },
    { DARWIN_PLATFORM_T8015, DARWIN_BOARD_D211AP,  DARWIN_SOC_A11, DARWIN_DEVICE_TYPE_MOBILE, "iPhone10,5" },
    { DARWIN_PLATFORM_T8015, DARWIN_BOARD_D201AP,  DARWIN_SOC_A11, DARWIN_DEVICE_TYPE_MOBILE, "iPhone10,4" },
    { DARWIN_PLATFORM_T8015, DARWIN_BOARD_D20AP,   DARWIN_SOC_A11, DARWIN_DEVICE_TYPE_MOBILE, "iPhone10,1" },

    /* iPhone 7 Plus & 7 */
    { DARWIN_PLATFORM_T8010, DARWIN_BOARD_D111AP,  DARWIN_SOC_A10, DARWIN_DEVICE_TYPE_MOBILE, "iPhone9,4" },
    { DARWIN_PLATFORM_T8010, DARWIN_BOARD_D11AP,   DARWIN_SOC_A10, DARWIN_DEVICE_TYPE_MOBILE, "iPhone9,2" },
    { DARWIN_PLATFORM_T8010, DARWIN_BOARD_D101AP,  DARWIN_SOC_A10, DARWIN_DEVICE_TYPE_MOBILE, "iPhone9,3" },
    { DARWIN_PLATFORM_T8010, DARWIN_BOARD_D10AP,   DARWIN_SOC_A10, DARWIN_DEVICE_TYPE_MOBILE, "iPhone9,1" },

    /* iPhone SE (1st Generation) */
    { DARWIN_PLATFORM_S8000, DARWIN_BOARD_N69UAP,  DARWIN_SOC_A9,  DARWIN_DEVICE_TYPE_MOBILE, "iPhone8,4" },
    { DARWIN_PLATFORM_S8003, DARWIN_BOARD_N69AP,   DARWIN_SOC_A9,  DARWIN_DEVICE_TYPE_MOBILE, "iPhone8,4" },

    /* iPhone 6s Plus & 6s */
    { DARWIN_PLATFORM_S8000, DARWIN_BOARD_N66AP,   DARWIN_SOC_A9,  DARWIN_DEVICE_TYPE_MOBILE, "iPhone8,2" },
    { DARWIN_PLATFORM_S8003, DARWIN_BOARD_N66MAP,  DARWIN_SOC_A9,  DARWIN_DEVICE_TYPE_MOBILE, "iPhone8,2" },
    { DARWIN_PLATFORM_S8000, DARWIN_BOARD_N71AP,   DARWIN_SOC_A9,  DARWIN_DEVICE_TYPE_MOBILE, "iPhone8,1" },
    { DARWIN_PLATFORM_S8003, DARWIN_BOARD_N71MAP,  DARWIN_SOC_A9,  DARWIN_DEVICE_TYPE_MOBILE, "iPhone8,1" },

    /* iPhone 6 Plus & 6 */
    { DARWIN_PLATFORM_T7000, DARWIN_BOARD_N56AP,  DARWIN_SOC_A8,  DARWIN_DEVICE_TYPE_MOBILE, "iPhone7,1" },
    { DARWIN_PLATFORM_T7000, DARWIN_BOARD_N61AP,  DARWIN_SOC_A8,  DARWIN_DEVICE_TYPE_MOBILE, "iPhone7,2" },

    /* iPhone 5s */
    { DARWIN_PLATFORM_S5L8960, DARWIN_BOARD_N53AP, DARWIN_SOC_A7, DARWIN_DEVICE_TYPE_MOBILE, "iPhone6,2" },
    { DARWIN_PLATFORM_S5L8960, DARWIN_BOARD_N51AP, DARWIN_SOC_A7, DARWIN_DEVICE_TYPE_MOBILE, "iPhone6,1" },

    { NULL }
};

#define DARWIN_DEVICE_LIST_LEN              (sizeof (device_list) / sizeof (device_list[0])) - 1
#define DOES_STRING_EXIST (a, b)               strstr(a, b) != NULL

/**
 *  Call with any string, and it will be matched with a defined device above. If this
 *  were to fail, the returned result would be "string (Unknown)", however if everything
 *  works, the result should be "A14 Bionic T8101 (D54PAP)".
 * 
 */
char *darwin_get_device_from_string (char *string);


/***********************************************************************
* Darwin Component Detection.
*
*   These functions handle detecting different Darwin components such
*   as the Kernel, iBoot, SecureROM and SEP.
***********************************************************************/

/**
 * 
 */
typedef enum darwin_component_type_t
{
    DARWIN_COMPONENT_TYPE_UNKNOWN,
    DARWIN_COMPONENT_TYPE_IBOOT,
    DARWIN_COMPONENT_TYPE_KERNEL,
    DARWIN_COMPONENT_TYPE_SEPOS,
    DARWIN_COMPONENT_TYPE_IMG4
} darwin_component_type_t;

/**
 * 
 */
htool_return_t
darwin_detect_firmware_component_kernel (htool_binary_t *bin);
htool_return_t
darwin_detect_firmware_component_iboot (htool_binary_t *bin);

#endif /* __htool_darwin_h__ */
