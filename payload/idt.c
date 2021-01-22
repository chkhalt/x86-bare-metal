/*
 * idt.c
 */

#include "boot.h"
#include "video.h"
#include "compiler.h"
#include "idt.h"

#define NUMISR 48

/* idtr (32-bit) */
#pragma pack(push, 1)
typedef struct _idt_r32_t {
	uint16_t size;
	uint32_t offset;
} idt_r32_t;

/* idt descriptor (32-bit) */
typedef struct _idt_d32_t {
	uint16_t offset_1;
	uint16_t selector;
	uint8_t  zero;
	uint8_t  type_attr;
	uint16_t offset_2;
} idt_d32_t;
#pragma pack(pop)


idt_r32_t __use_section_data __align(16) idtr32  = { 0, 0 };
idt_d32_t __use_section_data __align(16) idt32[NUMISR] = {
	[0 ... NUMISR-1] = { 0, CODE32, 0, 0x8e, 0}
};

static void __attribute__((noreturn)) unhandled(int isr)
{
	printf("*** unhandled isr=%d ***\n", isr);
	__halt();
}

#define __ISR_NAME(num) isr_n ## num ## _handler

#define DEFINE_UNHANDLED_ISR(num)\
void __ISR_NAME(num)(void) {     \
	unhandled(num);              \
}

/* intel */
DEFINE_UNHANDLED_ISR(0)
DEFINE_UNHANDLED_ISR(1)
DEFINE_UNHANDLED_ISR(2)
DEFINE_UNHANDLED_ISR(3)
DEFINE_UNHANDLED_ISR(4)
DEFINE_UNHANDLED_ISR(5)
DEFINE_UNHANDLED_ISR(6)
DEFINE_UNHANDLED_ISR(7)
DEFINE_UNHANDLED_ISR(8)
DEFINE_UNHANDLED_ISR(9)
DEFINE_UNHANDLED_ISR(10)
DEFINE_UNHANDLED_ISR(11)
DEFINE_UNHANDLED_ISR(12)
DEFINE_UNHANDLED_ISR(13)
DEFINE_UNHANDLED_ISR(14)
DEFINE_UNHANDLED_ISR(15)
DEFINE_UNHANDLED_ISR(16)
DEFINE_UNHANDLED_ISR(17)
DEFINE_UNHANDLED_ISR(18)
DEFINE_UNHANDLED_ISR(19)
DEFINE_UNHANDLED_ISR(20)

/* unused */
DEFINE_UNHANDLED_ISR(21)
DEFINE_UNHANDLED_ISR(22)
DEFINE_UNHANDLED_ISR(23)
DEFINE_UNHANDLED_ISR(24)
DEFINE_UNHANDLED_ISR(25)
DEFINE_UNHANDLED_ISR(26)
DEFINE_UNHANDLED_ISR(27)
DEFINE_UNHANDLED_ISR(28)
DEFINE_UNHANDLED_ISR(29)
DEFINE_UNHANDLED_ISR(30)
DEFINE_UNHANDLED_ISR(31)

/* irq 0-7 */
DEFINE_UNHANDLED_ISR(32)
DEFINE_UNHANDLED_ISR(33)
DEFINE_UNHANDLED_ISR(34)
DEFINE_UNHANDLED_ISR(35)
DEFINE_UNHANDLED_ISR(36)
DEFINE_UNHANDLED_ISR(37)
DEFINE_UNHANDLED_ISR(38)
DEFINE_UNHANDLED_ISR(39)

/* irq 8-15 */
DEFINE_UNHANDLED_ISR(40)
DEFINE_UNHANDLED_ISR(41)
DEFINE_UNHANDLED_ISR(42)
DEFINE_UNHANDLED_ISR(43)
DEFINE_UNHANDLED_ISR(44)
DEFINE_UNHANDLED_ISR(45)
DEFINE_UNHANDLED_ISR(46)
DEFINE_UNHANDLED_ISR(47)


void idt_set_gate(uint8_t num, void *isr)
{
	uint32_t offset = (uint32_t) isr;
	idt32[num].offset_1 = (uint16_t) offset;
	idt32[num].offset_2 = (uint16_t) (offset >> 16);
}

void idt_init(void)
{
	/* set unhandled */
	idt_set_gate(0,  (void *) __ISR_NAME(0));
	idt_set_gate(1,  (void *) __ISR_NAME(1));
	idt_set_gate(2,  (void *) __ISR_NAME(2));
	idt_set_gate(3,  (void *) __ISR_NAME(3));
	idt_set_gate(4,  (void *) __ISR_NAME(4));
	idt_set_gate(5,  (void *) __ISR_NAME(5));
	idt_set_gate(6,  (void *) __ISR_NAME(6));
	idt_set_gate(7,  (void *) __ISR_NAME(7));
	idt_set_gate(8,  (void *) __ISR_NAME(8));
	idt_set_gate(9,  (void *) __ISR_NAME(9));
	idt_set_gate(10, (void *) __ISR_NAME(10));
	idt_set_gate(11, (void *) __ISR_NAME(11));
	idt_set_gate(12, (void *) __ISR_NAME(12));
	idt_set_gate(13, (void *) __ISR_NAME(13));
	idt_set_gate(14, (void *) __ISR_NAME(14));
	idt_set_gate(15, (void *) __ISR_NAME(15));
	idt_set_gate(16, (void *) __ISR_NAME(16));
	idt_set_gate(17, (void *) __ISR_NAME(17));
	idt_set_gate(18, (void *) __ISR_NAME(18));
	idt_set_gate(19, (void *) __ISR_NAME(19));
	idt_set_gate(20, (void *) __ISR_NAME(20));
	idt_set_gate(21, (void *) __ISR_NAME(21));
	idt_set_gate(22, (void *) __ISR_NAME(22));
	idt_set_gate(23, (void *) __ISR_NAME(23));
	idt_set_gate(24, (void *) __ISR_NAME(24));
	idt_set_gate(25, (void *) __ISR_NAME(25));
	idt_set_gate(26, (void *) __ISR_NAME(26));
	idt_set_gate(27, (void *) __ISR_NAME(27));
	idt_set_gate(28, (void *) __ISR_NAME(28));
	idt_set_gate(29, (void *) __ISR_NAME(29));
	idt_set_gate(30, (void *) __ISR_NAME(30));
	idt_set_gate(31, (void *) __ISR_NAME(31));
	idt_set_gate(32, (void *) __ISR_NAME(32));
	idt_set_gate(33, (void *) __ISR_NAME(33));
	idt_set_gate(34, (void *) __ISR_NAME(34));
	idt_set_gate(35, (void *) __ISR_NAME(35));
	idt_set_gate(36, (void *) __ISR_NAME(36));
	idt_set_gate(37, (void *) __ISR_NAME(37));
	idt_set_gate(38, (void *) __ISR_NAME(38));
	idt_set_gate(39, (void *) __ISR_NAME(39));
	idt_set_gate(40, (void *) __ISR_NAME(40));
	idt_set_gate(41, (void *) __ISR_NAME(41));
	idt_set_gate(42, (void *) __ISR_NAME(42));
	idt_set_gate(43, (void *) __ISR_NAME(43));
	idt_set_gate(44, (void *) __ISR_NAME(44));
	idt_set_gate(45, (void *) __ISR_NAME(45));
	idt_set_gate(46, (void *) __ISR_NAME(46));
	idt_set_gate(47, (void *) __ISR_NAME(47));

	/* set idtr */
	idtr32.size = sizeof(idt32) - 1;
	idtr32.offset = (uint32_t) idt32;

	__asm__ volatile ("lidt %0" :: "m" (idtr32));
}
