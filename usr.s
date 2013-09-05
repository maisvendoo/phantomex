/*-----------------------------------------------------------------------------
/
/		Switch to ring 3 and ring 0
/		(c) maisvendoo, 21.08.2013
/
/----------------------------------------------------------------------------*/
.set        USER_CS,        0x1B    /* user mode code selector */   
.set        USER_SS,        0x23    /* user mode stack selector */
.set        USER_DS,        0x23    /* user mode data selector */
.set        KERNEL_CS,      0x08
.set        IF_MASK,        0x200

/*-----------------------------------------------------------------------------
/       Switch to ring 3
/       void user_mode_switch(void* entry_point, u32int user_stack_top)
/----------------------------------------------------------------------------*/
.global user_mode_switch

user_mode_switch:
  
    mov     4(%esp), %edx           /* entry_point --> EDX  */
    
    /* Set user data selector */
    mov     $USER_DS, %ax
    mov     %ax, %ds
    mov     %ax, %es
    
    /* Prepare to SS, CS and EIP register loading */    
    mov     8(%esp), %eax   /* Save ESP state */
    pushl   $USER_SS        /* Push SS value into stack */
    pushl   %eax            /* Push ESP value into stack */
    
    pushf                   /* Push EFLAGS into stack  */
    pop     %eax
    or      $IF_MASK, %eax
    push    %eax
    
    push    $USER_CS        /* Push CS value into stack */
    push    %edx            /* Push EIP unto stack */ 
                            /* (user mode thread entry point) */
    
    iret                    /* Return from interrupt to ring 3! */

/*-----------------------------------------------------------------------------
/       Switch to ring 0 (used from system call!!!)
/----------------------------------------------------------------------------*/
.global kernel_mode_switch

.extern destroy_proc

kernel_mode_switch:
        
    jmp    destroy_proc
 
    


