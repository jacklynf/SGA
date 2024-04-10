#define WAIT_TIME 1 // 1 second wait time -- adjust this value for shorter/longer timer
#define CLOCK 9830400
#define PRESCALAR 1024
#define TICK_TIME 1/(CLOCK/PRESCALAR)
#define MY_TCNT1 65536 - (WAIT_TIME/TICK_TIME) 

void init_timer();