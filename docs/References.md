#### Kernelcache Format
- Apple Platform Security - Kernel Extensions in macOS: https://support.apple.com/en-gb/guide/security/sec8e454101b/web
	- High level overview of Kernel Extensions in macOS
	- Doesn't provide much technical depth
- Brandon Azad, Google Project Zero - Handling Kernel Extensions in iOS 12: https://bazad.github.io/2018/06/ios-12-kernelcache-tagged-pointers/
	- Covers in-depth the Kernel cache format in iOS 12
- Brandon Azad, Google Project Zero - Analyzing the iOS 12 kernelcache's tagged pointers: https://bazad.github.io/2018/06/ios-12-kernelcache-tagged-pointers/
	- Covers tagged pointers and pointer authentication.
	- Doesn't directly cover the format, but has some technical detail on the segments in the kernel binary.
- macOS Boot Process: https://news.ycombinator.com/item?id=26113488
	- Has some detail on how macOS KEXTs function.
- Me: https://h3adsh0tzz.com/posts/macho-file-format
- Me again: https://h3adsh0tzz.com/posts/handle-kernel-extensions

#### Reverse engineering / disassembling
- Brandon Azad, Google Project Zero - Kernel pointer crash log: https://bazad.github.io/2018/04/kernel-pointer-crash-log-ios/
	- Has some detail on kASLR.
- Brandon Azad, Google Project Zero - ida_kernelcache tool: https://github.com/bazad/ida_kernelcache
- Jonathan Levin, JTool - Taking the O out of otool: http://www.newosxbook.com/tools/jtool.html