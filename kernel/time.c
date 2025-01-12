#include <kernel/time.h>
#include <kernel/interrupt.h>
#include <cpu.h>

#define RTC_ADDR	0x70
#define RTC_DATA	0x71
/* 8253/8254 PIT (Programmable Interval Timer) */
#define TIMER0         0x40 /* I/O port for timer channel 0 */
#define TIMER_MODE     0x43 /* I/O port for timer mode control */
#define RATE_GENERATOR 0x34 /* 00-11-010-0 :
			     * Counter0 - LSB then MSB - rate generator - binary
			     */
#define TIMER_FREQ     1193182L/* clock frequency for timer in PC and AT */
#define HZ             100  /* clock freq (software settable on IBM-PC) */
#define CLOCK_IRQ   32

u32 current_timestamp;
int ticks;

u8 readRTC(int addr){
	//*** must disable int and ***
	outb(RTC_ADDR, addr);
	return inb(RTC_DATA);
}

int bcd2byte(u8 x){
	return ((x>>4)&0xF)*10 + (x & 0xF);
}

void clock_handler(trapframe_t *frame)
{
	ticks++;
	if(!(ticks % HZ)){
		current_timestamp++;
        klog("%d ", ticks);
	}
	
	//wakeup(&ticks);

}

void clock_init(){
	ticks = 0;
	/* initialize 8253 PIT */
	outb(TIMER_MODE, RATE_GENERATOR);
	outb(TIMER0, (u8)(TIMER_FREQ / HZ));
	outb(TIMER0, (u8)((TIMER_FREQ / HZ) >> 8));
	/* initialize clock-irq */
    register_interrupt_handler(CLOCK_IRQ, ISR_IRQ, clock_handler);
	// put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
	current_timestamp = get_init_rtc_timestamp();
	// irq_enable();					   /* 让8259A可以接收时钟中断 */
	enable_irq(0);
}

// 计算自1970-1-1 00:00:00 UTC 的秒数(时间戳) 全是技巧的Gauss算法, copy自linux kernel
u32 mktime(struct tm* time){
	unsigned int mon = time->tm_mon + 1, 
	year = time->tm_year + 1900, day = time->tm_mday, 
	hour = time->tm_hour, min = time->tm_min, sec = time->tm_sec;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec - time->__tm_gmtoff; /* finally seconds */
}

static struct tm* _gmtime(u32 timestamp, struct tm* tm_time){
	static int days_four_year = 1461;
    static const unsigned char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	u32 localtime = timestamp + tm_time->__tm_gmtoff;
	tm_time->tm_sec = localtime % 60;
	localtime /= 60; // min
	tm_time->tm_min = localtime % 60;
	localtime /= 60; // hour
	tm_time->tm_hour = localtime % 24;
	localtime /= 24; // day
	tm_time->tm_wday = (int)(localtime + 4)%7; // 1970-1-1 星期四
	localtime += 25567; //1900-1-1 ~ 1970-1-1
	int year = ((localtime/days_four_year) << 2);
    localtime %= days_four_year;
    localtime += ((year + 99) / 100) - ((year + 299)/400);// 修正置闰
    int ydays = 365 + (int)(((year%4 == 0) && (year%100 != 0)) || ((year+300) % 400 == 0));
    while(ydays <= localtime){
        localtime -= ydays;
        year++;
        ydays = 365 + (int)(((year%4 == 0) && (year%100 != 0)) || ((year+300) % 400 == 0));
    }
    int leapyear = ((year%4 == 0) && (year%100 !=0)) || ((year+300) %400 == 0);
    tm_time->tm_year = year;
    tm_time->tm_yday = localtime;
    int mon = 0;
    int mdays = Days[mon] + (int)(leapyear&&mon==1);
    while (mdays <= localtime)
    {
        localtime -= mdays;
        mon++;
        mdays = Days[mon] + (int)(leapyear&&mon==1);
    }
    tm_time->tm_mon = mon;
    tm_time->tm_mday = localtime + 1;
	return tm_time;
}

struct tm* gmtime(u32 timestamp, struct tm* tm_time){
	tm_time->__tm_gmtoff = 0;
	_gmtime(timestamp, tm_time);
	return tm_time;
}

struct tm* localtime(u32 timestamp, struct tm* tm_time){
	tm_time->__tm_gmtoff = (LOCAL_TIMEZONE)*3600;
	_gmtime(timestamp, tm_time);
	return tm_time;
}

// ** must ** disable int
void get_rtc_datetime(struct tm* time){
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

u32 get_init_rtc_timestamp(){
	struct tm time = {0};
	get_rtc_datetime(&time);
	return mktime(&time);
}