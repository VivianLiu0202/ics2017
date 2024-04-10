#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({                                       \
  Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
  guest_to_host(addr);                                                        \
})

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

//pa4 level1 add page_translate
#define PDX(va) (((uint32_t)(va) >> 22) & 0x03ff)
#define PTX(va) (((uint32_t)(va) >> 12) & 0x03ff)
#define OFFSET(va) (((uint32_t)(va) >> 0) & 0x0fff)
#define REMOVE_OFFSET(pte) ((uint32_t)(pte) & 0xfffff000) 

bool IfDataCrossBoundary(vaddr_t addr, int len){
  return ((addr + len - 1) & ~PAGE_MASK) != (addr & ~ PAGE_MASK);
}


paddr_t page_translate(vaddr_t vaddr, bool flag){
  PDE pde;
  PDE *pgdir;
  PTE pte;
  PTE *ptdir;
  if(cpu.cr0.protect_enable && cpu.cr0.paging){
    pgdir = (PDE*)(REMOVE_OFFSET(cpu.cr3.val));
    pde.val = paddr_read((paddr_t)&pgdir[PDX(vaddr)], 4);
    // printf("%x %d\n", pde, pde.present);
    assert(pde.present);
    pde.accessed = true;

    ptdir = (PTE*)(REMOVE_OFFSET(pde.val));
    pte.val = paddr_read((paddr_t)&ptdir[PTX(vaddr)], 4);
    assert(pte.present);
    pte.accessed = true;
    pte.dirty = flag ? 1 : pte.dirty;

    return REMOVE_OFFSET(pte.val) | OFFSET(vaddr);
  }
  else{
    return vaddr;
  }
}

//pa2 level3: add read&wirte
uint32_t paddr_read(paddr_t addr, int len)
{
  int ismmio;
  if ((ismmio = is_mmio(addr)) != -1)
  {
    return mmio_read(addr, len, ismmio);
  }
  else
  {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data)
{
  int ismmio;
  if ((ismmio = is_mmio(addr)) != -1)
  {
    return mmio_write(addr, len, data, ismmio);
  }
  else
  {
    memcpy(guest_to_host(addr), &data, len);
  }
}


//pa4 level1 add vaddr

uint32_t vaddr_read(vaddr_t vaddr, int len)
{
  if(IfDataCrossBoundary(vaddr, len)){
    uint32_t ret = 0;
    for(int i=0; i<len; i++){
      paddr_t paddr = page_translate(vaddr+i, false);
      ret += paddr_read(paddr, 1) << (8 * i);
    }
    return ret;
  }
  else{
    paddr_t paddr = page_translate(vaddr, false);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t vaddr, int len, uint32_t data)
{
  if(IfDataCrossBoundary(vaddr, len)){
    for(int i=0; i<len; i++){
      paddr_t paddr = page_translate(vaddr+i, true);
      paddr_write(paddr, 1, data>>(8*i));
    }
  }
  else{
    paddr_t paddr = page_translate(vaddr, true);
    paddr_write(paddr, len, data);
  }
}
