/*
 * string.c
 */

#include "string.h"
#include "inttypes.h"


void memcpy(void *dst, const void *src, uint32_t n)
{
	char *d = dst;
	const char *s = src;

	while (n--) {
		*d++ = *s++;
	}
}

int memcmp(const void *s1, const void *s2, uint32_t n)
{
	const unsigned char *p1 = s1, *p2 = s2;

	if (n != 0)
	{
		do
		{
			if (*p1++ != *p2++) {
				return (*--p1 - *--p2);
			}

		} while (--n != 0);
	}

	return 0;
}
