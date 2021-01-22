/*
 * paging.h
 */

#ifndef PAGING_H
#define PAGING_H

#define MAXVIRTADDR 0x400000

void init_early_pages(void);
void set_paging_on(void);
void set_paging_off(void);
//void early_range_map(void *phys, void *virt);
//void early_range_map_uc(void *phys, void *virt);

#endif
