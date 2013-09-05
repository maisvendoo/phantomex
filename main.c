/*------------------------------------------------------------------------------
//
//		PhantomEx main kernel module
//		(c) maisvendoo, 04.07.2013
//
//----------------------------------------------------------------------------*/
#include	"main.h"

u32int init_esp;

vscreen_t vs04;
vscreen_t vs01;

u8int start_y = 15;

vfs_node_t* fs_root = 0;

thread_t* th01 = 0;
thread_t* th02 = 0;

multiboot_header_t* multi_boot = 0;

u32int	count01 = 0;
u32int	count02 = 0;

/*------------------------------------------------------------------------------
//		Main kernel thread
//----------------------------------------------------------------------------*/
void thread02(void)
{

	vs04.cur_x = 10;
	vs04.cur_y = 19;
	vs04.vmemory = (u16int*) 0x15000000;

	char	tmp[256];

	while (1)
	{
		vs04.cur_x = 10;
		vs04.cur_y = 18;

		dec2dec_str(count02, tmp);

		vprint_text(&vs04, tmp);

		count02 += 4;
	}
}

/*------------------------------------------------------------------------------
//		User space thread
//----------------------------------------------------------------------------*/
void thread01(void)
{
	vs01.cur_x = 10;
	vs01.cur_y = 19;
	vs01.vmemory = (u16int*) 0x15000000;

	char	tmp[256];

	while (1)
	{
		vs01.cur_x = 10;
		vs01.cur_y = 19;

		dec2dec_str(count01, tmp);

		vprint(&vs01, tmp);

		count01++;
	}
}

/*------------------------------------------------------------------------------
//		Startup function
//----------------------------------------------------------------------------*/
int main(multiboot_header_t* mboot, u32int initial_esp)
{
	/* Get initial kernel stack top */
	init_esp = initial_esp;

	multi_boot = mboot;

	u32int*	mod_start = 0;
	u32int*	mod_end = 0;
	u32int	mods_count = multi_boot->mods_count;
	int		i = 0;

	/* Initialization of GDT and IDT */
	init_descriptor_tables();

	/* Registration of CPU interrupts */
	register_interrupt_handler(INT_0, &division_by_zero);
	register_interrupt_handler(INT_6, &fault_opcode);
	register_interrupt_handler(INT_8, &double_error);
	register_interrupt_handler(INT_10, &invalid_tss);
	register_interrupt_handler(INT_11, &segment_is_not_available);
	register_interrupt_handler(INT_12, &stack_error);
	register_interrupt_handler(INT_13, &general_protection_error);
	register_interrupt_handler(INT_14, &page_fault);

	clear();

	/* Memory manager initialization */
	check_memory_map((memory_map_entry_t*) mboot->mmap_addr, mboot->mmap_length);
	init_memory_manager(init_esp);

	/* Timer initialization */
	init_timer(BASE_FREQ);

	/* Enable all interrupts */
	asm volatile ("sti");

	/* Multitasking initialization */
	init_task_manager();

	/* System calls initialization */
	init_syscalls();

	/* I/O dispatcher initialization */
	/*init_io_dispatcher();*/

	/* kernel modules location */
	if (mods_count > 0)
	{
		mod_start = (u32int*) kmalloc(sizeof(u32int)*mods_count);
		mod_end = (u32int*) kmalloc(sizeof(u32int)*mods_count);

		for (i = 0; i < mods_count; i++)
		{
			mod_start[i] = *(u32int*)(multi_boot->mods_addr + 8*i);
			mod_end[i] = *(u32int*)(multi_boot->mods_addr + 8*i + 4);
		}
	}

	/* RAM FS initialization */
	fs_root = (vfs_node_t*) init_ramdisk(mod_start[0]);


	exec_proc("hello", false);

	th01 = kthread_create(&thread01, 0x4000, false, false);
	th02 = kthread_create(&thread02, 0x4000, true, false);

	print_text("Kernel heap free size: ");
	char tmp[256];
	print_dec_value(get_kernel_heap_free() / 1024);
	print_text(" KB\n");

	return 0;
}
