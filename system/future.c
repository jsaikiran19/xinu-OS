#include <future.h>
#include <xinu.h>

future_t* future_alloc(future_mode_t mode, uint size, uint nelem) {
  intmask mask;
  mask = disable();
future_t *future;
	future=(future_t *)getmem(size+sizeof(future_t));
  future->data=sizeof(future_t)+(char*)future;
  future->mode=mode;
  future->state=FUTURE_EMPTY;
 // TODO: write your code here

  restore(mask);
  
	return future;
}

// TODO: write your code here for future_free, future_get and future_set

syscall future_free(future_t *future)
{
    intmask mask;
    mask = disable();
    restore(mask);
    return freemem((char*)future, sizeof(future_t)+future->size);
}

syscall future_set(future_t *future,char *data)
{
  
    intmask mask;
    mask = disable();
    if (future->state == FUTURE_EMPTY)
    {
        future->state=FUTURE_READY;
      memcpy(future->data,data,sizeof(data));
      
    }
    else if (future->state==FUTURE_READY)
    {
      return SYSERR;
    }
    else if (future->state== FUTURE_WAITING)
    {
      memcpy(future->data,data,sizeof(data));
      future->state=FUTURE_EMPTY;
      resume(future->pid);
    }
    restore(mask);
}
syscall future_get(future_t *future,char *data)
{
  intmask mask;
  mask=disable();
  if(future->state == FUTURE_EMPTY)
  {
    future->state=FUTURE_WAITING;
    future->pid=getpid();
    suspend(future->pid);
    memcpy(data,future->data,sizeof(future->data));
    
  }
  else if(future->state == FUTURE_READY)
  {
    memcpy(data,future->data,sizeof(data));
    future->state=FUTURE_EMPTY;
  }
  
  else if(future->state== FUTURE_WAITING)
  {
    return SYSERR;
  }
  restore(mask);
}
