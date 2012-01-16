#ifndef __LIBC__MEMORY_H__
#define __LIBC__MEMORY_H__

void* malloc(size_t len);
void free(void* ptr);

void* mmap(void* addr, size_t len, int prot, int flags, int fd, uint64_t offset);
void* munmap(void* addr, size_t len);

#endif

