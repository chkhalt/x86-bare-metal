/*
 * boot.S - switch to protected mode, run 32-bit code
 */

 #include "boot.h"
 #include "compiler.h"

/* set when we came from sipi */
#define DX_INIT_FLG 0x600

.section __TEXT_NAME__,__TEXT_FLAGS__

.code16
jmp realmode

.balign 8
gdt:
.quad 0x0000000000000000	/* NULL Segment */
.quad 0x00CF92000000FFFF	/* Data 32      */
.quad 0x00CF9A000000FFFF	/* Code 32      */
.quad 0x000F92000000FFFF	/* Data 16      */
.quad 0x000F9A000000FFFF	/* Code 16      */

gdtr:
.short gdtr-gdt-1
.long  gdt

realmode:
	/* normalize segments */
	mov     %cs, %ax
	test    %ax, %ax
	jnz     L_set_data_seg

	/* get seg from ip */
	call    .+3
	pop     %ax
	shr     $8, %ax
	shl     $4, %ax

	/* set cs:ip */
	push    %ax
	push    $L_set_data_seg
	lret

L_set_data_seg:
	xor     %bx, %bx
	mov     %ax, %ds
	shl     $4,  %ax
	mov     %bx, %ss
	mov     %ax, %sp
	sti
	cld

	test    $DX_INIT_FLG, %dx
	jnz     L_ap_startup

L_boot_startup:
	cli
	movw    $2, %ax
	int     $0x10

	mov     %dl, drive(%bx)
	push    $0x40
	pop     %es

	/* reserve memory (bda) */
	xor     %eax, %eax
	/* decrease memory */
	mov     LOAD_INFO_U16_KBSIZE, %ax
	sub     %ax, %es:0x13
	/* adjust align */
	mov     LOAD_INFO_U8_KBALIGN, %al
	neg     %al
	andb    %al, %es:0x13

	/* save segment */
	mov     %es:0x13, %ax
	shl     $6, %ax
	mov     %ax, dap+6(%bx)

	/* set 32-bit code offset */
	shl     $4, %eax
	mov     %eax, startup32(%bx)

	/* load payload */
	mov     LOAD_INFO_U16_KBSIZE, %cx
	shl     $1, %cx /* kb times 2 */
	movw    %cx, count(%bx)

	mov     LOAD_INFO_U32_INDEX, %cx
	mov     %cx, dap+8(%bx)

L_load_sector:
	/* more sectors to read? */
	testw   $0xffff, count(%bx)
	jz      L_ap_startup

	/* read sector */
	lea     dap(%bx), %si
	mov     drive, %dl
	mov     $0x4200, %ax
	int     $0x13
	jb      L_retry

	/* adjust values */
	xor     %ax, %ax
	mov     $0x1000, %cx
	decw    count(%bx)       /* dec count */
	incl    dap+8(%bx)       /* inc index */
	addw    $512, dap+4(%bx) /* adjust offset */
	cmovb   %cx, %ax
	add     %ax, dap+6(%bx)  /* adjust segment, if necessary */
	jmp     L_load_sector

L_retry:
	mov     $0xe, %ah
	mov     $7, %bx
	lea     readerr(%bx), %si
1:
	lodsb
	test    %al, %al
	jz      L_load_sector

	int     $0x10
	jmp     1b

L_ap_startup:
	/* fix gdtr offset */
	xor     %ebx, %ebx
	xor     %ecx, %ecx
	mov     %cs, %cx
	shl     $4, %ecx
	add     $8, %ecx
	mov     %ecx, gdtr+2(%bx)

	/* switch to pmode */
	lgdt    gdtr(%bx)
	mov     %cr0, %eax
	or      $1, %eax
	mov     %eax, %cr0

	/* data segs */
	mov     $DATA32, %ax
	mov     %ax, %ds
	mov     %ax, %es
	mov     %ax, %fs
	mov     %ax, %gs
	mov     %ax, %ss

	ljmpl   *%cs:startup32(%bx)

.balign 16
dap:
.short 0x10
.short 0x01 /* +2 num of sectors */
.short 0x00 /* +4 offset */
.short 0x00 /* +6 segment */
.long  0x00 /* +8 lba low */
.long  0x00 /* +c lba high */

drive:   .short 0
count:   .short 0
readerr: .asciz "reading again...\r\n"

.org STARTUP32_OFFSET
/* seg:off */
startup32: .long 0, CODE32

.org MBR_LOAD_INFO_OFFSET
DEFINE_ASM_LOAD_INFO

.org MBR_PART_TABLE_OFFSET
.quad 0,0
.quad 0,0
.quad 0,0
.quad 0,0

.org  MBR_BOOT_SIGNATURE
.byte 0x55, 0xaa


