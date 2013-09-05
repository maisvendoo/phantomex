/*------------------------------------------------------------------------------
//
//	System timer module
//	(c) maisvendoo, 06.07.2013
//
//----------------------------------------------------------------------------*/

#include	"timer.h"
#include	"isr.h"
#include	"text_framebuffer.h"
#include	"scheduler.h"

u32int tick = 0;

u8int hour = 0;
u8int min = 0;
u8int sec = 0;

/*------------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------*/
void timer_callback(registers_t regs)
{
	if (is_multitask())
	{
		task_switch();
	}
}

/*------------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------*/
void init_timer(u32int frequency)
{
  
  u32int divisor;
  u8int low;
  u8int high;
  
  register_interrupt_handler(IRQ0, &timer_callback);
  
  divisor = 1193180/frequency;
   
  outb(0x43, 0x36);
  
  low = (u8int) (divisor & 0xFF);
  high = (u8int) ( (divisor >> 8) & 0xFF);
  
  outb(0x40, low);
  outb(0x40, high);  
}
