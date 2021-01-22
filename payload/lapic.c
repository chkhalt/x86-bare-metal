/*
 * lapic.c
 */

#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "cpu.h"
#include "boot.h"
#include "video.h"
#include "string.h"
#include "compiler.h"
#include "lapic.h"

/* ap trampoline */
#define APTRAMPOLINE 0x8000

/* APIC BASE MSR */
#define IA32_APIC_BASE          0x1b
#define APIC_BASE_BSP           0x100 /* Intel RW, AMD R Only */
#define APIC_BASE_x2APIC_ENABLE 0x400 /* Intel only, reserved in AMD */
#define APIC_BASE_APIC_ENABLE   0x800

/* Intel APIC, Pentium, P6 family, 3-wire APIC Bus */
/* AMD APIC, all families ?, bus ? */
#define APIC_ID           0x20  /* (RW) id = (u32 >> 24) */
#define APIC_VER          0x30  /* R Only */
#define APIC_TPR          0x80  /* Task Priority Register (RW) */
#define APIC_APR          0x90  /* Arbitration Priority Register (R Only) */
#define APIC_PPR          0xa0  /* Processor Priority Register (Read Only) */
#define APIC_EOI          0xb0  /* EOI Register (Write Only) */

/* WARNING: this register seems to be reserved, review before use */
/* http://www.cs.cmu.edu/afs/cs/academic/class/15213-s01/docs/intel-sys.pdf */
#define APIC_RRD          0xc0  /* Remote Read Register (Read Only) */
#define APIC_LDR          0xd0  /* Logical Destination Register (RW) */
#define APIC_DFR          0xe0  /* Destination Format Register (RW) */
#define APIC_SIVR         0xf0  /* Spurious Interrupt Vector Register (RW) */

/* In-Service Register (R Only) */
#define APIC_ISR_31_0     0x100
#define APIC_ISR_63_32    0x110
#define APIC_ISR_95_64    0x120
#define APIC_ISR_127_96   0x130
#define APIC_ISR_159_128  0x140
#define APIC_ISR_191_160  0x150
#define APIC_ISR_223_192  0x160
#define APIC_ISR_255_224  0x170

/* Trigger Mode Register (R Only) */
#define APIC_TMR_31_0     0x180
#define APIC_TMR_63_32    0x190
#define APIC_TMR_95_64    0x1a0
#define APIC_TMR_127_96   0x1b0
#define APIC_TMR_159_128  0x1c0
#define APIC_TMR_191_160  0x1d0
#define APIC_TMR_223_192  0x1e0
#define APIC_TMR_255_224  0x1f0

/* Interrupt Request Register (Read Only) */
#define APIC_IRR_31_0     0x200
#define APIC_IRR_63_32    0x210
#define APIC_IRR_95_64    0x220
#define APIC_IRR_127_96   0x230
#define APIC_IRR_159_128  0x240
#define APIC_IRR_191_160  0x250
#define APIC_IRR_223_192  0x260
#define APIC_IRR_255_224  0x270

/* Error Status Register (Read Only) */
#define APIC_ESR          0x280

/* WARNING: this register seems to be reserved, review before use
   http://www.cs.cmu.edu/afs/cs/academic/class/15213-s01/docs/intel-sys.pdf
   >= Nehalem Intel V3 - 10.4.8 Local APIC Version Register
*/
#define APIC_LVT_CMCI     0x2f0 /* Intel (RW), reserved in AMD */

/* Interrupt Command Registers (RW) */
#define APIC_ICR0         0x300
#define APIC_ICR1         0x310

/* Local Vector Table */
#define APIC_LVT_TIMER    0x320  /* Timer Register (RW) */

/* >= P4 (xAPIC) */
#define APIC_LVT_THERM    0x330  /* Thermal Sensor Register (RW) */
#define APIC_LVT_PERFC    0x340  /* Performance Monitoring Counters (RW) >= Pentium Pro */
#define APIC_LVT_LINT0    0x350  /* LVT LINT0 (RW) */
#define APIC_LVT_LINT1    0x360  /* LVT LINT1 (RW) */
#define APIC_LVT_ERR      0x370  /* LVT Error (RW) >= Pentium */

#define APIC_TIMER_INI    0x380  /* Timer Initial Count Register (RW) */
#define APIC_TIMER_CNT    0x390  /* Timer Current Count Register (R Only) */
#define APIC_TIMER_DIV    0x3e0  /* Timer Divide Configuration Register (RW) */

