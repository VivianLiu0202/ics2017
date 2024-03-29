#include "cpu/exec.h"

make_EHelper(mov)
{
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

//pa2 level1: finish instruction exec function
make_EHelper(push)
{
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop)
{
  rtl_pop(&id_src->val);
  operand_write(id_dest, &id_src->val);

  print_asm_template1(pop);
}

make_EHelper(pusha)
{
  // 保存ESP的原始值，因为我们在推送过程中会改变ESP
  rtl_lr(&t0, R_ESP, 4); // t0 = original ESP

  // 按顺序压入EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t0); // 注意这里推送的是原始ESP的值
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa)
{
  // 按相反的顺序弹出EDI, ESI, EBP, 跳过ESP，EBX, EDX, ECX, 和EAX
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);

  // 跳过原始ESP的值
  rtl_addi(&cpu.esp, &cpu.esp, 4);

  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

//pa2 add level2: leave
//
make_EHelper(leave)
{
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd)
{
  //先判断是16/32位再进行合适的操作
  if (decoding.is_operand_size_16)
  {
    //ax的16位整数扩展为32位，高16位用ax的符号位填充保存到dx
    rtl_lr_w(&t0, R_AX);
    rtl_sext(&t0, &t0, 2);
    //第16位是符号位
    rtl_sari(&t0, &t0, 16);
    rtl_sr_w(R_DX, &t0);
  }
  else
  {
    //eax的32位整数扩展为64位，高32位用eax的符号位填充保存到edx
    //rtl_sari(&cpu.edx,&cpu.eax,31);
    rtl_lr_l(&t0, R_EAX);
    rtl_sari(&t0, &t0, 31);
    rtl_sari(&t0, &t0, 1);
    rtl_sr_l(R_EDX, &t0);
    //rtl_sr_w(R_EDX,&t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

//pa2 add step2:cwtl
make_EHelper(cwtl)
{
  if (decoding.is_operand_size_16)
  {
    rtl_lr_b(&t0, R_AX);
    rtl_sext(&t0, &t0, 1);
    rtl_sr_w(R_AX, &t0);
  }
  else
  {
    rtl_lr_w(&t0, R_AX);
    rtl_sext(&t0, &t0, 2);
    rtl_sr_l(R_EAX, &t0);
  }
  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}
make_EHelper(movsx)
{
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx)
{
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea)
{
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
