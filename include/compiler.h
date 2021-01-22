/*
 * compiler.h - compiler specific defs
 */

#ifndef COMPILER_H
#define COMPILER_H

#if defined (__APPLE__)
#define __TEXT_NAME__ __TEXT
#define __TEXT_FLAGS__ __text,regular,pure_instructions

#define __DATA_NAME__ __DATA
#define __DATA_FLAGS__ __data

#define LABEL(n) _ ##n

#elif defined (__linux__)
#define __TEXT_NAME__ .text
#define __TEXT_FLAGS__ "ax",@progbits

#define __DATA_NAME__ .data
#define __DATA_FLAGS__ "aw",@progbits#

#define LABEL(n) n
#endif

#define __STR(...) #__VA_ARGS__

#define __USE_SECTION(name, flags) \
	__attribute__((section(__STR(name,flags))))

#define __use_section_data \
	__USE_SECTION(__DATA_NAME__,__DATA_FLAGS__)

#define __align(n) __attribute__((aligned(n)))
#define __trap() while(1)
#define __halt() while (1) __asm__ volatile ("hlt")

#define __interrupt __attribute__ ((interrupt))

#endif /* COMPILER_H */
