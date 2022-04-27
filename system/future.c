#include <future.h>
#include <xinu.h>

future_t *future_alloc(future_mode_t mode, uint size, uint nelem)
{

  int mask = disable();
  future_t *future = (future_t *)getmem((size) + sizeof(future_t));
  future->data = sizeof(future_t) + (char *)future;
  future->mode = mode;
  future->state = FUTURE_EMPTY;
  restore(mask);
  return future;
}

syscall future_free(future_t *future)
{
  int mask = disable();
  restore(mask);
  return freemem((char *)future, sizeof(future_t) + future->size);
}

syscall future_set(future_t *future, char *data)
{
  int mask = disable();

  if (future->state == FUTURE_EMPTY)
  {
    
    memcpy(future->data, data, sizeof(data));
    future->state = FUTURE_READY;
    return OK;
  }

  if (future->state == FUTURE_WAITING)
  {
    memcpy(future->data, data, sizeof(data));
    resume(future->pid);
    future->state = FUTURE_EMPTY;
    return OK;
  }
  restore(mask);
  return SYSERR;
}
syscall future_get(future_t *future, char *data)
{

  int mask = disable();
  if (future->state == FUTURE_EMPTY)
  {
    future->state = FUTURE_WAITING;
    future->pid = getpid();
    suspend(future->pid);
    memcpy(data, future->data, sizeof(future->data));
    return OK;
  }
  if (future->state == FUTURE_READY)
  {
    memcpy(data, future->data, sizeof(data));
    future->state = FUTURE_EMPTY;
    return OK;
  }
  restore(mask);
  return SYSERR;
}