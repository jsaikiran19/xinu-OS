/* Global variable for producer consumer */
extern int n; /* this is just declaration */

extern sid32 can_join;
extern sid32 can_read;
extern sid32 can_write;
/* Function Prototype */
void consumer(int count);
void producer(int count);
