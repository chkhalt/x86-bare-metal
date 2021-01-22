/*
 * tlb_after_sipi.c
 */

#include "cpu.h"
#include "video.h"
#include "lapic.h"
#include "paging.h"
#include "compiler.h"


static inline __attribute__((always_inline))
void x86_basic_init()
{
	cli();
	x86_cpu_init();
	apic_init();
	init_early_pages();
	set_paging_on();
	sti();
}

void __attribute__((noreturn)) startup32()
{
	x86_basic_init();
	puts("[tlb_after_sipi]: start\n");
	puts("*** not implemented ***\n");
	puts("[tlb_after_sipi]: end\n");
	__trap();
}

