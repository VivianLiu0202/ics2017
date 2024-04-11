#include "common.h"

extern off_t fs_lseek(int fd, off_t offset, int whence);
#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [_KEY_NONE] = "NONE",
    _KEYS(NAME)};

/*
size_t events_read(void *buf, size_t len)
{
  int key = _read_key();
  char str[20];
  bool down = false;

  if (key & 0x8000)
  {
    key ^= 0x8000;
    down = true;
  }
  if (key != _KEY_NONE)
    sprintf(str, "%s %s\n", down ? "kd" : "ku", keyname[key]);
  else
    sprintf(str, "t %d\n", _uptime());

  if (strlen(str) <= len)
  {
    strncpy((char *)buf, str, strlen(str));
    return strlen(str);
  }
  Log("strlen(event)>len,return 0");
  return 0;
}*/

//pa4 revise
int current_game = 0;
size_t events_read(void *buf, size_t len)
{
  int key = _read_key();
  bool is_down = false;
  if (key & 0x8000)
  {
    key ^= 0x8000;
    is_down = true;
  }
  if (key == _KEY_NONE)
  {
    uint32_t ut = _uptime();
    sprintf(buf, "t %d\n", ut);
  }
  else
  {
    if (key & 0x8000)
    {
      key ^= 0x8000;
      is_down = true;
    }
    sprintf(buf, "%s %s\n", is_down ? "kd" : "ku", keyname[key]);
    //pa4 level5 add:
    if (key == 13 && is_down)
    {
      //F12 Down
      current_game = (current_game == 0 ? 1 : 0);
      fs_lseek(5, 0, 0);
    }
    //sprintf(buf, "%s %s\n", is_down ? "kd" : "ku", keyname[key]);
  }

  return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

//pa3 level3 add:  dispinfo_read
void dispinfo_read(void *buf, off_t offset, size_t len)
{
  strncpy(buf, dispinfo + offset, len);
}

extern void getScreen(int *p_width, int *p_height);

void fb_write(const void *buf, off_t offset, size_t len)
{
  assert(offset % 4 == 0 && len % 4 == 0);
  int index, x1, y1, y2;
  int width = 0, height = 0;
  getScreen(&width, &height);
  index = offset / 4;
  y1 = index / width;
  x1 = index % width;
  index = (offset + len) / 4;
  y2 = index / width;
  assert(y2 >= y1);
  if (y2 == y1)
  {
    _draw_rect(buf, x1, y1, len / 4, 1);
    return;
  }
  int tempw = width - x1;
  if (y2 - y1 == 1)
  {
    _draw_rect(buf, x1, y1, tempw, 1);
    _draw_rect(buf + tempw * 4, 0, y2, len / 4 - tempw, 1);
    return;
  }
  _draw_rect(buf, x1, y1, tempw, 1);
  int tempy = y2 - y1 - 1;
  _draw_rect(buf + tempw * 4, 0, y1 + 1, width, tempy);
  _draw_rect(buf + tempw * 4 + tempy * width * 4, 0, y2, len / 4 - tempw - tempy * width, 1);
}

void init_device()
{
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention

  //pa3 level3
  int width = 0, height = 0;
  getScreen(&width, &height);
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", width, height);
  Log("dispinfo\n%s", dispinfo);
}
