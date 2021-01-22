/*
 * smp_wakeup_test.c
 */

#include "cpu.h"
#include "video.h"
#include "lapic.h"
#include "paging.h"
#include "compiler.h"


typedef struct _stack32 {
	uint8_t d[1024];
} __align(16) stack32_t;

stack32_t __use_section_data pcpu_stack_32[MAXCPU];

static inline __attribute__((always_inline))
void set_thread_stack(void)
{
	stack32_t *sp = &pcpu_stack_32[__apicid() + 1];
	__asm__ volatile ("movl %0, %%esp" :: "m" (sp));
}

static inline __attribute__((always_inline))
void x86_basic_init()
{
	cli();
	set_thread_stack();
	x86_cpu_init();
	apic_init();
	init_early_pages();
	set_paging_on();
	sti();
}

void __attribute__((noreturn)) startup32()
{
	uint8_t apicid = __apicid();
	x86_basic_init();
	printf("cpu %d: initialized\n", apicid);

	if (apicid == 0) {
		for (uint8_t i = 1; i < MAXCPU; i++) {
			printf("cpu %d: trying to wake up ap %d...\n", apicid, i);
			apic_init_thread(i, startup32);
			apic_timer_wait_ms(1000);
		}
	}

	__halt();
}

