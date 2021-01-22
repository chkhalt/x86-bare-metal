/*
 * paging.c
 */

#include "cpu.h"
#include "lapic.h"
#include "compiler.h"
#include "inttypes.h"
#include "paging.h"

#define PAGE_SIZE    4096
#define PAGE_FLG_P   1
#define PAGE_FLG_W   2
#define PAGE_FLG_U   4
#define PAGE_FLG_PWT 8
#define PAGE_FLG_PCD 16

typedef uint32_t pde32_t;
typedef uint32_t pte32_t;

typedef struct _pde32_table {
	pde32_t entry[1024];
} __align(PAGE_SIZE) pde32_table_t ;

typedef struct _pte32_table {
	pte32_t entry[1024];
} __align(PAGE_SIZE) pte32_table_t;

/* only one address space pre cpu */
pde32_table_t __use_section_data per_cpu_pde[MAXCPU] = { 0 };
pte32_table_t __use_section_data per_cpu_pte[MAXCPU] = { 0 };

/* pte to map lapic area */
pte32_table_t __use_section_data per_cpu_lapic_pte[MAXCPU] = { 0 };


/* set 4M identity mapping */
void init_early_pages(void)
{
	uint8_t apicid = __apicid();
	pte32_t pte = 0;
	pde32_t pde = ((pde32_t) &per_cpu_pte[apicid]) | PAGE_FLG_P | PAGE_FLG_W;

	/* set pde, first 4M, lapic */
	per_cpu_pde[apicid].entry[0] = pde;

	pde = ((pde32_t) &per_cpu_lapic_pte[apicid]) | PAGE_FLG_P | PAGE_FLG_W;
	per_cpu_pde[apicid].entry[(uint32_t)APICBASE >> 22] = pde;

	/* set pte 4M, 4K pages */
	for (uint32_t i = 0; i < (sizeof(pte32_table_t) / sizeof(pte32_t)); i++) {
		pte = ((pte32_t) (i * PAGE_SIZE)) | PAGE_FLG_P | PAGE_FLG_W;
		per_cpu_pte[apicid].entry[i] = pte;
	}

	/* set pte lapic (4K page) */
	pte = (uint32_t)APICBASE | PAGE_FLG_P | PAGE_FLG_W | PAGE_FLG_PWT | PAGE_FLG_PCD;
	per_cpu_lapic_pte[apicid].entry[((uint32_t)APICBASE >> 12) & 0x3ff] = pte;

	/* set cr3 */
	__writecr3((long) &per_cpu_pde[apicid]);
}

#if 0
static inline void
_invlpg(void *memptr)
{
	__asm__ volatile ("invlpg %0" :: "m" (memptr));
}

/* map page frame inside the early 4M space */
void early_range_map(void *phys, void *virt)
{
	uint8_t apicid = __apicid();

	/* ensure 4K alignment */
	uint32_t p = ((uint32_t) phys) & 0xfffff000;
	uint32_t v = ((uint32_t) virt) & 0x003ff000; /* wrap around 4M */

	/* map page frame */
	per_cpu_pte[apicid].entry[v >> 12] = p | PAGE_FLG_P | PAGE_FLG_W;
	_invlpg(virt);
}

void early_range_map_uc(void *phys, void *virt)
{
	uint8_t apicid = __apicid();

	/* ensure 4K alignment */
	uint32_t p = ((uint32_t) phys) & 0xfffff000;
	uint32_t v = ((uint32_t) virt) & 0x003ff000; /* wrap around 4M */

	/* map page frame */
	per_cpu_pte[apicid].entry[v >> 12] = p | PAGE_FLG_P | PAGE_FLG_W |
	PAGE_FLG_PWT | PAGE_FLG_PCD;
	_invlpg(virt);
}
#endif

void set_paging_on(void)
{
	/* enable */
	__writecr0(__readcr0() | CR0_PG);
}

void set_paging_off(void)
{
	/* disable */
	__writecr0(__readcr0() & ~CR0_PG);
}

