#include "cpu/exec.h"

make_EHelper(jmp)
{
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc)
{
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm)
{
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

//pa2 level1: add call
make_EHelper(call)
{
  // the target address is calculated at the decode stage
  //先把eip压栈，再进行跳转
  rtl_li(&t2, decoding.seq_eip);
  rtl_push(&t2);
  decoding.is_jmp = 1;
  print_asm("call %x", decoding.jmp_eip);
}

//pa2 level1: add ret , 没有译码函数，直接实现执行函数
make_EHelper(ret)
{
  rtl_pop(&decoding.jmp_eip);
  decoding.is_jmp = 1;
  print_asm("ret");
}

//pa2 level2: add call_rm
make_EHelper(call_rm)
{
  rtl_li(&t2, decoding.seq_eip);
  rtl_push(&t2);
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;
  print_asm("call *%s", id_dest->str);
}
