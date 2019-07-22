#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "vga.h"

extern struct ptable_t ptable;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_getmaxproc(void){
		return NPROC;
}

int sys_getptable(void){
		struct proc *pt;
		int nproc;

		if(argint(1, &nproc) < 0 || argptr(0, (char**)&pt, nproc) < 0)
				return -1;

		acquire(&ptable.lock);
		for(struct proc *p = ptable.proc; p < &ptable.proc[NPROC]; p++){
				*pt = *p;
				pt++;
		}
		release(&ptable.lock);

		return 0;
}

int sys_setvga(void){
	vga_setmode(VGA_320x200x256);
	return 0;
}

int sys_draw_pixel(void){
	int x, y, c;
	if(argint(0, &x) < 0 || argint(1, &y) < 0 || argint(2, &c) < 0)
		return -1;
//	cprintf("x:%d y:%d c:%d\n", x, y, c);
	draw_pixel(x, y, c);
	return 0;
}
