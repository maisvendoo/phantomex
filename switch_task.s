/*-----------------------------------------------------------------------------
/
/		Switch tasks
/		(c) maisvendoo, 21.08.2013
/
/----------------------------------------------------------------------------*/
.extern		current_thread
.extern		tss

/*-----------------------------------------------------------------------------
/       void task_switch(void)
/----------------------------------------------------------------------------*/
.global		task_switch

task_switch:

			push	%ebp 				/* C-function prolog */
			
			pushf						/* Save EFLAGS in thread stack */
			cli							/* Disable all interrupts */
			
			/* Save current task's ESP */
			mov	current_thread, %edx
			mov	%esp, 28(%edx)

			/* Get next task handler */
			mov	4(%edx), %ecx
			mov %ecx, current_thread
				
						
			mov current_thread, %edx
			
			/* Set page directory */
            mov 12(%edx), %ebx          /* current_thread->process --> EBX */
            mov 12(%ebx), %ecx          /* process->page_dir --> ECX */
            mov %ecx, %cr3              /* reload CR3 */
            
			/* Set new ESP */
			mov 28(%edx), %esp			
			
			/* Set TSS kernel stack */
			mov 40(%edx), %eax			/* current_thread->stack_top --> EAX */
			mov $tss, %edx				/* tss address --> EDX */
			mov %eax, 4(%edx)			/* EAX --> tss.esp */			
			
			popf						/* Restore EFLAGS from new task stack */
			
			pop		%ebp				/* C-function epilog */            
            
			ret

