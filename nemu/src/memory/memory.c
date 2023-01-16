#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t page_translate(vaddr_t addr, bool iswrite);

bool is_cross_bondary(vaddr_t addr, int len) {
	bool result;
	result = (((addr+len-1) & ~PAGE_MASK) != (addr & ~PAGE_MASK)) ? true : false;
	return result;
}

uint32_t paddr_read(paddr_t addr, int len) {
  int map;
	map = is_mmio(addr);
	if(map != -1)
		return mmio_read(addr, len, map);
	else
		return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
	int map;
	map = is_mmio(addr);
	if(map != -1)
		mmio_write(addr, len, data, map);
	else
  	memcpy(guest_to_host(addr), &data, len);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
	paddr_t paddr;
	if(cpu.cr0.paging) {
		if(is_cross_bondary(addr, len)) {
			union {
				uint8_t bytes[4];
				uint32_t dword;
			} data = {0};
			for(int i = 0; i < len; i++) {
				paddr = page_translate(addr+i, false);
				data.bytes[i] = paddr_read(paddr, 1);
			}
			return data.dword;
		}
		else {
			paddr = page_translate(addr, false);
			return paddr_read(paddr, len);
		}
	}
	else
 		return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
	paddr_t paddr;
	if(cpu.cr0.paging) {
		if(is_cross_bondary(addr, len)) {
			for(int i = 0; i < len; i++) {
				paddr = page_translate(addr+i, true);
				paddr_write(paddr, 1, data);
				data >>= 8;
			}
		}
		else {
			paddr = page_translate(addr, true);
			return paddr_write(paddr, len, data);
		}
	}
	else
  	paddr_write(addr, len, data);
}

uint32_t page_translate(vaddr_t addr, bool iswrite)
{
	PDE pde, *pgdir;
	PTE pte, *pgtable;
	paddr_t paddr = addr;
	if(cpu.cr0.protect_enable && cpu.cr0.paging) {
		pgdir = (PDE*)(intptr_t)(cpu.cr3.page_directory_base << 12);
		pde.val = paddr_read((intptr_t)&pgdir[(addr >> 22)&0x3ff], 4);
		//Log("pde.val: 0x%x", pde.val);
		assert(pde.present);
		pde.accessed = 1;
		
		pgtable = (PTE*)(intptr_t)(pde.page_frame << 12);
		pte.val = paddr_read((intptr_t)&pgtable[(addr >> 12)&0x3ff], 4);
		//Log("pte.val: 0x%x", pte.val);
		assert(pte.present);
		pte.dirty = iswrite ? 1 : pte.dirty;
		paddr = (pte.page_frame << 12) | (addr & PAGE_MASK);
	}
	return paddr;
	
}

