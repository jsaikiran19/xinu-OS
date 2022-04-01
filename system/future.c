#include <future.h>
#include <xinu.h>

future_t *future_alloc(future_mode_t mode, uint size, uint nelem)
{
  intmask mask;
  mask = disable();
  future_t *future;

  future = (future_t *)getmem((size * nelem) + sizeof(future_t));
  future->data = sizeof(future_t) + (char *)future;
  future->mode = mode;
  future->size = size;
  future->state = FUTURE_EMPTY;
  future->get_queue = newqueue();
  if (mode == FUTURE_QUEUE)
  {
    future->set_queue = newqueue();
    future->max_elems = nelem;
    future->count = 0;
    future->head = 0;
    future->tail = 0;
  }
  restore(mask);

  return future;
}

// TODO: write your code here for future_free, future_get and future_set

syscall future_free(future_t *future)
{
  intmask mask;
  mask = disable();
  restore(mask);
  qid16 q = future->get_queue;
  qid16 q2 = future->set_queue;
  while (!isempty(q))
  {
    pid32 pid = dequeue(q);
    kill(pid);
  }
  while (!isempty(q2))
  {
    pid32 pid = dequeue(q2);
    kill(pid);
  }
  return freemem((char *)future, sizeof(future_t) + future->size);
}

syscall future_set(future_t *future, char *data)
{

  intmask mask;
  mask = disable();
  if (future->mode == FUTURE_QUEUE)
  {
    if (future->count == future->max_elems)
    {
      enqueue(getpid(), future->set_queue);
      suspend(getpid());
    }

    char *tailelemptr = future->data + (future->tail * future->size);
    memcpy(tailelemptr, data, future->size);
    future->tail = (future->tail + 1) % future->max_elems;
    future->count++;

    if (nonempty(future->get_queue))
    {
      resume(dequeue(future->get_queue));
    }

    restore(mask);
    return OK;
  }
  if (future->state == FUTURE_EMPTY)
  {
    future->state = FUTURE_READY;
    memcpy(future->data, data, sizeof(data));
  }
  else if (future->state == FUTURE_READY)
  {
    return SYSERR;
  }
  else if (future->state == FUTURE_WAITING)
  {
    memcpy(future->data, data, sizeof(data));
    future->state = future->mode == FUTURE_EXCLUSIVE ? FUTURE_EMPTY : FUTURE_READY;
    // resume(future->pid);
    qid16 q = future->get_queue;
    while (!isempty(q))
    {
      resume(dequeue(q));
    }
  }
  restore(mask);
}
syscall future_get(future_t *future, char *data)
{
  intmask mask;
  mask = disable();
  if (future->mode == FUTURE_QUEUE)
  {
    if (future->count == 0)
    {
      enqueue(getpid(), future->get_queue);
      suspend(getpid());
    }

    char *headelemptr = future->data + (future->head * future->size);
    memcpy(data, headelemptr, future->size);
    future->head = (future->head + 1) % future->max_elems;
    future->count--;

    if (nonempty(future->set_queue))
    {
      resume(dequeue(future->set_queue));
    }

    restore(mask);
    return OK;
  }
  if (future->state == FUTURE_EMPTY)
  {
    future->state = FUTURE_WAITING;
    future->pid = getpid();
    suspend(future->pid);
    enqueue(future->pid, future->get_queue);
    memcpy(data, future->data, sizeof(future->data));
  }
  else if (future->state == FUTURE_READY)
  {
    memcpy(data, future->data, sizeof(data));
    future->state = future->mode == FUTURE_EXCLUSIVE ? FUTURE_EMPTY : FUTURE_READY;
  }

  else if (future->state == FUTURE_WAITING && future->mode == FUTURE_EXCLUSIVE)
  {
    return SYSERR;
  }
  else if (future->mode == FUTURE_SHARED)
  {
    pid32 pid = getpid();
    suspend(pid);
    enqueue(pid, future->get_queue);
  }
  restore(mask);
}
