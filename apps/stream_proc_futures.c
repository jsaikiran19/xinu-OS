#include <xinu.h>
#include <stream.h>
#include <run.h>
#include <future.h>
#include "tscdf.h"

uint port;
int num_streams, work_queue_depth, time_window, output_time;
int stream_proc_futures(int nargs, char *args[]);
void stream_consumer_future(int32 id, future_t *f);

int stream_proc_futures(int nargs, char *args[])
{
    ulong secs, msecs, time;
    secs = clktime;
    msecs = clkticks;
    // TODO: Parse arguments
    char usage[] = "run tscdf_fq -s <num_streams> -w <work_queue_depth> -t <time_window> -o <output_time>\n";
    int i;
    char *ch, c;
    if (nargs != 9)
    {
        printf("%s", usage);
        signal(can_exit);
        return SYSERR;
    }
    else
    {
        i = nargs - 1;
        while (i > 0)
        {
            ch = args[i - 1];
            c = *(++ch);

            switch (c)
            {
            case 's':
                num_streams = atoi(args[i]);
                break;

            case 'w':
                work_queue_depth = atoi(args[i]);
                break;

            case 't':
                time_window = atoi(args[i]);
                break;

            case 'o':
                output_time = atoi(args[i]);
                break;

            default:
                printf("%s", usage);
                signal(can_exit);
                return SYSERR;
            }

            i -= 2;
        }
    }
    port = ptcreate(num_streams);
    if (port == SYSERR)
    {
        printf("Error: Port Creation\n");
        signal(can_exit);
        return -1;
    }

    future_t **futures = (struct future_t **)getmem(num_streams * sizeof(future_t));

    for (int i = 0; i < num_streams; i++)
    {
        futures[i] = future_alloc(FUTURE_QUEUE, sizeof(de), work_queue_depth);
        resume(create((void *)stream_consumer_future, 2048, 20, "stream_consumer_future", 2, i, futures[i]));
    }

    for (int i = 0; i < n_input; i++)
    {
        char *a = (char *)stream_input[i];
        int st = atoi(a);
        while (*a++ != '\t')
            ;
        int ts = atoi(a);
        while (*a++ != '\t')
            ;
        int v = atoi(a);

        de *pd = (de *)getmem(sizeof(de));
        pd->time = ts;
        pd->value = v;

        future_set(futures[st], pd);
    }

    
    for (i = 0; i < num_streams; i++)
    {
        uint32 pm = ptrecv(port);
        printf("process %d exited\n", pm);
    }

    ptdelete(port, 0);


    for (int i = 0; i < num_streams; i++)
    {
        future_free(futures[i]);
    }

    // TODO: Measure the time of this entire function and report it at the end
    time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
    printf("time in ms: %u\n", time);
    signal(can_exit);
    return 0;
}

void stream_consumer_future(int32 id, future_t *f)
{
    kprintf("stream_consumer_future id:%d (pid:%d)\n", id, getpid());
    
    struct tscdf *tc = tscdf_init(time_window);
    int c = 0;
    char *output;
    int *qarray = (int32 *)getmem(6 * sizeof(int32));

    while (TRUE)
    {
        de* qElement;
        future_get(f, qElement);

        int update = tscdf_update(tc, qElement->time, qElement->value);

        if (update == SYSERR)
        {
            return SYSERR;
        }

        else if (qElement->time == 0)
        {
            break;
        }
        c++;


        if (c == output_time)
        {
            c = 0;
            output = "";
            qarray = tscdf_quartiles(tc);

            if (qarray==NULL)
            {
                kprintf("tscdf_quartiles returned NULL\n");
                continue;
            }

            sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
            kprintf("%s\n", output);
            freemem((char *)qarray, (6 * sizeof(int32)));
        }
    }

    tscdf_free(tc);
    future_free(f);
    kprintf("stream_consumer_future exiting\n");
    pid32 pid = getpid();
    ptsend(port, pid);
    return;
}
