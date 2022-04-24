#include <future.h>
#include <xinu.h>

future_t *future_alloc(future_mode_t mode, uint size, uint nelem)
{

  future_t *future = (future_t *)getmem(size + sizeof(future_t));
  future->data = sizeof(future_t) + (char *)future;
  future->mode = mode;
  future->state = FUTURE_EMPTY;
  future->get_queue = newqueue();
  return future;
}

syscall future_free(future_t *future)
{
  qid16 q = future->get_queue;
  while (!isempty(q))
  {
    pid32 pid = dequeue(q);
    kill(pid);
  }
  return freemem((char *)future, sizeof(future_t) + future->size);
}

syscall future_set(future_t *future, char *data)
{

  if (future->state == FUTURE_EMPTY)
  {
    
    memcpy(future->data, data, sizeof(data));
    future->state = FUTURE_READY;
    return OK;
  }

  else if (future->state == FUTURE_WAITING)
  {
    memcpy(future->data, data, sizeof(data));
    resume(future->pid);
     future->state = future->mode==FUTURE_EXCLUSIVE ? FUTURE_EMPTY : FUTURE_READY;
     qid16 q = future->get_queue;
    while (!isempty(q))
    {
      resume(dequeue(q));
    }
    return OK;
  }
  return SYSERR;
}
syscall future_get(future_t *future, char *data)
{

  if (future->state == FUTURE_EMPTY)
  {
    future->state = FUTURE_WAITING;
    future->pid = getpid();
    suspend(future->pid);
    enqueue(future->pid, future->get_queue);
    memcpy(data, future->data, sizeof(future->data));
    return OK;
  }
  else if (future->state == FUTURE_READY)
  {
    memcpy(data, future->data, sizeof(data));
    future->state = future->mode==FUTURE_EXCLUSIVE ? FUTURE_EMPTY : FUTURE_READY;
    return OK;
  }
  else if (future->mode == FUTURE_SHARED)
  {
    pid32 pid = getpid();
    suspend(pid);
    enqueue(pid, future->get_queue);
    return OK;
  }

  return SYSERR;
}