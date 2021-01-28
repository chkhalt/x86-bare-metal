# x86-bare-metal
Minimal code to learn about x86 system programming. For now it's just code to test some cpu resources.

**build**  
`$ make`  
Should be possible to build on gcc or clang.  
For macosx you will probably need binutils (if so use: `brew install binutils`). 


**install**  
`$ ./bminstall -p payload/something targetdisk`  

**test**  
`qemu-system-x86_64 -drive file=targetdisk,format=raw -monitor stdio -s -cpu core2duo -smp cores=4`  
