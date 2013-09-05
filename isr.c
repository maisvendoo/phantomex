/*------------------------------------------------------------------------------
//
//	High level interrupt handler
//	(c) maisvendoo, 06.07.2013
//
//----------------------------------------------------------------------------*/

#include	"isr.h"
#include	"text_framebuffer.h"

isr_t	interrupt_handlers[256];

/*------------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------*/
void isr_handler(registers_t regs)
{
    
  if (interrupt_handlers[regs.int_num] != 0)
  {
    isr_t handler = interrupt_handlers[regs.int_num];
    
    handler(regs);
  }
  
}

/*------------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------*/
void irq_handler(registers_t regs)
{
  if (regs.int_num >= 40)
  {
    outb(0xA0, 0x20);
  }
  
  outb(0x20, 0x20);
  
  if (interrupt_handlers[regs.int_num] != 0)
  {
    isr_t handler = interrupt_handlers[regs.int_num];
    
    handler(regs);
  }
  
}

/*------------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------*/
void register_interrupt_handler(u8int n, isr_t handler)
{
  interrupt_handlers[n] = handler;      
}