/* AMD only */
#define APIC_EXT_FEATURES 0x400
#define APIC_EXT_CONTROL  0x410
#define APIC_SEOI         0x420
#define APIC_IER0         0x480
#define APIC_IER1         0x490
#define APIC_IER2         0x4a0
#define APIC_IER3         0x4b0
#define APIC_IER4         0x4c0
#define APIC_IER5         0x4d0
#define APIC_IER6         0x4e0
#define APIC_IER7         0x4f0
#define APIC_LVT_EXT0     0x500
#define APIC_LVT_EXT1     0x510
#define APIC_LVT_EXT2     0x520
#define APIC_LVT_EXT3     0x530


/* Intel xAPIC, Pentium 4, Intel Xeon, System Bus */
#define xAPIC_ID          0x20   /* Intel >= Nehalem R Only, intel < Nehalem RW, id = (u32 >> 24) */
#define xAPIC_VER         0x30   /* R Only */
#define xAPIC_TPR         0x80   /* Task Priority Register (RW) */
#define xAPIC_PPR         0xa0   /* Processor Priority Register (Read Only) */
#define xAPIC_EOI         0xb0   /* EOI Register (Write Only) */
#define xAPIC_LDR         0xd0   /* Logical Destination Register (Read Only) */
#define xAPIC_DFR         0xe0   /* Destination Format Register (RW ?) */
#define xAPIC_SIVR        0xf0   /* Spurious Interrupt Vector Register (RW) */

/* In-Service Register (R Only) */
#define xAPIC_ISR_31_0    0x100
#define xAPIC_ISR_63_32   0x110
#define xAPIC_ISR_95_64   0x120
#define xAPIC_ISR_127_96  0x130
#define xAPIC_ISR_159_128 0x140
#define xAPIC_ISR_191_160 0x150
#define xAPIC_ISR_223_192 0x160
#define xAPIC_ISR_255_224 0x170

/* Trigger Mode Register (R Only) */
#define xAPIC_TMR_31_0    0x180
#define xAPIC_TMR_63_32   0x190
#define xAPIC_TMR_95_64   0x1a0
#define xAPIC_TMR_127_96  0x1b0
#define xAPIC_TMR_159_128 0x1c0
#define xAPIC_TMR_191_160 0x1d0
#define xAPIC_TMR_223_192 0x1e0
#define xAPIC_TMR_255_224 0x1f0

/* Interrupt Request Register (Read Only) */
#define xAPIC_IRR_31_0    0x200
#define xAPIC_IRR_63_32   0x210
#define xAPIC_IRR_95_64   0x220
#define xAPIC_IRR_127_96  0x230
#define xAPIC_IRR_159_128 0x240
#define xAPIC_IRR_191_160 0x250
#define xAPIC_IRR_223_192 0x260
#define xAPIC_IRR_255_224 0x270

/*  Error Status Register (RW) */
#define xAPIC_ESR         0x280

/* WARNING: review before use
 * >= Nehalem Intel V3 - 10.4.8 Local APIC Version Register
 */
#define xAPIC_LVT_CMCI    0x2f0  /* Intel (RW), reserved in AMD */

/* Interrupt Command Registers (RW) */
#define xAPIC_ICR0        0x300
#define xAPIC_ICR1        0x310

/* Local Vector Table */
#define xAPIC_LVT_TIMER   0x320  /* Timer Register (RW) */
#define xAPIC_LVT_THERM   0x330  /* Thermal Sensor Register (RW) */
#define xAPIC_LVT_PERFC   0x340  /* Performance Monitoring Counters (RW) */
#define xAPIC_LVT_LINT0   0x350  /* LVT LINT0 (RW) */
#define xAPIC_LVT_LINT1   0x360  /* LVT LINT1 (RW) */
#define xAPIC_LVT_ERR     0x370  /* LVT Error (RW) */

#define xAPIC_TIMER_INI   0x380  /* Timer Initial Count Register (RW) */
#define xAPIC_TIMER_CNT   0x390  /* Timer Current Count Register (R Only) */
#define xAPIC_TIMER_DIV   0x3e0  /* Timer Divide Configuration Register (RW) */


/* Intel x2APIC, >= Nehalem, System Bus
   read, write using RDMSR, WRMSR
*/
#define x2APIC_ID         0x802  /* R Only, id = u32 */
#define x2APIC_VER        0x803  /* R Only */
#define x2APIC_TPR        0x808  /* Task Priority Register (RW) */
#define x2APIC_PPR        0x80a  /* Processor Priority Register (Read Only) */
#define x2APIC_EOI        0x80b  /* EOI Register (Write Only) */
#define x2APIC_LDR        0x80d  /* Logical Destination Register (Read Only) */
#define x2APIC_SIVR       0x80f  /* Spurious Interrupt Vector Register (RW) */

/* In-Service Register (R Only) */
#define x2APIC_ISR_31_0    0x810
#define x2APIC_ISR_63_32   0x811
#define x2APIC_ISR_95_64   0x812
#define x2APIC_ISR_127_96  0x813
#define x2APIC_ISR_159_128 0x814
#define x2APIC_ISR_191_160 0x815
#define x2APIC_ISR_223_192 0x816
#define x2APIC_ISR_255_224 0x817

