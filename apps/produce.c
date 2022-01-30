#include <xinu.h>
#include <prodcons.h>

void producer(int count)
{
  // TODO: implement the following:
  // - Iterates from 0 to count (count including)
  //   - setting the value of the global variable 'n' each time
  //   - print produced value (new value of 'n'), e.g.: "produced : 8"
  int i;
  for (i = 0; i <= count; i++)
  {
    if (wait(can_write))
    {
      n = i;
      printf("produced : %d", n);
      printf("\n");
      signal(can_read);
    }
  }
  return;
}
