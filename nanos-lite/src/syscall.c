// #include "common.h"
// #include "syscall.h"

// extern int fs_open(const char *pathname, int flags, mode_t mode);
// extern int fs_read(int fd, void *buf, size_t count);
// extern int fs_write(int fd, void *buf, size_t count);
// extern off_t fs_lseek(int fd, off_t offset, int whence);
// extern int fs_close(int fd);
// extern int mm_brk(uint32_t);

// int sys_none()
// {
//   return 1;
// }

// void sys_exit(int code)
// {
//   _halt(code);
// }

// //pa3 level2: add sys_write
// size_t sys_write(int fd, void *buf, size_t len)
// {
//   if (fd == 1 || fd == 2)
//   {
//     //char a;
//     //Log("output: %s", (char *)buf);
//     for (int i = 0; i < len; i++)
//     {
//       _putc(((char *)buf)[i]);
//     }
//     return len;
//   }
//   if (fd >= 3)
//     return fs_write(fd, buf, len);
//   Log("fd<=0");
//   return -1;
// }

// //pa3 level2: add sys_brk
// //pa4 level1: change sys_brk;s
// int sys_brk(uintptr_t addr)
// {
//   // Log("sys_brk");
//   //SYSCALL_ARG1(r) = mm_brk(SYSCALL_ARG2(r));
//   return mm_brk(addr);
// }

// //pa3 level2   fs function
// int sys_open(const char *filename)
// {
//   return fs_open(filename, 0, 0);
// }

// ssize_t sys_read(int fd, void *buf, size_t len)
// {
//   return fs_read(fd, buf, len);
// }

// int sys_lseek(int fd, off_t offset, int whence)
// {
//   return fs_lseek(fd, offset, whence);
// }

// int sys_close(int fd)
// {
//   return fs_close(fd);
// }

// _RegSet *do_syscall(_RegSet *r)
// {
//   //pa3 level1: add
//   uintptr_t a[4];
//   a[0] = SYSCALL_ARG1(r);
//   a[1] = SYSCALL_ARG2(r);
//   a[2] = SYSCALL_ARG3(r);
//   a[3] = SYSCALL_ARG4(r);

//   switch (a[0])
//   {
//   case SYS_none:
//     SYSCALL_ARG1(r) = sys_none();
//     break;
//   case SYS_exit:
//     sys_exit(a[1]);
//     break;
//   case SYS_write:
//     SYSCALL_ARG1(r) = sys_write(a[1], (void *)a[2], a[3]);
//     break;
//   case SYS_brk:
//     SYSCALL_ARG1(r) = 0;
//     break;
//   case SYS_open:
//     SYSCALL_ARG1(r) = sys_open((char *)a[1]);
//     break;
//   case SYS_read:
//     SYSCALL_ARG1(r) = sys_read(a[1], (void *)a[2], a[3]);
//     break;
//   case SYS_lseek:
//     SYSCALL_ARG1(r) = sys_lseek(a[1], a[2], a[3]);
//     break;
//   case SYS_close:
//     SYSCALL_ARG1(r) = sys_close(a[1]);
//     break;

//   default:
//     panic("Unhandled syscall ID = %d", a[0]);
//   }

//   return NULL;
// }
#include "common.h"
#include "syscall.h"

int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
extern int mm_brk(uint32_t);

_RegSet *sys_none(_RegSet *r)
{
  SYSCALL_ARG1(r) = 1;
  return NULL;
}

_RegSet *sys_write(_RegSet *r)
{
  int fd = SYSCALL_ARG2(r);
  char *buf = (char *)SYSCALL_ARG3(r);
  int len = SYSCALL_ARG4(r);

  // if (fd == 1 || fd == 2)
  // {
  //   for (int i = 0; i < len; i++)
  //   {
  //     _putc(buf[i]);
  //   }
  //   SYSCALL_ARG1(r) = SYSCALL_ARG4(r);
  // }
  // return NULL;

  SYSCALL_ARG1(r) = fs_write(fd, buf, len);
  return NULL;
}

static inline _RegSet *sys_open(_RegSet *r)
{
  const char *pathname = (const char *)SYSCALL_ARG2(r);
  int flags = (int)SYSCALL_ARG3(r);
  int mode = (int)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_open(pathname, flags, mode);
  return NULL;
}
static inline _RegSet *sys_read(_RegSet *r)
{
  int fd = (int)SYSCALL_ARG2(r);
  char *buf = (char *)SYSCALL_ARG3(r);
  int len = (int)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_read(fd, buf, len);
  return NULL;
}
static inline _RegSet *sys_close(_RegSet *r)
{
  int fd = (int)SYSCALL_ARG2(r);
  SYSCALL_ARG1(r) = fs_close(fd);
  return NULL;
}
static inline _RegSet *sys_lseek(_RegSet *r)
{
  int fd = (int)SYSCALL_ARG2(r);
  off_t offset = (off_t)SYSCALL_ARG3(r);
  int whence = (int)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_lseek(fd, offset, whence);
  return NULL;
}

_RegSet *sys_exit(_RegSet *r)
{
  _halt(SYSCALL_ARG2(r));
  return NULL;
}

_RegSet *sys_brk(_RegSet *r)
{
  // SYSCALL_ARG1(r) = 0;
  SYSCALL_ARG1(r) = mm_brk(SYSCALL_ARG2(r));
  return NULL;
}

_RegSet *do_syscall(_RegSet *r)
{
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  printf("Now handling syscall ID = %d\n", a[0]);

  switch (a[0])
  {
  case SYS_none:
  {
    return sys_none(r);
    break;
  }
  case SYS_open:
  {
    return sys_open(r);
    break;
  }
  case SYS_read:
  {
    return sys_read(r);
    break;
  }
  case SYS_write:
  {
    return sys_write(r);
    break;
  }
  case SYS_exit:
  {
    return sys_exit(r);
    break;
  }
  case SYS_close:
  {
    return sys_close(r);
    break;
  }
  case SYS_lseek:
  {
    return sys_lseek(r);
    break;
  }
  case SYS_brk:
  {
    return sys_brk(r);
    break;
  }
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}