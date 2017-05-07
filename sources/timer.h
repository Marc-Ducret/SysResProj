#ifndef TIMER_H
#define TIMER_H
void init_timer(u32 frequency);

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

typedef struct {
    int mseconds;
    int seconds;
    int minutes;
    int hours;
    int day;
    int month;
    int year;
    int century;
} rtc_time_t;

rtc_time_t current_time;
void print_time(rtc_time_t *t);
void update_time(void);
void reset_time();
#endif /* TIMER_H */

