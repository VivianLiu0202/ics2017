#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)
extern size_t get_ramdisk_size();
void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);

uintptr_t loader(_Protect *as, const char *filename)
{
  //ramdisk_read函数：从ramdisk中offset偏移处的len字节读入到buf中
  ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  return (uintptr_t)DEFAULT_ENTRY;
}
