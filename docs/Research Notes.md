## macOS Kernel

While the macOS kernel is located at `/System/Library/Kernels/kernel`, this is not the kernel the machine boots from. This process is detailed better in https://news.ycombinator.com/item?id=26113488

- iBoot is loaded from `/System/Volumes/Preboot/{UUID}/boot/{longUUID}/usr/standalone/firmware/iBoot.img4`
- iBoot will then load the kernelcache from `/System/Volumes/Preboot/{UUID}/boot/{longUUID}/System/Library/Caches/kernelcache`
- 