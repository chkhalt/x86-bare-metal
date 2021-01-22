/*
 * lapic.h - local apic
 */

#ifndef LAPIC_H
#define LAPIC_H

#include "inttypes.h"

#define APICBASE 0xfee00000

/* ipi modes */
#define IPI_MODE_FIXED   0x0000
#define IPI_MODE_SMI     0x0200
#define IPI_MODE_NMI     0x0400
#define IPI_MODE_INIT    0x0500
#define IPI_MODE_STARTUP 0x0600

/* ipi shorthands */
#define IPI_SELF         0x40000
#define IPI_ALL          0x80000 /* all, including self */
#define IPI_OTHERS       0xC0000 /* all, excluding self */

void apic_init();
void apic_timer_wait_ms(uint32_t msec);
void apic_send_ipi(uint8_t id, uint32_t shorthand, uint32_t mode, uint8_t vector);
void apic_init_thread(uint8_t id, void (*startup32)(void));

#endif
