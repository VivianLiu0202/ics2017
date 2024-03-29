#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48 // Note that this is not standard
static unsigned long boot_time;

void _ioe_init()
{
  boot_time = inl(RTC_PORT);
}

//pa2 level3: add uptime
unsigned long _uptime()
{
  unsigned long ms = inl(RTC_PORT) - boot_time;
  return ms;
}

uint32_t *const fb = (uint32_t *)0x40000;

_Screen _screen = {
    .width = 400,
    .height = 300,
};

extern void *memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h)
{
  int i;
  for (i = 0; i < _screen.width * _screen.height; i++)
  {
    fb[i] = i;
  }
}

void _draw_sync()
{
}

//pa2 level3: add keyboard
int _read_key()
{
  uint32_t key = _KEY_NONE;
  if (inb(0x64) & 0x1)
    key = inl(0x60);

  return key;
}
