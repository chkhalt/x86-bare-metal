/*
 * idt.h
 */

#ifndef IDT_H
#define IDT_H

#include "inttypes.h"

typedef struct _isr_frame {
#ifdef __x86_64__
	uint64_t ip, cs, flags;
#else
	uint32_t ip, cs, flags;
#endif
} isr_frame_t;

typedef void (*isr_handler_t)(isr_frame_t *frame);

void idt_init(void);
void idt_set_gate(uint8_t num, void *isr);

#endif
