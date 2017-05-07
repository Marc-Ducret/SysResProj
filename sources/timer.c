#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "io.h"
#include "lib.h"
#include "int.h"
#include "printing.h"
#include "timer.h"

void init_timer(u32 frequency) {
    u32 divisor = 1193180 / frequency;
    
    if (divisor == 0 || divisor >> 16 != 0) {
        kprintf("Failed to init the time : invalid divisor %d\n", divisor);
        return;
    }
    
    // Send the command byte.
    outportb(0x43, 0x36);
    
    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    u8 l = (u8)(divisor & 0xFF);
    u8 h = (u8)( (divisor>>8) & 0xFF );
    
    // Send the frequency divisor.
    outportb(0x40, l);
    outportb(0x40, h);
    
    kprintf("Timer set to frequency %d Hz\n", frequency);
}

u8 get_rtc_reg(int reg) {
    outportb(CMOS_ADDRESS, reg);
    return inportb(CMOS_DATA);
}

int update_in_progress() {
    outportb(CMOS_ADDRESS, 0x0A);
    return (inportb(CMOS_DATA) & 0x80);
}

void reset_time() {
    // Reads the entire time from CMOS registers, and thus waits for a stable status.
    // This may take up to one second.
    u8 second, minute, hour, day, month, year;
    u8 lsecond, lminute, lhour, lday, lmonth, lyear;
    
    while (update_in_progress());   // Make sure an update isn't in progress
    second = get_rtc_reg(0x00);
    minute = get_rtc_reg(0x02);
    hour = get_rtc_reg(0x04);
    day = get_rtc_reg(0x07);
    month = get_rtc_reg(0x08);
    year = get_rtc_reg(0x09);
 
    do {
        lsecond = second;
        lminute = minute;
        lhour = hour;
        lday = day;
        lmonth = month;
        lyear = year;

        while (update_in_progress());           // Make sure an update isn't in progress
        second = get_rtc_reg(0x00);
        minute = get_rtc_reg(0x02);
        hour = get_rtc_reg(0x04);
        day = get_rtc_reg(0x07);
        month = get_rtc_reg(0x08);
        year = get_rtc_reg(0x09);

    } while ((lsecond != second) || (lminute != minute) || (lhour != hour) ||
             (lday != day) || (lmonth != month) || (lyear != year));
    u8 registerB = get_rtc_reg(0x0B);
    
    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
    }
    
    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }
    
    current_time.century = 20;
    current_time.day = day;
    current_time.year = year;
    current_time.hours = hour;
    current_time.month = month;
    current_time.seconds = second;
    current_time.mseconds = 0;
    current_time.minutes = minute;
}

void update_time() {
    // Updates time base on milliseconds and start date.
    rtc_time_t *t = &current_time;
    while (t->mseconds >= 1000) {
        t->mseconds -= 1000;
        t->seconds ++;
    }
    while (t->seconds >= 60) {
        t->seconds -= 60;
        t->minutes ++;
    }
    while (t->minutes >= 60) {
        t->minutes -= 60;
        t->hours ++;
    }
    if (t->hours >= 24) {
        reset_time();
    }
    return;
}

void print_time(rtc_time_t *t) {
    kprintf("%d:%d:%d, %d/%d/%d", t->hours, t->minutes, t->seconds,
            t->day, t->month, (t->year % 100) + 2000);
}

rtc_time_t gettimeofday(void) {
    // Returns time
    return current_time;
}

