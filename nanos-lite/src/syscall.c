#include "common.h"
#include "syscall.h"

int sys_none()
{
  return 1;
}

void sys_exit(int code)
{
  _halt(code);
}

//pa3 level2: add sys_write
size_t sys_write(int fd, void *buf, size_t len)
{
  if (fd == 1 || fd == 2)
  {
    //char a;
    Log("output: %s", (char *)buf);
    for (int i = 0; i < len; i++)
    {
      _putc(((char *)buf)[i]);
    }
    return len;
  }
  // if (fd >= 3)
  //   return fs_write(fd, buf, len);
  // Log("fd<=0");
  return -1;
}

//pa3 level2: add sys_brk
int sys_brk(uintptr_t addr)
{
  // Log("sys_brk");
  return 0;
}

_RegSet *do_syscall(_RegSet *r)
{
  //pa3 level1: add
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0])
  {
  case SYS_none:
    SYSCALL_ARG1(r) = sys_none();
    break;
  case SYS_exit:
    sys_exit(a[1]);
    break;
  case SYS_write:
    SYSCALL_ARG1(r) = sys_write(a[1], (void *)a[2], a[3]);
    break;
  case SYS_brk:
    SYSCALL_ARG1(r) = 0;
    break;

  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
