/*
 * pit.h
 */

#ifndef PIT_H
#define PIT_H

#include "inttypes.h"

#define PIT_MODE_0 0 /* Interrupt On Terminal Count */
#define PIT_MODE_1 1 /* Hardware Re-triggerable One-shot */
#define PIT_MODE_2 2 /* Rate Generator */
#define PIT_MODE_3 3 /* Square Wave Generator */
#define PIT_MODE_4 4 /* Software Triggered Strobe */
#define PIT_MODE_5 5 /* Hardware Triggered Strobe */


void pit2_wait_msec(uint16_t msec);
int pit_write(uint8_t ch, uint8_t mode, uint16_t val);
int pit_read(uint8_t ch);
int pit_stat(uint8_t ch);

#endif
