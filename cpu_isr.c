/*-----------------------------------------------------------------------------
 *
 * 		CPU interrupt handlers
 * 		(c) maisvendoo, 30.07.2013
 *
 *---------------------------------------------------------------------------*/

#include		"cpu_isr.h"

/*------------------------------------------------------------------------------
//		Print registers
//----------------------------------------------------------------------------*/
void print_regs(registers_t regs)
{
	debug_msg("EAX = ", regs.eax);
	debug_msg("EBX = ", regs.ebx);
	debug_msg("ECX = ", regs.ecx);
	debug_msg("EDX = ", regs.edx);
	debug_msg("ESP = ", regs.esp);
	debug_msg("EBP = ", regs.ebp);
	debug_msg("EIP = ", regs.eip);
	debug_msg("EFLAGS = ", regs.eflags);
}

/*------------------------------------------------------------------------------
//		#DE - division by zero
//----------------------------------------------------------------------------*/
void division_by_zero(registers_t regs)
{
	print_text("Exception: DIVISION BY ZERO\n");
	print_regs(regs);
}

/*------------------------------------------------------------------------------
//		#UD - fault opcode
//----------------------------------------------------------------------------*/
void fault_opcode(registers_t regs)
{
  print_text("FAULT OPERATION CODE...\n");
  print_regs(regs);

  while (1);
}

/*------------------------------------------------------------------------------
//		#DF - double error
//----------------------------------------------------------------------------*/
void double_error(registers_t regs)
{
	print_text("Exception: DOUBLE EXCEPTION\n");
	debug_msg("Error code: ", regs.err_code);

	while (1);
}

/*------------------------------------------------------------------------------
//		#TS - Invalid TSS
//----------------------------------------------------------------------------*/
void invalid_tss(registers_t regs)
{
	u32int ext = regs.err_code & EXT_BIT;
	u32int idt = regs.err_code & IDT_BIT;
	u32int ti = regs.err_code & TI_BIT;
	u32int selector = regs.err_code & ERR_CODE_MASK;

	print_text("Exception: INVALID TSS\n");
	print_text("cause of error: ");

	if (ext)
	{
		print_text("HARDWARE INTERRUPT\n");
	}

	if (idt)
	{
		print_text("IDT GATE\n");
	}

	if (ti)
	{
		print_text("LDT GATE\n");
	}

	debug_msg("Invalid selector: ", selector);

	while (1);
}

/*------------------------------------------------------------------------------
//		#NP - Segment is't available
//----------------------------------------------------------------------------*/
void segment_is_not_available(registers_t regs)
{
	u32int ext = regs.err_code & EXT_BIT;
	u32int idt = regs.err_code & IDT_BIT;
	u32int ti = regs.err_code & TI_BIT;
	u32int selector = regs.err_code & ERR_CODE_MASK;

	print_text("Exception: SEGMENT IS'T AVAILABLE\n");
	print_text("cause of error: ");

	if (ext)
	{
		print_text("HARDWARE INTERRUPT\n");
	}

	if (idt)
	{
		print_text("IDT GATE\n");
	}

	if (ti)
	{
		print_text("LDT GATE\n");
	}

	debug_msg("Invalid selector: ", selector);

	while (1);
}

/*------------------------------------------------------------------------------
//		#SS - Stack error
//----------------------------------------------------------------------------*/
void stack_error(registers_t regs)
{
	print_text("Exception: STACK ERROR\n");
	debug_msg("Error code: ", regs.err_code);

	while (1);
}

/*------------------------------------------------------------------------------
//		#GP - General protection error
//----------------------------------------------------------------------------*/
void general_protection_error(registers_t regs)
{
	print_text("Exception: GENERAL PROTECTION ERROR\n");
	debug_msg("Error code: ", regs.err_code);

	while (1);
}

/*------------------------------------------------------------------------------
//		#PF - paging memory fault
//----------------------------------------------------------------------------*/
void page_fault(registers_t regs)
{
  u32int fault_addr = read_cr2();

  int present = !(regs.err_code & 0x1);		/* Page not present */
  int rw = regs.err_code & 0x2;				/* Page is read only */
  int user = regs.err_code & 0x4;			/* User mode */
  int reserved = regs.err_code & 0x8;		/* Reserved bits is writed */
  int id = regs.err_code & 0x10;			/* */

  print_text("Page fault: ");

  if (present)
    print_text("NOT PRESENT, ");

  if (rw)
    print_text("READ ONLY, ");

  if (user)
    print_text("USER MODE,  ");

  if (reserved)
    print_text("WRITING TO RESERVED BITS, ");

  if (id)
	print_text("EIP error ");

  print_text("at address ");
  print_hex_value(fault_addr);
  print_text("\n");

  while (1);
}
