#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void *new_page(void)
{
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p)
{
  panic("not implement yet");
}

/* The brk() system call handler. */

//pa4 level1 : change mm_brk
/*
int mm_brk(uint32_t new_brk)
{
  if (current->cur_brk == 0)
  {
    current->cur_brk = current->max_brk = new_brk;
  }
  else
  {
    if (new_brk > current->max_brk)
    {
      // TODO: map memory region [current->max_brk, new_brk)
      // into address space current->as
      uintptr_t vaddr = (((uint32_t)(current->max_brk) + 0xfff) & 0xfffff000);
      while (vaddr < new_brk)
      {
        _map(&current->as, (void *)vaddr, (void *)new_page());
        vaddr += PGSIZE;
      }
      current->max_brk = new_brk;
    }
    current->max_brk = new_brk;
  }
  return 0;
}*/
int mm_brk(uint32_t new_brk) {
  //pa3 step3 debug
  if(current->cur_brk == 0) {
    current->cur_brk = current->max_brk = new_brk;
  }
  else {
    //pa4 step1 ��������Ķ���ӳ�䵽�����ַ�ռ��С�
    //firstΪ��Ҫ����ӳ��ĵ�һ��ҳ��������ַ�׵�ַ��endΪ��Ҫ����ӳ������һ��ҳ��������ַ�׵�ַ��
    //����new_brk�Ƿ���ҳ����ģ�������Ӧ��first��end
    if (new_brk > current->max_brk) {
      uint32_t first=PGROUNDUP(current->max_brk);
      uint32_t end=PGROUNDDOWN(new_brk);
      if((new_brk & 0xfff) ==0) end -= PGSIZE;
      for(uint32_t va=first;va<=end;va+=PGSIZE){
        void* pa=new_page();
        _map(&(current->as),(void*)va,pa);
      }
      current->max_brk=new_brk;
    }
    current->cur_brk=new_brk;
  }
  return 0;
}


void init_mm()
{
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
  Log("finish pte_init...");
}
