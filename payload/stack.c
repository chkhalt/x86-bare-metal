/*
 * compiler.c
 */

#include "video.h"
#include "compiler.h"


unsigned long __use_section_data __stack_chk_guard = 0;

void __attribute__((noreturn)) __stack_chk_fail(void)
{
	puts("*** stack smashing detected ***\n");
	__trap();
}
