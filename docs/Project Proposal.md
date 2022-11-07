### Introduction

The proposed project is HTool v3.0.0. HTool is a static analysis tool for analysing iOS firmware files (Kernel, iBoot Bootloader, General Mach-O Files). HTool v1.0.0 was originally released in 2020 on my website and was written entirely by myself using an opensource library that is on my Github page, and v2.0.0 was a rewrite that was never developed further than implementing a Mach-O parser.

**Potential Project Titles**
- How can static analysis of Darwin-based OS Firmware Files be improved?
- Developing a static analysis tool, "HTool", for reverse engineering Apple OS Firmware Files.


**Objectives**
For v3.0.0, I have the following features in mind:
- HTool Binary Loader
- Mach-O & ELF Parser
- Kernelcache analysis
	- KEXT analysis & extract for all three kernelcache types
	- Kernel version & build information
	- Platform information
- macOS Kernel & KernelCollection analysis
- Device Database

Further features that could be implemented providing there is sufficient time:
- DeviceTree Parser
- iBoot analysis
- SEP/SEPOS analysis

I will write a brief overview of each feature in this document, as well as a more in-depth design document that will be stored in this repository under docs/

### HTool Binary Loader

The "HTool Binary Loader" is a concept for a library that provides a robust API for handling files of different formats. The idea is that this API provides a loader that can detect and parse different binary formats and provide a single C structure that can be passed around the program depending on the commands given by the user.

For example, the user runs htool to parse a kernelcache. The Loader API would be invoked to detect the file, create a `htool_binary_t` structure, parse the kernelcache, populate the struct and return it back - this can then be passed throughout the program as it runs whatever operation it needs to do. 

A more technical design is outlined in [[HTool Binary Loader]]

### Mach-O / ELF Parser

Mach-O files are used on Apple's Darwin operating systems. The XNU kernel - a hybrid kernel of the Mach/OSFMK and BSD kernels - inherits this Mach Object file format from Mach/OSFMK. Mach-O's have a defined structure, with a header, load commands, segment commands, symbol tables and dynamic linking information.

The Libhelper Project library - available on my Github - already provides the basic Mach-O parser. HTool will make use of that parser to provide a detailed and nicely-formatted output of a Mach-O file structure to the console.

### Kernelcache Analysis

The "Kernel Cache" is used on iOS, iPadOS, watchOS, tvOS and any other operating system that is forked from iOS. On these devices we have two things - the Kernel, and Kernel Extensions.

The Kernel - built from the XNU source tree with some additions for iOS - is a single Mach-O file. The Kernel Extensions, known as KEXTs, are built from their own source trees, are also single Mach-O files, and are used to extend the functionality of the kernel. These KEXTs will generally add hardware-specific support.

The Kernel Cache is the Kernel plus all the KEXTs. There are three different kernel cache formats that have been used over the years:
- Split Style
- Merged Style
- Fileset Style

HTool would need to be able to identify:
- Firstly, whether a given file is an iOS-based Kernel Cache, or a macOS Kernel.
- Then, which of these formats a given kernel cache uses.
- Identify all the Kernel Extensions contained, with Bundle IDs, file offsets and virtual addresses.
- Ability to extract a KEXT to it's own file.

#### Split Style

The Split Style kernel cache is where the KEXTs are stored, one after the other - in the kernel cache file - as whole, complete Mach-O's that can be extracted, disassembled or executed. The actual Kernel is a Mach-O that starts at the beginning of the cache file and contains an additional segment called `__PRELINK_INFO`. This segment has an XML that maps all the KEXTs within the cache, storing their Bundle Identifier (ID), offset and virtual address they should be loaded at.

#### Merged Style.

With the Merged Style kernel cache, all information mapping the locations of each KEXT have been removed from the `__PRELINK_INFO` segment. 

Instead of the KEXTs being stored as complete Mach-O's, the merged style cache takes all of the KEXT `__TEXT` segments, and all the `__DATA` segments, and stores them together as one giant block, with the headers of each KEXT pointing to where their respective `__TEXT` and `__DATA` segments start and end.

Within the `__PRELINK_INFO` segment, a `__kmod_info` section has been added. This contains a list of `kmod_info` structs that each define a KEXT with a bundle ID, file offset and virtual address.

#### Fileset Style

With iOS 16 Apple introduced a new Mach-O filetype - `MH_FILESET` - and a new Load Command - `LC_FILESET_ENTRY`. A Fileset is a Mach-O that contains other Mach-O's. This is what Apple have used for the latest cache format.

Apple have reverted to storing the KEXTs as whole Mach-O's with their `__TEXT` and `__DATA` segments stored in the same location. The difference here is that the first Mach-O header in the cache is not the actual kernel binary like the other two formats - it's a Mach-O Fileset.

This Fileset only contains around 200 `LC_FILESET_ENTRY` commands that map all the KEXTs, plus the kernel binary - identified with the Bundle ID `com.apple.kernel`. 

### Kernel Version Build & Platform Information

The kernel and all KEXTs contain build strings, usually looking like this:
```
Darwin Kernel Version 21.0.0: Sun Aug 15 20:15:00 PDT; root:xnu-8019.12.5~1/RELEASE_ARM64_T8110
```
From this version string alone, we can determine a number of things:
- Darwin Version
- XNU source version
- Release type
- Architecture and platform
- Build date

### Device Database

A simple "database" that contains all the known devices. This will be stored as an array of `darwin_device_t` structs that contain the platform name, board name, soc name, device type and device identifier.

```
struct darwin_device_properties {

	char *platform;

	char *board;

	char *soc;

	int type;

	char *identifier;

};

typedef struct darwin_device_properties darwin_device_t;
```
An example of defining a type with this structure would be:
```
{ DARWIN_PLATFORM_T8101, DARWIN_BOARD_D54PAP, DARWIN_SOC_A14_BIONIC, DARWIN_DEVICE_TYPE_MOBILE, "iPhone13,4" },
```

### macOS Kernel Collection Analysis

macOS uses a different format to other operating systems. 