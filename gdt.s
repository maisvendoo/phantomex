/*-----------------------------------------------------------------------------
 *		Load IDT 
 *---------------------------------------------------------------------------*/
.global	gdt_flush

gdt_flush:

	mov		4(%esp), %eax
	lgdt	(%eax)
	
	mov		$0x10, %ax
	mov		%ax, %ds
	mov		%ax, %es
	mov		%ax, %fs
	mov		%ax, %gs
	mov		%ax, %ss
	
	ljmp	$0x08,$flush
	
flush:
	ret

/*-----------------------------------------------------------------------------
 *		Load IDT 
 *---------------------------------------------------------------------------*/
.global idt_flush

idt_flush:

	mov		4(%esp), %eax
	lidt	(%eax)
	ret

/*-----------------------------------------------------------------------------
 *		Load TR
 *---------------------------------------------------------------------------*/	
.global tss_flush

tss_flush:

    mov 	4(%esp), %eax
	ltr		%ax
	ret

/*-----------------------------------------------------------------------------
 *		Read TR
 *---------------------------------------------------------------------------*/	
 .global get_tr
 
 get_tr:
 
 	xor		%eax, %eax
 	str		%eax
 	ret
 