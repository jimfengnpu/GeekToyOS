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
}

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

/// @brief sleep by global clock interrupt
/// @param sleep_tick 
void clock_sleep(clock_t sleep_tick) {
	clock_t start = ticks;
	while(ticks != start + sleep_tick) {
		pause();
	}
}

/// @brief 等待至多 sleep_tick ticks, 立即返回当检测到: flag != target(change==1) 或 flag == target(change==0）
/// @param sleep_tick 
/// @param flag watch flag ptr
/// @param target if change == 1, original value of flag ,else watch target value
/// @param change trigger mode
/// @return 1: flag triggered; 0: timeout
int clock_sleep_watch_flag(clock_t sleep_tick, volatile int* flag, int target, int change) {
	clock_t start = ticks;
	while(ticks != start + sleep_tick) {
		pause();
		if(change){
			if(*flag != target)
				return 1;
		} else {
			if(*flag == target)
				return 1;	
		}
	}
	return 0;
}
