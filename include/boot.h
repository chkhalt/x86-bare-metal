/*
 * boot.h
 */

#ifndef BOOT_H
#define BOOT_H

#define BOOTSEG 0x7c0
#define STARTUP32_OFFSET      424
#define MBR_LOAD_INFO_OFFSET  432
#define MBR_PART_TABLE_OFFSET 446
#define MBR_BOOT_SIGNATURE    510

/* gdt segments */
#define DATA32 0x08
#define CODE32 0x10
#define DATA16 0x18
#define CODE16 0x20

#ifdef __ASSEMBLY__
#define LOAD_INFO_U32_INDEX  MBR_LOAD_INFO_OFFSET+0
#define LOAD_INFO_U16_KBSIZE MBR_LOAD_INFO_OFFSET+4
#define LOAD_INFO_U8_KBALIGN MBR_LOAD_INFO_OFFSET+6

#define DEFINE_ASM_LOAD_INFO \
.long 0; .short 0; .byte 0; .byte 0

#else /* !__ASSEMBLY__ */
struct load_info {
	unsigned int   index;   /* sector index */
	unsigned short kbsize;  /* size in KBytes */
	unsigned char  kbalign; /* alignment */
	unsigned char  zero;
} __attribute__((packed));
#endif

#endif /* BOOT_H */
