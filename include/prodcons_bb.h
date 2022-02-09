
extern int arr_q[5];
extern sid32 can_produce; 
extern sid32 can_consume;
extern sid32 can_exit_prodcons_bb;
extern int write_index;
extern int read_index;
extern int total_count;
void consumer_bb(int id, int count);
void producer_bb(int id, int count);