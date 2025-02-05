#include <kernel/sched.h>
#include <kernel/clock.h>

void sched_handler(trapframe_t *frame){
	if(ticks % HZ == 0){
		info("sc ");
	}
}

void sched_start(){
	arch_local_clock_enable();

}