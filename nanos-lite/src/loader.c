#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

//pa3 level1 add extern function
extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void *new_page(void);

//pa3 level2 add fs function
int fs_open(const char *pathname, int flags, mode_t mode);
size_t fs_filesz(int fd);
ssize_t fs_read(int fd, void *buf, size_t len);
int fs_close(int fd);

uintptr_t loader(_Protect *as, const char *filename)
{
  //pa3 level1 :add loader
  //ramdisk_read函数：从ramdisk中offset偏移处的len字节读入到buf中
  // ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  // return (uintptr_t)DEFAULT_ENTRY;

  //pa3 level2 change loader
  int fd = fs_open(filename, 0, 0);
  // Log("Filename = %s,fd = %d", filename, fd);
  // fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));

  //pa4 level1: change loader

  int f_size = fs_filesz(fd);
  Log("Load %d bytes file, named %s, fd %d", f_size, filename, fd);
  void *pa = DEFAULT_ENTRY;
  void *va = DEFAULT_ENTRY;
  while (f_size > 0)
  {
    pa = new_page();
    _map(as, va, pa);
    fs_read(fd, pa, PGSIZE);

    va += PGSIZE;
    f_size -= PGSIZE;
    // Log("f_size remaining:%d..",f_size);
  }

  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
