#include "fs.h"

// extern void ramdisk_read(void *buf, off_t offset, size_t len);
// extern void ramdisk_write(void *buf, off_t offset, size_t len);
// extern void dispinfo_read(void *buf, off_t offset, size_t len);
// extern void fb_write(const void *buf, off_t offset, size_t len);
// extern size_t events_read(void *buf, size_t len);

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

extern void getScreen(int *p_width, int *p_height);

//pa3 level2: change Finfo
typedef struct
{
  char *name;        //文件名
  size_t size;       //文件大小
  off_t disk_offset; //在ramdisk中偏移量
  off_t open_offset; //打开文件的当前操作位置
} Finfo;

enum
{
  FD_STDIN,
  FD_STDOUT,
  FD_STDERR,
  FD_FB,
  FD_EVENTS,
  FD_DISPINFO,
  FD_NORMAL
};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    {"stdin (note that this is not the actual stdin)", 0, 0},
    {"stdout (note that this is not the actual stdout)", 0, 0},
    {"stderr (note that this is not the actual stderr)", 0, 0},
    [FD_FB] = {"/dev/fb", 0, 0},
    [FD_EVENTS] = {"/dev/events", 0, 0},
    [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs()
{
  // TODO: initialize the size of /dev/fb
  //pa3 level3
  file_table[FD_FB].size = _screen.height * _screen.width * 4;
}

/*
size_t fs_filesz(int fd)
{
  assert(fd >= 0 && fd < NR_FILES);
  return file_table[fd].size;
}

off_t disk_offset(int fd)
{
  assert(fd >= 0 && fd < NR_FILES);
  return file_table[fd].disk_offset;
}

off_t get_open_offset(int fd)
{
  assert(fd >= 0 && fd < NR_FILES);
  return file_table[fd].open_offset;
}

void set_open_offset(int fd, off_t n)
{
  assert(fd >= 0 && fd < NR_FILES);
  assert(n >= 0);
  if (n > file_table[fd].size)
    n = file_table[fd].size;
  file_table[fd].open_offset = n;
}

//pa3 level2 : add fs_open
int fs_open(const char *pathname, int flags, mode_t mode)
{
  //Log("Pathname: %s.", pathname);
  for (int i = 0; i < NR_FILES; i++)
  {
    if (strcmp(file_table[i].name, pathname) == 0)
    {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  // assert(0);
  panic("this filename not exists!");
  return -1;
}

//pa3 level2 : add fs_read
ssize_t fs_read(int fd, void *buf, size_t len)
{
  assert(fd >= 0 && fd < NR_FILES);
  if (fd < 3 || fd == FD_FB)
  {
    Log("error:fd<3 || fd==FD_FB");
    return 0;
  }
  //pa3 level3:在文件系统的 fs_read 函数中添加对/dev/events 的支持。
  if (fd == FD_EVENTS)
  {
    return events_read(buf, len);
  }
  int n = fs_filesz(fd) - get_open_offset(fd);
  if (n > len)
  {
    n = len;
  }

  //pa3 level3:在 fs_read()的实现中对 FD_DISPINFO 进行"重定向"。
  if (fd == FD_DISPINFO)
  {
    dispinfo_read(buf, get_open_offset(fd), n);
  }
  else
  {
    ramdisk_read(buf, disk_offset(fd) + get_open_offset(fd), n);
  }
  set_open_offset(fd, get_open_offset(fd) + n);
  return n;
}

//pa3 level2: add fs_write
ssize_t fs_write(int fd, void *buf, size_t len)
{
  assert(fd >= 0 && fd < NR_FILES);
  if (fd < 3 || fd == FD_DISPINFO)
  {
    Log("error:fd<3 || fd==FD_DISPINFO");
    return 0;
  }
  int n = fs_filesz(fd) - get_open_offset(fd);
  if (n > len)
  {
    n = len;
  }
  if (fd == FD_FB)
  {
    fb_write(buf, get_open_offset(fd), n);
  }
  else
    ramdisk_write(buf, disk_offset(fd) + get_open_offset(fd), n);

  set_open_offset(fd, get_open_offset(fd) + n);
  return n;
}

//pa3 level2 : add fs_close
int fs_close(int fd)
{
  assert(fd >= 0 && fd < NR_FILES);
  return 0;
}

//pa3 level2: add fs_lseek
off_t fs_lseek(int fd, off_t offset, int whence)
{
  switch (whence)
  {
  case SEEK_SET:
    set_open_offset(fd, offset);
    return get_open_offset(fd);
  case SEEK_CUR:
    set_open_offset(fd, get_open_offset(fd) + offset);
    return get_open_offset(fd);
  case SEEK_END:
    set_open_offset(fd, fs_filesz(fd) + offset);
    return get_open_offset(fd);
  default:
    panic("whence error.\n");
    return -1;
  }
}
*/

//pa4 level1 revise fs

int fs_open(const char *path, int flags, int mode)
{
  Log("Pathname: %s", path);
  for (int i = 0; i < NR_FILES; i++)
  {
    if (strcmp(file_table[i].name, path) == 0)
    {
      return i;
    }
  }
  assert(0);
  return -1;
}

size_t fs_filesz(int fd)
{
  return file_table[fd].size;
}

ssize_t fs_read(int fd, void *buf, size_t len)
{
  ssize_t f_size = fs_filesz(fd);
  if (file_table[fd].open_offset + len > f_size)
  {
    len = f_size - file_table[fd].open_offset;
  }
  switch (fd)
  {
  case FD_STDIN:
  case FD_STDOUT:
  case FD_STDERR:
  {
    return 0;
    break;
  }
  case FD_EVENTS:
  {
    len = events_read(buf, len);
    break;
  }
  case FD_DISPINFO:
  {
    dispinfo_read(buf, file_table[fd].open_offset, len);
    file_table[fd].open_offset += len;
    break;
  }
  default:
  {
    // Log("Reading %s from %d..open_offset:%d,disk_offset:%d,len:%d",
    // file_table[fd].name,
    // file_table[fd].disk_offset + file_table[fd].open_offset,
    // file_table[fd].open_offset,
    // file_table[fd].disk_offset,
    // len);
    ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
    file_table[fd].open_offset += len;
    // Log("Read finish..");
    break;
  }
  }
  return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len)
{
  ssize_t f_size = fs_filesz(fd);
  switch (fd)
  {
  case FD_STDOUT:
  case FD_STDERR:
  {
    // stdout stderr
    for (int i = 0; i < len; i++)
    {
      _putc(((char *)buf)[i]);
    }
    break;
  }
  case FD_FB:
  {
    // frame buffer
    fb_write(buf, file_table[fd].open_offset, len);
    file_table[fd].open_offset += len;
    break;
  }
  default:
  {
    if (file_table[fd].open_offset + len > f_size)
    {
      len = f_size - file_table[fd].open_offset;
    }
    // Log("Writing %s..open_offset:%d,disk_offset:%d,len:%d",
    // file_table[fd].name,
    // file_table[fd].open_offset,
    // file_table[fd].disk_offset,
    // len);
    ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
    file_table[fd].open_offset += len;
    break;
  }
  }
  return len;
}

off_t fs_lseek(int fd, off_t offset, int whence)
{
  off_t ret = -1;
  switch (whence)
  {
  case SEEK_SET:
  {
    if (offset >= 0 && offset <= file_table[fd].size)
    {
      file_table[fd].open_offset = offset;
      ret = file_table[fd].open_offset;
    }
    break;
  }
  case SEEK_CUR:
  {
    if (offset + file_table[fd].open_offset >= 0 && offset + file_table[fd].open_offset <= file_table[fd].size)
    {
      file_table[fd].open_offset += offset;
      ret = file_table[fd].open_offset;
    }
    break;
  }
  case SEEK_END:
  {
    file_table[fd].open_offset = file_table[fd].size + offset;
    ret = file_table[fd].open_offset;
    break;
  }
  default:
  {
    Log("undefined whence..");
    assert(0);
  }
  }
  return ret;
}

int fs_close(int fd)
{
  Log("Closing %s with fd:%d..",
      file_table[fd].name,
      fd);
  return 0;
}