/* Trigger Mode Register (R Only) */
#define x2APIC_TMR_31_0    0x818
#define x2APIC_TMR_63_32   0x819
#define x2APIC_TMR_95_64   0x81a
#define x2APIC_TMR_127_96  0x81b
#define x2APIC_TMR_159_128 0x81c
#define x2APIC_TMR_191_160 0x81d
#define x2APIC_TMR_223_192 0x81e
#define x2APIC_TMR_255_224 0x81f

/* Interrupt Request Register (Read Only) */
#define x2APIC_IRR_31_0    0x820
#define x2APIC_IRR_63_32   0x821
#define x2APIC_IRR_95_64   0x822
#define x2APIC_IRR_127_96  0x823
#define x2APIC_IRR_159_128 0x824
#define x2APIC_IRR_191_160 0x825
#define x2APIC_IRR_223_192 0x826
#define x2APIC_IRR_255_224 0x827

#define x2APIC_ESR         0x828 /* Error Status Register (RW) */
#define x2APIC_LVT_CMCI    0x82f /* Intel (RW), reserved in AMD */

/* Interrupt Command Registers (RW) */
#define x2APIC_ICR         0x830

/* Local Vector Table */
#define x2APIC_LVT_TIMER   0x832 /* Timer Register (RW) */
#define x2APIC_LVT_THERM   0x833 /* Thermal Sensor Register (RW) */
#define x2APIC_LVT_PERFC   0x834 /* Performance Monitoring Counters (RW) */
#define x2APIC_LVT_LINT0   0x835 /* LVT LINT0 (RW) */
#define x2APIC_LVT_LINT1   0x836 /* LVT LINT1 (RW) */
#define x2APIC_LVT_ERR     0x837 /* LVT Error (RW) */

#define x2APIC_TIMER_INI   0x838 /* Timer Initial Count Register (RW) */
#define x2APIC_TIMER_CNT   0x839 /* Timer Current Count Register (R Only) */
#define x2APIC_TIMER_DIV   0x83e /* Timer Divide Configuration Register (RW) */
#define x2APIC_SELF_IPI    0x83f /* Self-IPI (Write Only) */


#define APIC_LVT_MASK       0x10000

#define APIC_TIMER_ONESHOT  0
#define APIC_TIMER_PERIODIC 0x20000
#define APIC_TIMER_TSC      0x40000

#define APIC_TIMER_IRQ      32
#define APIC_SPURIOUS_IRQ   39

/* timer handlers (per cpu) */
//timer_handler_t __use_section_data __align(16) apic_percpu_timer[MAXCPU] = { 0 };
uint32_t __use_section_data __align(16) pcpu_ticks_per_ms[MAXCPU] = { 0 };

static inline void write_apic_u32(uint32_t off, uint32_t val)
{
	uint32_t *vptr = (uint32_t *) (APICBASE + off);
	*vptr = val;
}

static inline uint32_t read_apic_u32(uint32_t off)
{
	uint32_t *vptr = (uint32_t *) (APICBASE + off);
	return *vptr;
}

static void __interrupt
apic_spurious_handler(isr_frame_t *frame __attribute__((unused)))
{
	printf("*** apic spurious (ignored) ***\n");
}

static void __interrupt apic_timer_irq(isr_frame_t *frame __attribute__((unused)))
{
	write_apic_u32(APIC_EOI, 0);
}

static inline int apic_present(void)
{
	uint32_t a = 1, b = 0, c = 0, d = 0;
	__cpuid(&a, &b, &c, &d);
	return (d & 0x100);
}

static inline void apic_timer_reset(uint32_t mode, uint32_t value)
{
	write_apic_u32(APIC_LVT_TIMER, APIC_LVT_MASK);
	write_apic_u32(APIC_TIMER_INI, value);
	write_apic_u32(APIC_LVT_TIMER, APIC_TIMER_IRQ | mode);
	write_apic_u32(APIC_TIMER_DIV, 3);
}

static void apic_timer_init()
{
	uint32_t ticks_per_ms = 0;

	/* set common handle */
	idt_set_gate(APIC_TIMER_IRQ, (void *) apic_timer_irq);

	/* calibrate (get ticks per us) */
	apic_timer_reset(APIC_TIMER_ONESHOT, 0xffffffff);
	pit2_wait_msec(16);
	ticks_per_ms = 0xffffffff - read_apic_u32(APIC_TIMER_CNT);
	ticks_per_ms >>= 4;

	write_apic_u32(APIC_LVT_TIMER, APIC_LVT_MASK);
	pcpu_ticks_per_ms[__apicid()] = ticks_per_ms;
}

