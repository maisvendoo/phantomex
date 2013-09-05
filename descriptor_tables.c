/*------------------------------------------------------------------------------
//
//	Descriptor tables initialization
//	(c) maisvendoo, 06.07.2013
//
//----------------------------------------------------------------------------*/

#include	"descriptor_tables.h"
#include	"text_framebuffer.h"

/*******************************************************************************
	GDT
*******************************************************************************/

/* GDT loading function */
extern void gdt_flush(u32int);

static void init_gdt(void);

static void gdt_set_gate(s32int, u32int, u32int, u8int, u8int);

static	void init_idt(void);

static	void idt_set_gate(u8int, u32int, u16int, u8int);

gdt_entry_t	gdt_entries[6];

gdt_ptr_t	gdt_ptr;

idt_entry_t	idt_entries[256];
idt_ptr_t	idt_ptr;

tss_entry_t	tss;

extern u32int init_esp;

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void init_gdt(void)
{
  gdt_ptr.limit = (sizeof(gdt_entry_t)*6) - 1;
  gdt_ptr.base = (u32int) &gdt_entries;
  
  gdt_set_gate(0, 0, 0, 0, 0);	/* Null segment */
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Code segment */
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Data segment */
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* User mode code segment */
  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* User mode code segment */

  write_tss(5, 0x10, init_esp);
  
  gdt_flush( (u32int) &gdt_ptr);

  tss_flush(0x28);
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void init_descriptor_tables(void)
{
  init_gdt();
  init_idt();
}

/*------------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------*/
void gdt_set_gate(s32int num, u32int base, u32int limit, u8int access, u8int gran)
/*void gdt_set_gate(u32int num, u32int base, u32int limit, u8int dpl, u8int type)*/
{
  
  gdt_entries[num].base_low = (base & 0xFFFF);
  gdt_entries[num].base_middle = (base >> 16) & 0xFF;
  gdt_entries[num].base_high = (base >> 24) & 0xFF;
  
  gdt_entries[num].limit_low = (limit & 0xFFFF);
  gdt_entries[num].granularity = (limit >> 16) & 0xF;
  
  gdt_entries[num].granularity |= gran & 0xF0;
  gdt_entries[num].access = access;
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void write_tss(s32int num, u32int ss0, u32int esp0)
{
	memset(&tss, 0, sizeof(tss_entry_t));

	tss.ss0 = ss0;
	tss.esp0 = esp0;

	tss.cs = 0x08;

	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10;

	tss.iomap = 0xFF;
	tss.iomap_offset = (u16int) ( (u32int) &tss.iomap - (u32int) &tss );

	u32int base = (u32int) &tss;
	u32int limit = sizeof(tss)-1;

	tss_descriptor_t* tss_d = (tss_descriptor_t*) &gdt_entries[num];

	tss_d->base_15_0 = base & 0xFFFF;
	tss_d->base_23_16 = (base >> 16) & 0xFF;
	tss_d->base_31_24 = (base >> 24) & 0xFF;

	tss_d->limit_15_0 = limit & 0xFFFF;
	tss_d->limit_19_16 = (limit >> 16) & 0xF;

	tss_d->present = 1;
	tss_d->sys = 0;
	tss_d->DPL = 0;
	tss_d->type = 9;

	tss_d->AVL = 0;
	tss_d->allways_zero = 0;
	tss_d->gran = 0;
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void set_kernel_stack_in_tss(u32int stack)
{
	tss.esp0 = stack;
}

/*******************************************************************************
	IDT
*******************************************************************************/

extern void idt_flush(u32int);

/*------------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------*/
void init_idt(void)
{
  idt_ptr.limit = sizeof(idt_entry_t)*256 - 1;
  idt_ptr.base = (u32int) &idt_entries;
  
  memset(&idt_entries, 0, sizeof(idt_entry_t)*256);  
  
  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);
  outb(0x21, 0x0);
  outb(0xA1, 0x0);
  
  idt_set_gate(0, (u32int)isr0, 0x08, 0x8E);
  idt_set_gate(1, (u32int)isr1, 0x08, 0x8E);
  idt_set_gate(2, (u32int)isr2, 0x08, 0x8E);
  idt_set_gate(3, (u32int)isr3, 0x08, 0x8E);
  idt_set_gate(4, (u32int)isr4, 0x08, 0x8E);
  idt_set_gate(5, (u32int)isr5, 0x08, 0x8E);
  idt_set_gate(6, (u32int)isr6, 0x08, 0x8E);
  idt_set_gate(7, (u32int)isr7, 0x08, 0x8E);
  
  idt_set_gate(8, (u32int)isr8, 0x08, 0x8E);
  idt_set_gate(9, (u32int)isr9, 0x08, 0x8E);
  idt_set_gate(10, (u32int)isr10, 0x08, 0x8E);
  idt_set_gate(11, (u32int)isr11, 0x08, 0x8E);
  idt_set_gate(12, (u32int)isr12, 0x08, 0x8E);
  idt_set_gate(13, (u32int)isr13, 0x08, 0x8E);
  idt_set_gate(14, (u32int)isr14, 0x08, 0x8E);
  idt_set_gate(15, (u32int)isr15, 0x08, 0x8E);
  
  idt_set_gate(16, (u32int)isr16, 0x08, 0x8E);
  idt_set_gate(17, (u32int)isr17, 0x08, 0x8E);
  idt_set_gate(18, (u32int)isr18, 0x08, 0x8E);
  idt_set_gate(19, (u32int)isr19, 0x08, 0x8E);
  idt_set_gate(20, (u32int)isr20, 0x08, 0x8E);
  idt_set_gate(21, (u32int)isr21, 0x08, 0x8E);
  idt_set_gate(22, (u32int)isr22, 0x08, 0x8E);
  idt_set_gate(23, (u32int)isr23, 0x08, 0x8E);
  
  idt_set_gate(24, (u32int)isr24, 0x08, 0x8E);
  idt_set_gate(25, (u32int)isr25, 0x08, 0x8E);
  idt_set_gate(26, (u32int)isr26, 0x08, 0x8E);
  idt_set_gate(27, (u32int)isr27, 0x08, 0x8E);
  idt_set_gate(28, (u32int)isr28, 0x08, 0x8E);
  idt_set_gate(29, (u32int)isr29, 0x08, 0x8E);
  idt_set_gate(30, (u32int)isr30, 0x08, 0x8E);
  idt_set_gate(31, (u32int)isr31, 0x08, 0x8E);      
  
  idt_set_gate(32, (u32int)irq0, 0x08, 0x8E);
  idt_set_gate(33, (u32int)irq1, 0x08, 0x8E);
  idt_set_gate(34, (u32int)irq2, 0x08, 0x8E);
  idt_set_gate(35, (u32int)irq3, 0x08, 0x8E);
  idt_set_gate(36, (u32int)irq4, 0x08, 0x8E);
  idt_set_gate(37, (u32int)irq5, 0x08, 0x8E);
  idt_set_gate(38, (u32int)irq6, 0x08, 0x8E);
  idt_set_gate(39, (u32int)irq7, 0x08, 0x8E);
  idt_set_gate(40, (u32int)irq8, 0x08, 0x8E);
  idt_set_gate(41, (u32int)irq9, 0x08, 0x8E);
  idt_set_gate(42, (u32int)irq10, 0x08, 0x8E);
  idt_set_gate(43, (u32int)irq11, 0x08, 0x8E);
  idt_set_gate(44, (u32int)irq12, 0x08, 0x8E);
  idt_set_gate(45, (u32int)irq13, 0x08, 0x8E);
  idt_set_gate(46, (u32int)irq14, 0x08, 0x8E);
  idt_set_gate(47, (u32int)irq15, 0x08, 0x8E);
  
  /* System calls */
  idt_set_gate(0x50, (u32int)isr80, 0x08, 0xEF);

  idt_flush((u32int) &idt_ptr); 
}

/*------------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------*/
void idt_set_gate(u8int num, u32int base, u16int selector, u8int flags)
{
  idt_entries[num].base_low = base & 0xFFFF;
  idt_entries[num].base_high = (base >> 16) & 0xFFFF;
  
  idt_entries[num].selector = selector;
  idt_entries[num].allways0 = 0;
  
  idt_entries[num].flags = flags;/* - для пользовательского режима */
}

u32int get_tss_esp0(void)
{
	return tss.esp0;
}
