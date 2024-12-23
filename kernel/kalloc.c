// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;
/*kmem 是一个内核内存分配器的结构，lock 是它的一个互斥锁。为了确保多个内核线程安全地访问内存分配器（即空闲内存列表 freelist），
需要先获取这个锁。这样可以防止多个线程同时修改 freelist 导致数据竞争或不一致*/

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)//PHYSTOP是内核物理地址的上限，end是下限，同时检查是否是页对齐地址
    panic("kfree");

  // Fill with junk to catch dangling refs.
  /*这行代码用 1 填充传入的页。这样做的目的是帮助捕获悬挂指针（dangling pointers）
  如果内存释放后仍然被访问，程序会访问到一个无效的内存区域，并且由于内存已被填充为 1，
  程序可以发现非法访问，这在调试时非常有用，能够及早发现错误*/
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  /*填充垃圾数据为了防止悬挂指针引用已经释放的内存，同时方便调试，但同时他也是有效的可分配的内存块，因此插入空闲页链表等待下次分配数据*/

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;//获取空闲页链表的头部
  if(r)
    kmem.freelist = r->next;//头部指针指向下一个页
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;//返回获取到的页地址
}
