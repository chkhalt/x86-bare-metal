/*
 * pic.h
 */

#ifndef IRQ_H
#define IRQ_H

#include "inttypes.h"

/* PIC real mode offsets */
#define PIC1_REAL_M_OFFSET  0x08
#define PIC2_REAL_M_OFFSET  0x70

/* PIC protected mode offsets */
#define PIC1_PROT_M_OFFSET  0x20 /* 0x20 - 32. IRQ0 fires int 32 */
#define PIC2_PROT_M_OFFSET  0x28

void pic_set_master_mask(uint8_t mask);
void pic_set_slave_mask(uint8_t mask);
void pic_send_eoi(unsigned char irq);
void pic_irq_remap(uint8_t offset1, uint8_t offset2);

#endif
