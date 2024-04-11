#include "common.h"

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE

//pa4 level1 change:
#define HAS_PTE

void init_mm(void);
void init_ramdisk(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
uint32_t loader(_Protect *, const char *);

extern void load_prog(const char *filename);

int main()
{
#ifdef HAS_PTE
  init_mm();
#endif

  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  init_ramdisk();

  init_device();

#ifdef HAS_ASYE
  Log("Initializing interrupt/exception handler...");
  init_irq();
#endif

  init_fs();

  // uint32_t entry = loader(NULL, "/bin/pal");
  // ((void (*)(void))entry)();

  //pa4 level1 change load_prog
  load_prog("/bin/pal");
  //pa4 level3
  load_prog("/bin/hello");

  //pa4 level5
  load_prog("/bin/videotest");

  //pa4 level2: add _trap()
  _trap();
  panic("Should not reach here");
}
