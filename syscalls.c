/*-----------------------------------------------------------------------------
 *
 * 		System calls interface
 * 		(c) maisvendoo, 21.08.2013
 *
 *---------------------------------------------------------------------------*/

#include	"syscalls.h"

/*-----------------------------------------------------------------------------
 * 		System calls table in fixed memory field
 *---------------------------------------------------------------------------*/
u32int* calls_table = (u32int*) SYSCALLS_TABLE;

/*-----------------------------------------------------------------------------
 * 		System call handler
 *---------------------------------------------------------------------------*/
void syscall_handler(registers_t regs)
{
	if (regs.eax >= NUM_CALLS)
		return;

	void* syscall = (void*) calls_table[regs.eax];

	syscall_entry_call(syscall);
}

/*-----------------------------------------------------------------------------
 * 		System calls initialization
 *---------------------------------------------------------------------------*/
void init_syscalls(void)
{
	/* Register interrupt handler */
	register_interrupt_handler(0x50, &syscall_handler);

	/* Fill function's table */
	calls_table[SYSCALL_EXIT] = (u32int) &destroy_proc;
	calls_table[SYSCALL_EXEC] = (u32int) &usr_exec;
	calls_table[SYSCALL_THREAD_CREATE] = (u32int) &uthread_create;
	calls_table[SYSCALL_THREAD_EXIT] = (u32int) &uthread_exit;

	calls_table[SYSCALL_VPRINT] = (u32int) &vprint_text;
}
