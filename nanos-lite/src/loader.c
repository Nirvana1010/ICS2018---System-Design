#include "common.h"
#include "fs.h"
#include "memory.h"

#define DEFAULT_ENTRY ((void *)0x8048000)
extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
extern void* new_page();
extern void _map(_Protect *p, void *va, void *pa);

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  
	//ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());

	/*
	int fd = fs_open(filename, 0, 0);
	Log("filename = %s, fd = %d", filename, fd);
	fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));
	fs_close(fd);
	*/
	
	int fd = fs_open(filename, 0, 0);
	int len = fs_filesz(fd);
	void *va, *pa;
	void* end = DEFAULT_ENTRY + len;
	Log("load file: %s, fd: %d, size: %d\n", filename, fd, len);
	for(va = DEFAULT_ENTRY; va < end; va += PGSIZE) {
		//Log("va: 0x%x", va);
		pa = new_page();
		_map(as, va, pa);
		fs_read(fd, pa, PGSIZE);
	}
	//fs_close(fd);
	
  return (uintptr_t)DEFAULT_ENTRY;
}
