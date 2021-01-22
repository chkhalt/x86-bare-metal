/*
 * video.h
 */

#ifndef VIDEO_H
#define VIDEO_H

void putchar(char c);
void puts(const char *s);
void printf(const char *fmt, ...)
	__attribute__((format (__printf__, 1, 2)));

#endif