void apic_init()
{
	if (apic_present() == 0) {
		puts("*** apic not present ***\n");
		__halt();
	}

	/* relocate ap trampoline */
	memcpy((void *)APTRAMPOLINE, (void *)(BOOTSEG<<4), 512);

	/* mask pic interruptions */
	pic_set_slave_mask(0xff);
	pic_set_master_mask(0xff);

	/* ensure DFR, LDR are on default */
	write_apic_u32(APIC_DFR, 0xffffffff);
	write_apic_u32(APIC_LDR, read_apic_u32(APIC_LDR) & 0x00ffffff);

	/* init lvt */
	write_apic_u32(APIC_LVT_TIMER, APIC_LVT_MASK); // ignore
	write_apic_u32(APIC_LVT_THERM, 0x400);   // nmi
	write_apic_u32(APIC_LVT_PERFC, 0x400);   // nmi
	write_apic_u32(APIC_LVT_LINT0, APIC_LVT_MASK); // ignore
	write_apic_u32(APIC_LVT_LINT1, APIC_LVT_MASK); // ignore

	/* set tpr=0 */
	write_apic_u32(APIC_TPR, 0);

	/* set spurious handler */
	idt_set_gate(APIC_SPURIOUS_IRQ, (void *) apic_spurious_handler);
	write_apic_u32(APIC_SIVR, 0x100 | APIC_SPURIOUS_IRQ);

	apic_timer_init();

	/* enable apic */
	__wrmsr(IA32_APIC_BASE, __rdmsr(IA32_APIC_BASE) | APIC_BASE_APIC_ENABLE);
	__wrmsr(IA32_APIC_BASE, __rdmsr(IA32_APIC_BASE) & ~APIC_BASE_x2APIC_ENABLE);
	__wrmsr(IA32_APIC_BASE, (__rdmsr(IA32_APIC_BASE) & 0x00000fff) | 0xfee00000);
}

void apic_timer_wait_ms(uint32_t msec)
{
	uint32_t ticks_per_ms = pcpu_ticks_per_ms[__apicid()];
	uint32_t tm_max_ms = 0xffffffff / ticks_per_ms;

	for (uint32_t i = 0; i < (msec / tm_max_ms); i++) {
		apic_timer_reset(APIC_TIMER_ONESHOT, tm_max_ms * ticks_per_ms);
		__asm__ volatile("hlt");
	}

	apic_timer_reset(APIC_TIMER_ONESHOT, (msec % tm_max_ms) * ticks_per_ms);
	__asm__ volatile("hlt");
}

static int ipi_mode_valid(uint32_t mode)
{
	switch (mode) {
		case IPI_MODE_FIXED:
		case IPI_MODE_SMI:
		case IPI_MODE_NMI:
		case IPI_MODE_INIT:
		case IPI_MODE_STARTUP:
			return 1;
	}

	return 0;
}

static int ipi_sh_valid(uint32_t shorthand)
{
	switch (shorthand) {
		case 0: /* no shorthand */
		case IPI_SELF:
		case IPI_ALL:
		case IPI_OTHERS:
			return 1;
	}

	return 0;
}

static void apic_wait_icr0_idle()
{
	if ((read_apic_u32(APIC_ICR0) & 0x1000) == 0) {
		return;
	}

	apic_timer_wait_ms(10);

	if ((read_apic_u32(APIC_ICR0) & 0x1000) != 0) {
		printf("bug: apic_wait_icr0_idle\n");
		__halt();
	}
}

void apic_send_ipi(uint8_t id, uint32_t shorthand, uint32_t mode, uint8_t vector)
{
	/* sanity checks */
	if ((id > MAXCPU) || !ipi_mode_valid(mode) || !ipi_sh_valid(shorthand)) {
		printf("*** bug: apic_send_ipi: invalid argument ***\n");
		printf("id=0x%x, mode=0x%x, shorthand=0x%x, vector=0x%x\n", id, mode,
			shorthand, vector);
		__halt();
	}

	/* send ipi */
	apic_wait_icr0_idle();
	write_apic_u32(APIC_ICR1, ((uint32_t) id) << 24);
	write_apic_u32(APIC_ICR0, shorthand | 0x4000 | mode | vector);
	apic_wait_icr0_idle();
}

void apic_init_thread(uint8_t id, void (*startup32)(void))
{
	uint32_t *apmem_startup32 = (uint32_t *)(APTRAMPOLINE + STARTUP32_OFFSET);
	*apmem_startup32 = (uint32_t) startup32;

	apic_send_ipi(id, 0, IPI_MODE_INIT, 0);
	apic_send_ipi(id, 0, IPI_MODE_STARTUP, (APTRAMPOLINE >> 12));
}
