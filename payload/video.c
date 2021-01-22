/*
 * video.c
 */

#include "io.h"
#include "video.h"
#include "compiler.h"
#include "inttypes.h"
#include "string.h"

#include <stdarg.h>

#define VIDEOMEM   0xb8000
#define MAXCOLUMNS 80
#define MAXLINES   25

uint8_t __use_section_data curx = 0;
uint8_t __use_section_data cury = 0;

enum {
	PRINTF_FORMAT_CHAR,
	PRINTF_FORMAT_STR,
	PRINTF_FORMAT_INT,
	PRINTF_FORMAT_UINT,
	PRINTF_FORMAT_LONG,
	PRINTF_FORMAT_ULONG,
	PRINTF_FORMAT_HEX,
	PRINTF_FORMAT_LONGHEX,
	PRINTF_FORMAT_UNKNOWN
};

static void setcursor(void)
{
	uint16_t position = ((cury * MAXCOLUMNS) + curx);

	/* low byte */
	outb(0x03D4, 0x0F);
	outb(0x03D5, (uint8_t) position);

	/* high byte */
	outb(0x03D4, 0x0E);
	outb(0x03D5, (uint8_t) (position >> 8));
}

static inline void _cpy8char(uint16_t dstpos, uint16_t srcpos)
{
	char *video = (char *) VIDEOMEM;
	memcpy(&video[dstpos], &video[srcpos], 8 * 2);
}

static inline void _cpyline(uint8_t dstn, uint8_t srcn)
{
	uint16_t dstpos, srcpos;

	for (int i = 0; i < MAXCOLUMNS; i += 8) {
		dstpos = ((dstn * MAXCOLUMNS) + i) * 2;
		srcpos = ((srcn * MAXCOLUMNS) + i) * 2;
		_cpy8char(dstpos, srcpos);
	}
}

static void setlinesup(void)
{
	for (int i = 0; i < MAXLINES; i++) {
		_cpyline(i, i + 1);
	}
}

static void incy(void)
{
	if (curx == 0) {
		if ((cury + 1) < MAXLINES) {
			cury++;
			return;
		}

		setlinesup();
		cury = MAXLINES - 1;
	}
}

static void incx(void)
{
	curx = ((curx + 1) % MAXCOLUMNS);
}

void putchar(char c)
{
	//__asm__ volatile(".byte 0xeb, 0xfe");

	char *video = (char *) VIDEOMEM;
	uint16_t position = ((cury * MAXCOLUMNS) + curx) * 2;

	if (c == '\n') {
		curx = 0;
		incy();
		goto _quit;
	}

	video[position] = c;
	video[position + 1] = 0x07;
	incx();
	incy();

_quit:
	setcursor();
}

void puts(const char *s)
{
	while (*s) {
		putchar(*s);
		s++;
	}
}

static void ultoa(unsigned long num, char *s, int len, unsigned int base)
{
	unsigned long digit;
	char ch, *sptr = s;

	/* convert */
	do {
		digit = num % base;
		*sptr++ = (digit < 10) ? '0' + digit : 'a' + digit - 10;
		num /= base;
	} while (--len && num);

	/* reverse */
	*sptr-- = '\0';
	while (sptr > s) {
		ch = *s;
		*s++ = *sptr;
		*sptr-- = ch;
	}
}

static int getfmt(const char **pfmt)
{
	const char *fmt = *pfmt;
	int format = PRINTF_FORMAT_UNKNOWN;

	switch (*fmt) {
		case 's':
			format = PRINTF_FORMAT_STR;
			break;

		case 'c' :
			format = PRINTF_FORMAT_CHAR;
			break;

		case 'i':
		case 'd':
			format = PRINTF_FORMAT_INT;
			break;

		case 'u':
			format = PRINTF_FORMAT_UINT;
			break;

		case 'x':
		case 'X':
			format = PRINTF_FORMAT_HEX;
			break;

		case 'l':
			format = (fmt[1] == 'd') ? PRINTF_FORMAT_LONG :
			(fmt[1] == 'u') ? PRINTF_FORMAT_ULONG :
			(fmt[1] == 'x') ? PRINTF_FORMAT_LONGHEX :
			(fmt[1] == 'X') ? PRINTF_FORMAT_LONGHEX : PRINTF_FORMAT_UNKNOWN;

			if (format != PRINTF_FORMAT_UNKNOWN) {
				(*pfmt)++;
			}

			break;
	}

	(*pfmt)++;
	return format;
}

void printf(const char *fmt, ...)
{
	char nextchar;
	char c, *s, intascii[32] = { 0 };
	unsigned long ul = 0;
	unsigned int d = 0;

	va_list arg;
	va_start(arg, fmt);

	while ((nextchar = *fmt++)) {
		if (nextchar != '%') {
			putchar(nextchar);
			continue;
		}

		switch (getfmt(&fmt)) {
			case PRINTF_FORMAT_CHAR:
				c = va_arg(arg, int);
				putchar(c);
				break;

			case PRINTF_FORMAT_STR:
				s = va_arg(arg, char *);
				puts(s);
				break;

			case PRINTF_FORMAT_LONG:
				ul = va_arg(arg, unsigned long);
				if (ul & (1 << ((sizeof(long) * 8) - 1))) {
					putchar('-');
					ul = ~ul + 1;
				}

				ultoa(ul, intascii, sizeof(intascii), 10);
				puts(intascii);
				break;

			case PRINTF_FORMAT_ULONG:
				ul = va_arg(arg, unsigned long);
				ultoa(ul, intascii, sizeof(intascii), 10);
				puts(intascii);
				break;

			case PRINTF_FORMAT_INT:
				d = va_arg(arg, unsigned int);
				if (d & (1 << ((sizeof(int) * 8) - 1))) {
					putchar('-');
					d = ~d + 1;
				}

				ultoa(d, intascii, sizeof(intascii), 10);
				puts(intascii);
				break;

			case PRINTF_FORMAT_UINT:
				d = va_arg(arg, unsigned int);
				ultoa(d, intascii, sizeof(intascii), 10);
				puts(intascii);
				break;

			case PRINTF_FORMAT_HEX:
				d = va_arg(arg, int);
				ultoa(d, intascii, sizeof(intascii), 16);
				puts(intascii);
				break;

			case PRINTF_FORMAT_LONGHEX:
				ul = va_arg(arg, unsigned long);
				ultoa(ul, intascii, sizeof(intascii), 16);
				puts(intascii);
				break;

			case -1:
				goto _quit;

			default:
				putchar('%');
				putchar(*fmt);
		}
	}

_quit:
	va_end(arg);
}

