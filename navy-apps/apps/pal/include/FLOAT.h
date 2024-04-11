#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;

//pa5 level1
static inline int F2int(FLOAT a) {
  int b;
  if(a>0) b=a/(1<<16);
  else b=-(-a/(1<<16));
  return b;
}
static inline FLOAT int2F(int a) {
  int b;
  if(a>0) b=a*(1<<16);

  else b=-(-a*(1<<16));
  return b;
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  return a*b;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  return a/b;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif
