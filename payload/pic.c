/*
 * pic.c
 */

#include "io.h"
#include "pic.h"

#define PIC1            0x20  /* IO base address for master PIC */
#define PIC2            0xA0  /* IO base address for slave PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)
#define ICW1_INIT       0x10
#define ICW1_ICW4       0x01
#define ICW4_8086       0x01
#define PIC_EOI         0x20


void pic_set_master_mask(uint8_t mask)
{
    outb(PIC1_DATA, mask);
}

void pic_set_slave_mask(uint8_t mask)
{
    outb(PIC2_DATA, mask);
}

void pic_send_eoi(unsigned char irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

/* from osdev: https://wiki.osdev.org/8259_PIC */
void pic_irq_remap(uint8_t offset1, uint8_t offset2)
{
    /* save masks */
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  /* PIC inicialization - Master */
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);  /* PIC inicialization - Slave */
    outb(PIC1_DATA, offset1);                   /* ICW2 - IRQ0 fires 'int 32' - Master */
    outb(PIC2_DATA, offset2);                   /* ICW2 - IRQ8 fires 'int 40' - Slave */
    outb(PIC1_DATA, 0x04);                      /* ICW3 - slave PIC in IRQ2 - 0x04 (0100) */

    outb(PIC2_DATA, 0x02);                      /* ICW3 - master PIC in slave pic IRQ1 */
    outb(PIC1_DATA, ICW4_8086);                 /* ICW4 - */
    outb(PIC2_DATA, ICW4_8086);                 /* ICW4 - */
    outb(0x21, 0x0);                            /* ICW3 - */
    outb(0xA1, 0x0);                            /* ICW3 - */

    // restore saved masks
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}
