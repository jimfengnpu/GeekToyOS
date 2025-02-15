#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/clock.h>
#include <kernel/smp.h>
#include <kernel/spinlock.h>
#include <atomic.h>

void preempt_enable()
{
	if(atomic_dec_and_test(&thiscpu->atomic_preempt_count)){
		release(&thiscpu->preempt_lock);
	}
}

void preempt_disable()
{
	if(atomic_inc(&thiscpu->atomic_preempt_count)){
		return;
	}
	acquire(&thiscpu->preempt_lock);
}

static int scheduler_check_resched()
{

}

struct task *idle_init()
{
	return NULL;
}

void sched_idle(){
	while(1){
		interrupt_enable();
		halt();
	}
}

void schedule()
{

}

void sched_yield()
{

}

void sched_handler(trapframe_t *frame){
	// if(ticks % HZ == 0){
	// 	info("%d ", (int)smp_cpuid());
	// }
}

void sched_init()
{

}

void sched_start(){
	arch_local_clock_enable();
	sched_yield();
	sched_idle();
}