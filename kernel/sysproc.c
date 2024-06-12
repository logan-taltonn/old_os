#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);

  // acquire(&myproc()->lock);
  addr = myproc()->sz;
  if(growproc(n) < 0) {
    // release(&myproc()->lock);
    return -1;
  }
  // release(&myproc()->lock);
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_sigalarm(void)
{
  int itvl;
  uint64 handler_addr;

  argint(0, &itvl); 
  argaddr(1, &handler_addr);

  struct proc* p = myproc();

  // sigalarm(0, 0) case (???)
  if (itvl == 0 && handler_addr == 0) {
    p->alarm_on = 0;
    return 0;
    // return -1;
  }

  // unravelling handler type
  // handler_t handler = (handler_t)handler_addr;

  // setting proc fields
  p->fn = handler_addr;
  p->tot_ticks = itvl;

  return 0;
}

uint64 sys_sigreturn(void)
{
  struct proc* p = myproc();
  *p->trapframe = *p->alarm_pc;
  p->trapframe->epc = p->alarm_pc->epc; 

  // kfree(p->alarm_pc);
  // p->alarm_pc = 0;
  p->alarm_on = 0;
  // p->curr_ticks = 0;
  
  return 0;
}

uint64 sys_clone(void) 
{
  uint64 start_addr; // start
  uint64 stack_top_addr; // stack_top
  argaddr(0, &start_addr);
  argaddr(1, &stack_top_addr);
  void (*start)() = (void (*)())start_addr;
  void* stack_top = (void*)stack_top_addr;
  return clone(start, stack_top);
}