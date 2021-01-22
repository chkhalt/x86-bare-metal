/*
 * cpu.c
 */

#include "io.h"
#include "pic.h"
#include "idt.h"
#include "cpu.h"
#include "compiler.h"
#include "inttypes.h"

extern unsigned long __stack_chk_guard; /* from stack.c */

static void fast_a20_enable(void)
{
	uint8_t b = inb(SYS_CTRL_PORTA);
	if ((b & 2) == 0) {
		outb(SYS_CTRL_PORTA,((b | 2) & ~CTRL_A_FLG_AHR));
	}
}

static void x86_enable_sse(void)
{
	uint32_t a = 1, b = 0, c = 0, d = 0;
	__cpuid(&a, &b, &c, &d);

	/* SSE */
	if (d & 0x2000000) {
		__writecr0(__readcr0() & ~CR0_EM);
		__writecr0(__readcr0() | CR0_MP);
		__writecr4(__readcr4() | CR4_OSFXSR);
		__writecr4(__readcr4() | CR4_OSXMMEXCPT);
	}
}

void x86_cpu_init(void)
{
	uint64_t tsc = rdtsc();

	/* ensure a20 */
	fast_a20_enable();

	/* remap irq */
	pic_irq_remap(PIC1_PROT_M_OFFSET, PIC2_PROT_M_OFFSET);

	/* init gates */
	idt_init();

	/* enable sse */
	x86_enable_sse();

	/* init stack guard */
	__stack_chk_guard = (uint32_t) (tsc ^ (tsc >> 32));
}
