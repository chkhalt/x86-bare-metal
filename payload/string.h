/*
 * string.h
 */

#ifndef STRING_H
#define STRING_H

#include "inttypes.h"

void memcpy(void *dst, const void *src, uint32_t n);
int memcmp(const void *s1, const void *s2, uint32_t n);

#endif
