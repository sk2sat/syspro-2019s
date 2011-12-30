#include "lwip/sys.h"
#include "arch/sys_arch.h"

#include "queue.h"
#include "kernel.h"
#include "proc.h"
#include "cpu.h"

#define DIE panic(__func__)

static struct {
  struct spinlock lk;
  struct cpu *cpu;
  u64 depth;
} lwprot;

//
// mbox
//
err_t
sys_mbox_new(sys_mbox_t *mbox, int size)
{
  if (size > MBOXSLOTS) {
    cprintf("sys_mbox_new: size %u\n", size);
    return ERR_MEM;
  }
  mbox->head = 0;
  mbox->tail = 0;
  mbox->invalid = 0;
  initlock(&mbox->s, "lwIP mbox");
  initcondvar(&mbox->c, "lwIP mbox");

  return ERR_OK;
}

void
sys_mbox_set_invalid(sys_mbox_t *mbox)
{
  mbox->invalid = 1;
}

int
sys_mbox_valid(sys_mbox_t *mbox)
{
  return !mbox->invalid;
}

err_t
sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
  err_t r = ERR_MEM;

  acquire(&mbox->s);
  if (mbox->head - mbox->tail < MBOXSLOTS) {
    mbox->msg[mbox->head % MBOXSLOTS] = msg;
    mbox->head++;    
    cv_wakeup(&mbox->c);
    r = ERR_OK;
  }
  release(&mbox->s);

  return r;
}

void
sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
  acquire(&mbox->s);
  while (mbox->head - mbox->tail == MBOXSLOTS) {
    lwip_core_unlock();
    cv_sleep(&mbox->c, &mbox->s);
    lwip_core_lock();
  }
  mbox->msg[mbox->head % MBOXSLOTS] = msg;
  mbox->head++;
  cv_wakeup(&mbox->c);
  release(&mbox->s);
}

void
sys_mbox_free(sys_mbox_t *mbox)
{
  if (mbox->head != mbox->tail)
    panic("sys_mbox_free");
}

u32_t
sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
  u64 start, to;
  u32 r;

  acquire(&mbox->s);
  start = nsectime();
  to = (u64)timeout*1000000 + start;
  while (mbox->head-mbox->tail == 0) {
    if (timeout != 0) {
      if (to < nsectime()) {
        r = SYS_ARCH_TIMEOUT;
        goto done;
      }
      lwip_core_unlock();
      cv_sleepto(&mbox->c, &mbox->s, to);
      lwip_core_lock();
    } else {
      lwip_core_unlock();
      cv_sleep(&mbox->c, &mbox->s);      
      lwip_core_lock();
    }
  }
  r = nsectime()-start;
  if (msg)
    *msg = mbox->msg[mbox->tail % MBOXSLOTS];
  mbox->tail++;

done:
  release(&mbox->s);
  return r;
}

u32_t
sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
  u32_t r = SYS_MBOX_EMPTY;

  acquire(&mbox->s);
  if (mbox->head - mbox->tail != 0) {
    if (msg)
      *msg = mbox->msg[mbox->tail % MBOXSLOTS];
    mbox->tail++;
    r = 0;
  }
  release(&mbox->s);
  return r;
}

//
// sem
//
err_t
sys_sem_new(sys_sem_t *sem, u8_t count)
{
  initlock(&sem->s, "lwIP sem");
  initcondvar(&sem->c, "lwIP condvar");
  sem->count = count;
  sem->invalid = 0;
  return ERR_OK;
}

void
sys_sem_free(sys_sem_t *sem)
{
}

void
sys_sem_set_invalid(sys_sem_t *sem)
{
  sem->invalid = 1;
}

int
sys_sem_valid(sys_sem_t *sem)
{
  return !sem->invalid;
}

void
sys_sem_signal(sys_sem_t *sem)
{
  acquire(&sem->s);  
  sem->count++;
  cv_wakeup(&sem->c);
  release(&sem->s);
}

u32_t
sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  u64 start, to;
  u32 r;

  acquire(&sem->s);
  start = nsectime();
  to = (u64)timeout*1000000 + start;
  while (sem->count == 0) {
    if (timeout != 0) {
      if (to < nsectime()) {
        r = SYS_ARCH_TIMEOUT;
        goto done;
      }
      lwip_core_unlock();
      cv_sleepto(&sem->c, &sem->s, to);
      lwip_core_lock();
    } else {
      lwip_core_unlock();
      cv_sleep(&sem->c, &sem->s);      
      lwip_core_lock();
    }
  }
  r = nsectime()-start;
  sem->count--;

done:
  release(&sem->s);
  return r;
}

//
// thread
//
struct lwip_thread {
  lwip_thread_fn thread;
  void *arg;
};

static void
lwip_thread(void *x)
{
  struct lwip_thread *lt = x;
  lwip_core_lock();
  lt->thread(lt->arg);
  lwip_core_unlock();
  kmfree(lt);
}

sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn thread, void *arg,
               int stacksize, int prio)
{
  struct lwip_thread *lt;
  struct proc *p;

  lt = kmalloc(sizeof(*lt));
  if (lt == NULL)
    return NULL;
  lt->thread = thread;
  lt->arg = arg;

  p = threadalloc(lwip_thread, lt);
  if (p == NULL)
    panic("lwip: sys_thread_new");
  safestrcpy(p->name, name, sizeof(p->name));

  acquire(&p->lock);
  p->state = RUNNABLE;
  addrun(p);
  release(&p->lock);

  return p;
}

//
// init
//
void
sys_init(void)
{
}

//
// serialization
//
void
lwip_core_unlock(void)
{
  release(&lwprot.lk);  
}

void
lwip_core_lock(void)
{
  acquire(&lwprot.lk);
}

void
lwip_core_init(void)
{
  initlock(&lwprot.lk, "lwIP lwprot");
}