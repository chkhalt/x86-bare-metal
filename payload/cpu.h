/*
 * cpu.h
 */

#ifndef CPU_H
#define CPU_H

#include "inttypes.h"

#define MAXCPU 4

#ifndef __ASSEMBLY__
#define CR0_PE 0x00000001
#define CR0_MP 0x00000002
#define CR0_EM 0x00000004
#define CR0_TS 0x00000008
#define CR0_ET 0x00000010
#define CR0_NE 0x00000020
#define CR0_WP 0x00010000
#define CR0_AM 0x00040000
#define CR0_NW 0x20000000
#define CR0_CD 0x40000000
#define CR0_PG 0x80000000

#define CR4_VME        0x00000001
#define CR4_PVI        0x00000002
#define CR4_TSD        0x00000004
#define CR4_DE         0x00000008
#define CR4_PSE        0x00000010
#define CR4_PAE        0x00000020
#define CR4_MCE        0x00000040
#define CR4_PGE        0x00000080
#define CR4_PCE        0x00000100
#define CR4_OSFXSR     0x00000200
#define CR4_OSXMMEXCPT 0x00000400
#define CR4_UMIP       0x00000800
#define CR4_LA57       0x00001000
#define CR4_VMXE       0x00002000
#define CR4_SMXE       0x00004000
#define CR4_FSGSBASE   0x00010000
#define CR4_PCIDE      0x00020000
#define CR4_OSXSAVE    0x00040000
#define CR4_SMEP       0x00100000
#define CR4_SMAP       0x00200000
#define CR4_PKE        0x00400000

#define SYS_CTRL_PORTA 0x92 /* System Control Port A */
#define CTRL_A_FLG_AHR 1    /* alternate hot reset */
#define CTRL_A_FLG_A20 2    /* a20 gate */

#define SYS_CTRL_PORTB 0x61 /* System Control Port B */
#define CTRL_B_FLG_T2I 1    /* pit channel 2, gate input */
#define CTRL_B_FLG_SDE 2    /* speaker data enable */
#define CTRL_B_FLG_T2O 32   /* pit channel 2, output */


void x86_cpu_init(void);

static inline uint64_t rdtsc(void)
{
    unsigned int high, low;
    __asm__ volatile ("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t) high << 32) | low;
}

static inline void
cli(void) {
	__asm__ volatile ("cli");
}

static inline void
sti(void) {
	__asm__ volatile ("sti");
}

static inline void
__cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
	/* ecx is often an input as well as an output. */
	__asm__ volatile (
		"cpuid;"
		: "=a" (*eax),
		  "=b" (*ebx),
		  "=c" (*ecx),
		  "=d" (*edx)
		:  "0" (*eax), "2" (*ecx)
	);
}

static inline uint8_t
__apicid()
{
	uint32_t eax = 1, ebx = 0, ecx = 0, edx = 0;
	__cpuid(&eax, &ebx, &ecx, &edx);
	return (ebx >> 24);
}

static inline long
__readcr0(void)
{
	long cr0val;
	__asm__ volatile ("mov %%cr0, %0" : "=a"(cr0val) : : "memory");
	return cr0val;
}

static inline void
__writecr0(long cr0val)
{
	__asm__ volatile ("mov %0, %%cr0" : : "a"(cr0val) : "memory");
}

static inline long
__readcr3(void)
{
	long cr3val;
	__asm__ volatile ("mov %%cr3, %0" : "=a"(cr3val) : : "memory");
	return cr3val;
}

static inline void
__writecr3(long cr3val)
{
	__asm__ volatile ("mov %0, %%cr3" : : "a"(cr3val) : "memory");
}

static inline long
__readcr4(void)
{
	long cr4val;
	__asm__ volatile ("mov %%cr4, %0" : "=a"(cr4val) : : "memory");
	return cr4val;
}

static inline void
__writecr4(long cr4val)
{
	__asm__ volatile ("mov %0, %%cr4" : : "a"(cr4val) : "memory");
}

static inline uint64_t
__rdmsr(uint32_t msr)
{
	uint32_t low = 0, high = 0;

	__asm__("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
	return ((uint64_t) high << 32) | low;
}

static inline void
__wrmsr(uint32_t msr, uint64_t val)
{
	uint32_t low = (uint32_t) val;
	uint32_t high = (uint32_t) (val >> 32);

	__asm__("wrmsr" : "=a"(low), "=d"(high) : "c"(msr));
}

#endif /* !__ASSEMBLY__ */
#endif /* CPU_H */
