#define WAIT_TIME 1 // ms wait time -- adjust this value for shorter/longer timer
#define CLOCK 7372800
#define PRESCALAR 1024
#define TICK_TIME 1/(CLOCK/PRESCALAR) // 1/7200
#define MY_TCNT1 65536 - (WAIT_TIME/TICK_TIME) 

void init_timer();