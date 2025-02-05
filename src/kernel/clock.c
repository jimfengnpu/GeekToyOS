#include <kernel/clock.h>
#include <kernel/interrupt.h>
#include <lib/time.h>
#include <cpu.h>



u32 current_timestamp;
clock_t ticks;

void clock_handler(trapframe_t *frame)
{
	// note: should not use time costly oper!!!
	ticks++;
	char strtime[24];
	struct tm tm_time;
	if(!(ticks % HZ)){
		current_timestamp++;
		localtime(current_timestamp, &tm_time);
		strftime(strtime, 24, "%Y-%m-%d:%H:%M:%S", &tm_time);
		if(tm_time.tm_sec % 5== 0)
		info("%s\n", strtime);
	}
	//wakeup(&ticks);
}

// void arch_clock_start(void); 
void clock_init(){
	ticks = 0;
	if (!arch_clock_init()){
		panic("kernel: failed to init clock!");
	}
	current_timestamp = arch_get_hw_timestamp();
	klog("CLOCK:timestamp %d\n", current_timestamp);
	arch_clock_start();
	interrupt_enable_irq(CLOCK_IRQ);
}

void clock_sleep(clock_t sleep_tick) {
	clock_t start = ticks;
	while(ticks != start + sleep_tick) {
		pause();
	}
}
