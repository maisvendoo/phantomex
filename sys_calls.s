/*-----------------------------------------------------------------------------
/
/       Call system call function
/       (c) maisvendoo, 21.08.2013
/
/----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
/      void syscall_entry_call(void* entry_point)
/----------------------------------------------------------------------------*/
.global     syscall_entry_call

syscall_entry_call:

        push    %edi
        push    %esi
        push    %edx            /* Push params into stack */
        push    %ecx
        push    %ebx
                        
        call    *24(%esp)       /* Call function at address in EDX  */
        
        add     $20, %esp       /* Clean stack */
        
        ret


