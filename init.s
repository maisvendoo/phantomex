/*------------------------------------------------------------------------------
//
//		PhantomEx, startap initialization code
//		(c) maisvendoo, 10.07.2013
//
//----------------------------------------------------------------------------*/

.code32

//------------------------------------------------------------------------------
//		Multibot header
//------------------------------------------------------------------------------
.set INIT_MBOOT_HEADER_MAGIC,           0x1BADB002
.set INIT_MBOOT_HEADER_FLAGS,           0x00000001
.set INIT_MBOOT_CHECKSUM,               0x00000000 - (INIT_MBOOT_HEADER_MAGIC + INIT_MBOOT_HEADER_FLAGS)

.extern main

.section .mboot

.int INIT_MBOOT_HEADER_MAGIC
.int INIT_MBOOT_HEADER_FLAGS
.int INIT_MBOOT_CHECKSUM

.section	.text

.global		init

init:
		cli /* Disable all interrupts */
		
		push	%esp
		push	%ebx
		
		call	main
		
		hlt
		
loop: /* loop infinity */ 
		jmp	loop
