#include <cpu.h>
#include <kernel/clock.h>
#include <kernel/interrupt.h>
#include <lib/time.h>
// hpet.c
int hpet_init();
void hpet_start();
static int clock_type; // 1 for hpet, 0 for 8253/8254PIT 

#define RTC_ADDR	0x70
#define RTC_DATA	0x71

/* rtc */
static inline u8 readRTC(int addr){
	//*** must disable int and ***
	outb(RTC_ADDR, addr);
	return inb(RTC_DATA);
}

static inline int bcd2byte(u8 x){
	return ((x>>4)&0xF)*10 + (x & 0xF);
}

// ** must ** disable int
void rtc_get_datetime(struct tm* time){
	int year, mon, day, hour, min, sec;
	do{
		year = readRTC(0x9);
		mon = readRTC(0x8);
		day = readRTC(0x7);
		hour = readRTC(0x4);
		min = readRTC(0x2);
		sec = readRTC(0);
	}while(sec != readRTC(0));
	if(!(readRTC(0xB) & 0x4)){
		year = bcd2byte(year);
		mon = bcd2byte(mon);
		day = bcd2byte(day);
		hour = bcd2byte(hour);
		min = bcd2byte(min);
		sec = bcd2byte(sec);
	}
	year += 1900;
	if(year < 1970)year += 100;
	time->tm_year = year - 1900;
	time->tm_mon = mon - 1;
	time->tm_mday = day;
	time->tm_hour = hour;
	time->tm_min = min;
	time->tm_sec = sec;
	time->__tm_gmtoff = RTC_TIMEZONE * 3600;
}

u32 arch_get_hw_timestamp(){
	struct tm time = {0};
	rtc_get_datetime(&time);
	return mktime(&time);
}

/* 8253/8254 PIT (Programmable Interval Timer) */
#define TIMER0         0x40 /* I/O port for timer channel 0 */
#define TIMER_MODE     0x43 /* I/O port for timer mode control */
#define RATE_GENERATOR 0x34 /* 00-11-010-0 :
			     * Counter0 - LSB then MSB - rate generator - binary
			     */
#define TIMER_FREQ     1193182L/* clock frequency for timer in PC and AT */
void pit_start()
{
    outb(TIMER_MODE, RATE_GENERATOR);
	outb(TIMER0, (u8)(TIMER_FREQ / HZ));
	outb(TIMER0, (u8)((TIMER_FREQ / HZ) >> 8));
}



int arch_clock_init(){
    if(hpet_init()){
        clock_type = 1;
    }
	register_interrupt_handler(IRQ_TO_INT(CLOCK_IRQ), ISR_IRQ, clock_handler);
    return 1;
}


void arch_clock_start(void)
{
    if (clock_type){
        hpet_start();
    }else {
		pit_start();
	}
}

void lapic_timer_enable(); // apic.c

int arch_local_clock_enable() {
	lapic_timer_enable();
}