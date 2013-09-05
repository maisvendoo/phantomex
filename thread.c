/*-----------------------------------------------------------------------------
 *
 * 		Thread's library
 * 		(c), maisvendoo 02.09.2013
 *
 *---------------------------------------------------------------------------*/

#include	"thread.h"

u32int		next_thread_id = 0;		/* Next thread ID */

/*-----------------------------------------------------------------------------
 *		Create thread
 *---------------------------------------------------------------------------*/
thread_t* kthread_create(void* entry_point,
	               	    size_t stack_size,
	               	    bool kernel,
	               	    bool suspend)
{
	void*	stack = NULL;
	u32int	eflags;

	/* Disable all interrupts */
	stop();

	/* Create new thread handler */
	thread_t* tmp_thread = (thread_t*) kmalloc(sizeof(thread_t));

	/* Clear memory */
	memset(tmp_thread, 0, sizeof(thread_t));

	/* Initialization of thread  */
	tmp_thread->id = get_thread_id();
	tmp_thread->list_item.list = NULL;

	process_t* proc = get_current_thread()->process;
	tmp_thread->process = proc;

	tmp_thread->stack_size = stack_size;
	tmp_thread->suspend = suspend;/* */
	tmp_thread->entry_point = (u32int) entry_point;

	/* Create thread's stack */
	stack = kmalloc(stack_size);

	tmp_thread->stack = stack;
	tmp_thread->esp = (u32int) stack + stack_size - 28;
	tmp_thread->stack_top = (u32int) stack + stack_size;


	/* Add thread to ring queue */
	add_thread(tmp_thread);

	/* Thread's count increment */
	proc->threads_count++;

	/* Fill stack */

	/* Create pointer to stack frame */
	u32int* esp = (u32int*) (stack + stack_size);

	eflags = read_eflags();

	eflags |= (1 << 9);

	if (kernel)
	{
		esp[-5] = (u32int) entry_point;
		esp[-7] = eflags;
	}
	else
	{
		esp[-3] = (u32int) tmp_thread;
		esp[-5] = (u32int) &start_user_thread;
		esp[-7] = eflags;
	}

	/* Enable all interrupts */
	start();

	return tmp_thread;
}

/*-----------------------------------------------------------------------------
 *		Correct exit from thread
 *---------------------------------------------------------------------------*/
void kthread_exit(thread_t* thread)
{
	/* Disable all interrupts */
	/*asm volatile ("cli");*/
	stop();

	/* Remove thread from queue */
	list_remove(&thread->list_item);

	thread->process->threads_count--;

	/* Free thread's memory (handler and stack) */
	kfree(thread->stack);
	thread->stack = NULL;
	free(thread->user_stack);
	thread->user_stack = NULL;
	kfree(thread);
	thread = NULL;

	/* Enable all interrupts */
	/*asm volatile ("sti");*/
	start();

	/* Jump to switch_task() */
	task_switch();
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void kthread_suspend(thread_t* thread, bool suspend)
{
	stop();

	if (suspend)
	{
		remove_thread(thread);
		thread->list_item.list = NULL;
		add_wait(thread);
	}
	else
	{
		remove_wait(thread);
		thread->list_item.list = NULL;
		add_thread(thread);
	}

	start();
}

/*-----------------------------------------------------------------------------
 *		Get thread ID
 *---------------------------------------------------------------------------*/
u32int get_thread_id(void)
{
	return next_thread_id++;
}

/*-----------------------------------------------------------------------------
 *		Start thread in user mode
 *---------------------------------------------------------------------------*/
void start_user_thread(thread_t* thread)
{
	void* user_stack = (void*) malloc(thread->stack_size);

	thread->user_stack = user_stack;

	user_mode_switch( (void*) thread->entry_point, (u32int) user_stack + thread->stack_size - 12);
}

/*-----------------------------------------------------------------------------
 *		Create user thread (for system call thread_create(...) )
 *---------------------------------------------------------------------------*/
thread_t* uthread_create(void* entry_point,
	               	     size_t stack_size,
	               	     bool suspend)
{
	return kthread_create(entry_point, stack_size, false, suspend);
}

/*-----------------------------------------------------------------------------
 *		Exit from user thread (for system call thread_exit(...) )
 *---------------------------------------------------------------------------*/
void uthread_exit(void)
{
	kthread_exit(get_current_thread());
}
