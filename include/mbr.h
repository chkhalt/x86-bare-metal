/*
 * mbr.h
 */

#ifndef MBR_H
#define MBR_H

#include <stdint.h>

#pragma pack(push, 1)
struct partentry {
	uint8_t  status;
	uint8_t  start_head;
	uint8_t  start_sector;
	uint8_t  start_track;
	uint8_t  type;
	uint8_t  end_head;
	uint8_t  end_sector;
	uint8_t  end_track;
	uint32_t start_index;
	uint32_t numof_sectors;
};

#define MBR_BOOT_CODE_SZ 440
struct mbr {
	uint8_t boot_code[MBR_BOOT_CODE_SZ];
	uint8_t uniq_id[4];
	uint8_t res[2];

	struct partentry part_table[4];
	uint16_t signature; /* 0xaa55 */
};
#pragma pack(pop)

static inline void lba2chs(uint32_t lbavalue, uint8_t hst[3])
{
	uint8_t c, h, s;

	s = (lbavalue % 63) + 1;
	lbavalue = lbavalue / 63;
	h = lbavalue % 255;
	lbavalue = lbavalue / 255;
	c = lbavalue % 1023;

	s |= (c & (3 << 8));
	c &= 255;

	hst[0] = h;
	hst[1] = s;
	hst[2] = c;
}

#endif /* !MBR_H */
