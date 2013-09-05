/*-----------------------------------------------------------------------------
 *
 * 		I/O ports dispatcher
 * 		(c) maisvendoo, 22.08.2013
 *
 *---------------------------------------------------------------------------*/

#include	"io_disp.h"

/*-----------------------------------------------------------------------------
 *		Globals
 *---------------------------------------------------------------------------*/
mutex_t*	port_mutex;

/*-----------------------------------------------------------------------------
 *		Initialization
 *---------------------------------------------------------------------------*/
void init_io_dispatcher(void)
{
	int i = 0;

	/* Allocate memory for port's mutexes and release them */
	port_mutex = (mutex_t*) kmalloc(sizeof(mutex_t)*PORTS_NUM);

	for (i = 0; i < PORTS_NUM; i++)
	{
		mutex_release(&port_mutex[i]);
	}
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
u8int in_byte(u16int port)
{
	u8int value = 0;

	mutex_get(&port_mutex[port], true);

	value = inb(port);

	mutex_release(&port_mutex[port]);

	return value;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void out_byte(u16int port, u8int value)
{
	mutex_get(&port_mutex[port], true);

	outb(port, value);

	mutex_release(&port_mutex[port]);
}
