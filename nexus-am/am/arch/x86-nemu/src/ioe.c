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

//pa3 level3 add: getScreen
void getScreen(int *width, int *height)
{
  *width = _screen.width;
  *height = _screen.height;
}

extern void *memcpy(void *, const void *, int);

//pa2 level3: finish _draw_rect
void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h)
{

  // int i;
  // for (i = 0; i < _screen.width * _screen.height; i++)
  // {
  //   fb[i] = i;
  // }
  int widthClamped = (w > _screen.width - x) ? _screen.width - x : w;
  int copyBytesPerRow = sizeof(uint32_t) * widthClamped;
  for (int row = 0; row < h && y + row < _screen.height; row++)
  {
    // Calculate destination pointer in the frame buffer
    uint32_t *dest = &fb[(y + row) * _screen.width + x];
    // Copy the pixel row from source to destination
    memcpy(dest, pixels, copyBytesPerRow);
    // Move the source pointer to the next row of pixels
    pixels += w;
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
