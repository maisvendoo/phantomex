/*-----------------------------------------------------------------------------
 *
 * 		Tasks scheduler
 * 		(c) maisvendoo, 11.08.2013
 *
 *---------------------------------------------------------------------------*/

#include	"scheduler.h"

/*-----------------------------------------------------------------------------
 * 		Globals
 *---------------------------------------------------------------------------*/
list_t		process_list;			/* List of process */
list_t		thread_list;			/* List of threads */
list_t		thread_wait;			/* List of suspended threads */

bool		multi_task = false;		/* Multitasking ready flag */

process_t*	kernel_proc = 0;		/* Kernel process handler */
thread_t*	kernel_thread = 0;		/* Main kernel thread handler */

process_t*	current_proc;			/* Current process */

thread_t*	current_thread;			/* Current thread */

extern	u32int init_esp;

physaddr_t	current_pdir = 0;

/*-----------------------------------------------------------------------------
 *		Initialization of task manager
 *---------------------------------------------------------------------------*/
void init_task_manager(void)
{
	u32int tmp_vaddr = 0;
	heap_t* heap = USER_HEAP_START;
	s8int	err = -1;
	physaddr_t tmp_paddr = 0;
	size_t page_count = 0;

	/* Disable all interrupts */
	u32int esp = read_esp();

	list_init(&process_list);
	list_init(&thread_list);
	list_init(&thread_wait);

	physaddr_t page_dir = clone_kernel_dir(&tmp_vaddr);

	write_cr3(page_dir);

	page_count = USER_HEAP_SIZE / PAGE_SIZE;

	tmp_paddr = alloc_phys_pages(page_count);


	err = map_pages(page_dir,
				    USER_HEAP_START,
				    tmp_paddr,
				    page_count,
				    0x07);

	if (err == -1)
	{
		print_text("Memory mapping error...FAIL\n");
		return;
	}

	page_count = USER_HEAP_INFO_SIZE / PAGE_SIZE;


	tmp_paddr = alloc_phys_pages(page_count);


	err =map_pages(page_dir,
	 		       USER_HEAP_BLOKS_INFO,
				   tmp_paddr,
				   page_count,
				   0x07);

	if (err == -1)
	{
		print_text("Memory mapping error...FAIL\n");
		return;
	}

	/* Process heap initialization */
	heap = (heap_t*) USER_HEAP_START;

	heap->start = USER_HEAP_START;
	heap->size = USER_HEAP_SIZE;
	heap->end = heap->start + heap->size;
	heap->count = 1;

	heap->blocks = (memory_block_t*) USER_HEAP_BLOKS_INFO;
	heap->blocks[0].base = heap->start;
	heap->blocks[0].size = sizeof(heap_t);

	/* Create kernel process */
	kernel_proc = (process_t*) kmalloc(sizeof(process_t));

	memset(kernel_proc, 0, sizeof(process_t));

	/* Kernel process creation */
	kernel_proc->pid = get_pid();
	kernel_proc->page_dir = page_dir;
	kernel_proc->list_item.list = NULL;
	kernel_proc->threads_count = 1;
	strcpy(kernel_proc->name, "Kernel");
	kernel_proc->suspend = false;

	list_add(&process_list, &kernel_proc->list_item);

	/* Create kernel thread */
	kernel_thread = (thread_t*) kmalloc(sizeof(thread_t));

	memset(kernel_thread, 0, sizeof(thread_t));

	kernel_thread->process = kernel_proc;
	kernel_thread->list_item.list = NULL;
	kernel_thread->id = get_thread_id();
	kernel_thread->stack_size = 0x4000;
	kernel_thread->suspend = false;
	kernel_thread->esp = esp;
	kernel_thread->stack_top = init_esp;

	list_add(&thread_list, &kernel_thread->list_item);

	current_proc = kernel_proc;
	current_thread = kernel_thread;

	set_kernel_stack_in_tss(kernel_thread->stack_top);

	/* Enable multitasking flag */
	start();
}

/*-----------------------------------------------------------------------------
 *		Get current process handler
 *---------------------------------------------------------------------------*/
process_t* get_current_proc(void)
{
	return current_proc;
}

/*-----------------------------------------------------------------------------
 *		Check task manager is ready
 *---------------------------------------------------------------------------*/
bool is_multitask(void)
{
	return multi_task;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
thread_t* get_current_thread(void)
{
	return current_thread;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
thread_t* get_thread(u32int id)
{
	thread_t*	thread = NULL;
	u32int		idx = 0;

	thread = current_thread;

	do
	{
		thread = (thread_t*) thread->list_item.next;
		idx++;

		if (thread->id == id)
			break;

	} while (idx < thread->list_item.list->count);

	if (idx == thread->list_item.list->count)
		return NULL;
	else
		return thread;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void add_thread(thread_t* thread)
{
	list_add(&thread_list, &thread->list_item);
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void remove_thread(thread_t* thread)
{
	list_remove(&thread->list_item);
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void add_process(process_t* proc)
{
	list_add(&process_list, &proc->list_item);
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void add_wait(thread_t* thread)
{
	list_add(&thread_wait, &thread->list_item);
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void remove_wait(thread_t* thread)
{
	list_remove(&thread->list_item);
}

void start(void)
{
	multi_task = true;
}

void stop(void)
{
	multi_task = false;
}
