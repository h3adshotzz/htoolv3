### Provide a suitable title for your project.

Developing a static analysis tool "HTool" for reverse engineering Apple OS Firmware Files.

---
#### **Define the main problem the project will attempt to solve, plus a supplemental problem that it is hoped the project will also attempt to address.**

Reverse engineering tools that are focused on Apple platforms, such as iOS and macOS, are limited, and those that are available are either no longer updated, or extremely expensive, i.e. JTool and IDA64.

Firmware files on Apple's platforms often use undocumented and proprietary file formats. For example:
- The Kernel Cache is a file that takes the Kernel binary, and all the "Kernel Extensions", are merged into a single file. Apple change the way that these KEXTs are located within this kernelcache every few versions, so the tool needs to be able to detect which version and handle them appropriately. Being able to "extract" these KEXTs, reverse engineer and debug them is vital for finding security vulnerabilities in different pieces of hardware.
- The bootloader, named iBoot, is also an undocumented file format. Like the Kernel Cache, it doesn't just contain the bootloader, but other firmware for different components.

This project aims to solve this problem by developed a command-line tool - similar to JTool - that provides security researchers the ability to statically analyse generic Mach-O binaries, and these file formats. 

This solution potentially covers areas such as software design, platforms, static analysis, memory management and efficiency.

An additional problem that could also be solved would be the support for more lower-level firmware files, DeviceTree, and SEP/SEPOS, that use more proprietary and obscure file formats. This would require additional research into the structure of these binaries. 

---
#### **Describe the inspiration and/or motivation for the project****. This should include why the problem interests you personally.**

This is a problem that inspires me because of my existing interest in Apple device security, reverse engineering and the XNU kernel internals. I have experience of using both JTool and IDA64 and have a clear idea of the functionality the tools requires to be able to solve the problem.

I enjoy looking into how low-level components in operating systems work. This project would involve investigating undocumented file structures, using frameworks such as LLVM for disassembling, and writing a binary loader that can identify and parse lots of different file formats with different structures efficiently.

The development of this tool would also benefit me in my eventual career goal of working in a reverse engineering or security research role, or just in general as a software engineer.

---
#### **Describe the areas of research you initially need to explore. Identify any initial references considered.**

iOS/macOS platforms have a reputation for "security through obscurity", therefore the resources available are limited. A part of the research that needs to be conducted will be reverse engineering firmware files to gain an understanding of their structure and format.

Research into, and an overview of, existing solutions, such as otool, JTool, IDA64, Hopper and Ghidra, would be useful to understand the scope of features that the solution should aim to support - and what kind of effort would be required.

Some areas, however, such as the Mach-O file format, are extensively documented and is a file format i have good knowledge of. Mach-O is essentially the macOS/iOS equivalent of an ELF or EXE file.

Azad, B. (2018) _Analyzing the iOS 12 kernelcache's tagged pointers_. Google Project Zero. Available at: https://bazad.github.io/2018/06/ios-12-kernelcache-tagged-pointers/ (Accessed: October 25, 2022).

Levin, J. (2017) _Jtool_, _JTool2 - Taking the O out of otool - squared_. Available at: http://www.newosxbook.com/tools/jtool.html (Accessed: October 25, 2022).

Apple. (2021) _Kernel extensions in macos_, _Apple Support_. Apple. Available at: https://support.apple.com/en-gb/guide/security/sec8e454101b/web (Accessed: October 25, 2022).

---
#### **Describe the artefact that may be generated during the lifetime of the project, based on an initial consideration of possible solutions to the problem chosen.**

The artefact that I hope to produce by the end of this project will be a command line tool - similar to JTool - that is capable of analysing and possibly disassembling the firmware files mentioned quickly, efficiently (as some files can be quite big) and produce a clean output that is easy to interpret.

Ideally, the files HTool should support at a minimum are:
- All Mach-O Binary formats
- Kernel Cache
- iBoot

Furthermore, if the tool can support the following, it would be a bonus
- DeviceTree
- SEP/SEPOS
- Any other generic firmware.

This would solve the proposed problem and provide a useful reverse engineering tool that is both up-to-date and free, and could serve as a basis for a future, open-source community developed reverse engineering tool.

---
#### **Detail what knowledge you wish or expect to gain from undertaking this project**

By undertaking this project I hope to improve my knowledge of the internals of the iOS and macOS operating systems; the structure and format of the Kernel cache, iBoot and other firmware components and my skills at writing applications that deal with parsing undocumented file formats. I also believe this project will improve my general reverse engineering abilities.