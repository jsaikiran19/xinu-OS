#include <xinu.h>
#include <stream.h>
#include <stdio.h>
#include <run.h>
#include "tscdf.h"

uint pcport;
void stream_consumer(int32 id, struct stream *str);
int queue_length, time_intv, out_time;
int32 stream_proc(int nargs, char *args[])
{
    ulong secs, msecs, time;
    int32 num_streams, work_queue_depth, time_window, output_time;
    secs = clktime;
    int st, ts, v, head;
    char *ch, c, *a;
    msecs = clkticks;
    char usage[] = "Usage: run tscdf -s <num_streams> -w <work_queue_depth> -t <time_window> -o <output_time>\n";

    /* Parse arguments out of flags */
    /* if not even # args, print error and exit */
    if (!(nargs % 2))
    {
        printf("%s", usage);
        return (-1);
    }
    else
    {
        int i = nargs - 1;
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
                return (-1);
            }

            i -= 2;
        }
    }
    pcport = ptcreate(num_streams);
    if (pcport == SYSERR)
    {
        printf("ptcreate failed\n");
        return (-1);
    }
    struct stream **strm = (struct stream **)getmem(num_streams * sizeof(struct stream *));
    queue_length = work_queue_depth;
    time_intv = time_window;
    out_time = output_time;

    if (strm == SYSERR)
    {
        printf("ERROR: GETMEM FAILED\n");
    }
    int i = 0;
    while (i < num_streams)
    {
        strm[i] = (struct stream *)getmem(sizeof(struct stream) + (sizeof(de) * work_queue_depth));
        if ((strm[i]  == SYSERR))
        {
            printf("ALLOCATION ERROR");
            return;
        }
        strm[i]->mutex = semcreate(1);
        strm[i]->items = semcreate(work_queue_depth);
        strm[i]->spaces = semcreate(0);
        strm[i]->head = 0;
        strm[i]->queue = sizeof(struct stream) + (char *)strm[i];
        i++;
    }

    for (int j = 0; j < num_streams; j++)
    {
        resume(create((void *)stream_consumer, 4096, 20, "stream_consumer", 2, j, strm[j]));
    }
    i = 0;
    while (i < n_input)
    {
        
        a = (char *)stream_input[i];
        st = atoi(a);
        while (*a++ != '\t')
            ;
        ts = atoi(a);
        while (*a++ != '\t')
            ;
        v = atoi(a);
        wait(strm[st]->items);
        wait(strm[st]->mutex);
        head = strm[st]->head;

        strm[st]->queue[head].value = v;
        strm[st]->queue[head].time = ts;
        
        head = ++head % work_queue_depth;
        strm[st]->head = head;
        signal(strm[st]->mutex);
        signal(strm[st]->spaces);
        i++;
    }

    for (int i = 0; i < num_streams; i++)
    {
        uint32 pm = ptrecv(pcport);
        printf("process %d exited\n", pm);
    }

    ptdelete(pcport, 0);

    time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
    kprintf("time in ms: %u\n", time);
    signal(can_exit);
    return (0);
}

void stream_consumer(int32 id, struct stream *str)
{
    int tail, result = 0;
    struct tscdf *tc = tscdf_init(time_intv);
    pid32 pid = getpid();
    kprintf("stream_consumer id:%d (pid:%d)\n", id, pid);
    while (1)
    {
        wait(str->spaces);
        wait(str->mutex);
        tail = str->tail;
        if ( str->queue[str->tail].value == 0 && str->queue[str->tail].time == 0 )
        {
            kprintf("stream_consumer exiting\n");
            break;
        }
        tscdf_update(tc, str->queue[tail].time, str->queue[tail].value);

        if (result++ == (out_time - 1))
        {
            char output[10];
            int *qarray = tscdf_quartiles(tc);
            if (qarray == NULL)
            {
                kprintf("tscdf_quartiles returned NULL\n");
                continue;
            }
            sprintf(output, "s%d: %d %d %d %d %d \n", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
            freemem((char *)qarray, (6 * sizeof(int32)));
            kprintf("%s", output);
            result = 0;
        }
        ++tail;
        tail = tail % queue_length;

        str->tail = tail;
        signal(str->mutex);
        signal(str->items);
    }
    tscdf_free(tc);
    ptsend(pcport, getpid());
}