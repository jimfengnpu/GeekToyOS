#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/clock.h>
#include <kernel/spinlock.h>
#include <atomic.h>
#include <smp.h>

static int atomic_preempt_count;
static struct spinlock preempt_lock[MAX_CORES];

void preempt_enable()
{
	if(atomic_dec_and_test(&atomic_preempt_count)){
		release(&preempt_lock[smp_cpuid()]);
	}
}

void preempt_disable()
{
	if(atomic_inc(&atomic_preempt_count)){
		return;
	}
	acquire(&preempt_lock[smp_cpuid()]);
}

void sched_idle(){
	while(1){
		interrupt_enable();
		halt();
	}
}

void sched_yield()
{

}

void sched_handler(trapframe_t *frame){
	if(ticks % HZ == 0){
		info("%d ", smp_cpuid());
	}
}

void sched_start(){
	arch_local_clock_enable();
	sched_yield();
}