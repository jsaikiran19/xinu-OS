#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>
void consumer(int count)
{
  // TODO: implement the following:
  // - Iterates from 0 to count (count including)
  //   - reading the value of the global variable 'n' each time
  //   - print consumed value (the value of 'n'), e.g. "consumed : 8"
  int i;
  for (i = 0; i <= count; i++)
  {
    if (wait(can_read))
    {
      printf("consumed : %d", n);
      printf("\n");
      signal(can_write);
    }
  }
  signal(can_join);
  return;
}

void consumer_bb(int id, int count) {
  // TODO: implement the following:
  // - Iterate from 0 to count (count excluding)
  //   - read the next available value from the global array `arr_q`
  //   - print consumer id (starting from 0) and read value as:
  //     "name : consumer_X, read : X"
  for(int i=0;i<count;i++) {
    wait(can_consume);
    total_count-=1;
    printf("name : consumer_%d, read : %d\n",id,arr_q[read_index]);
    signal(can_produce);
  }
  if (total_count==0) {
    signal(can_exit_prodcons_bb);
  }
  return;
}
