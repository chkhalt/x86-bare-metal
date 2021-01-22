/*
 * pit.c
 */

#include "io.h"
#include "cpu.h"
#include "pit.h"

#define PIT0_DATA 0x40
#define PIT1_DATA 0x41
#define PIT2_DATA 0x42
#define PIT_M_CMD 0x43


static inline void pit2_wait_ms()
{
	outb(SYS_CTRL_PORTB, inb(SYS_CTRL_PORTB) & ~CTRL_B_FLG_SDE);
	outb(SYS_CTRL_PORTB, inb(SYS_CTRL_PORTB) | CTRL_B_FLG_T2I);

	/* adjust pit 2 to 1KHz, one-shot */
	pit_write(2, PIT_MODE_1, 0x4a9);

	/* set pit 2 gate (low/high) */
	outb(SYS_CTRL_PORTB, inb(SYS_CTRL_PORTB) & ~CTRL_B_FLG_T2I);
	outb(SYS_CTRL_PORTB, inb(SYS_CTRL_PORTB) | CTRL_B_FLG_T2I);

	while (1) {
		if ((inb(SYS_CTRL_PORTB) & 0x20) == 0) {
			break;
		}
	}
}

void pit2_wait_msec(uint16_t msec)
{
	while (msec--) {
		pit2_wait_ms();
	}
}

int pit_write(uint8_t ch, uint8_t mode, uint16_t val)
{
	/* sanity check */
	if ((ch == 1) || (ch > 2) || (mode > 5)) {
		return -1;
	}

	/* access mode: lobyte/hibyte */
	outb(PIT_M_CMD, (ch << 6) | 0x30 | (mode << 1));

	/* set data */
	outb(PIT0_DATA + ch, val);
	outb(PIT0_DATA + ch, val >> 8);

	return 0;
}

int pit_read(uint8_t ch)
{
	uint8_t low = 0, high = 0;

	/* sanity check */
	if ((ch == 1) || (ch > 2)) {
		return -1;
	}

	/* access mode: latch */
	outb(PIT_M_CMD, (ch << 6));
	/* read low, high*/
	low = inb(PIT0_DATA + ch);
	high = inb(PIT0_DATA + ch);

	return ((uint16_t) high << 8) | low;
}

int pit_stat(uint8_t ch)
{
	/* sanity check */
	if ((ch == 1) || (ch > 2)) {
		return -1;
	}

	/* channel: read back */
	outb(PIT_M_CMD, 0xe0 | (1 << (ch + 1)));
	return inb(PIT0_DATA + ch);
}

