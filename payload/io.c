/*
 * io.c
 */

#include "inttypes.h"

uint8_t inb(uint16_t port)
{
	uint8_t val;
	__asm__ volatile("in %%dx,%%al;": "=a"(val) : "d"(port));
    return val;
}

uint16_t inw(uint16_t port)
{
	uint16_t val;
    __asm__ volatile("inw %w1, %0" : "=a" (val) : "Nd" (port));
    return val;
}

uint32_t inl(uint16_t port)
{
	uint32_t val;
    __asm__ volatile("inl %w1, %0" : "=a" (val) : "Nd" (port));
    return val;
}

void outb(uint16_t port, uint8_t val)
{
	__asm__ volatile("out %%al, %%dx;"::"a"(val), "d"(port));
}

void outw(uint16_t port, uint16_t val)
{
	__asm__ volatile("outw %0, %w1" :: "a" (val), "Nd" (port));
}

void outl(uint16_t port, uint32_t val)
{
	__asm__ volatile("outl %0, %w1" : : "a" (val), "Nd" (port));
}
