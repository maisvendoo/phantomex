/*-----------------------------------------------------------------------------
 *
 * 		Process's library
 * 		(c), maisvendoo 02.09.2013
 *
 *---------------------------------------------------------------------------*/

#include	"process.h"

u32int	next_pid = 0;

/*-----------------------------------------------------------------------------
 *		Execute kernel process
 *---------------------------------------------------------------------------*/
process_t* exec_proc(char* name, bool kernel)
{
	u32int		pdir_vaddr = 0;			/* Page directory virtual address */
	physaddr_t	page_dir = 0;			/* Page directory physical address */

	size_t		page_count;				/* Allocation page's count */
	size_t		stack_page_count;		/* Stack page's count */
	size_t		seg_page_count;			/* ELF segments page's count */
	size_t		heap_page_count;		/* Heap page's count */
	size_t		blocks_page_count;		/* Heap info blocks page count */

	physaddr_t	tmp_paddr = 0;			/* Temporary physical address */
	physaddr_t	stack_paddr = 0;
	physaddr_t	user_stack_paddr = 0;
	physaddr_t	seg_paddr = 0;
	physaddr_t	heap_paddr = 0;
	physaddr_t	blocks_paddr = 0;

	s8int		err = -1;				/* Error code */
	int			i = 0;
	size_t		sz = 0;					/* Reading data size */
	process_t*	proc = 0;				/* Process handler */
	thread_t*	thread = 0;				/* Thread handler */

	u32int		stack = 0;				/* Stack start address */
	u32int		stack_size = 0x4000;	/* Stack size */

	u32int		usr_stack = 0;

	u32int		eflags = 0;				/* EFLAGS buffer */
	u32int		seg_size = 0;

	heap_t*		heap;

	/* Load ELF info */
	elf_sections_t* elf = load_elf(name);

	/* Check file format */
	if (elf->elf_header->e_type != ET_EXEC)
	{
		print_text("This file is not executable...FAIL\n");
		return NULL;
	}

	/* Check architecture */
	if (elf->elf_header->e_mashine != EM_386)
	{
		print_text("This file is not for i386 architecture...FAIL\n");
		return NULL;
	}

	/* Create page directory */
	page_dir = clone_kernel_dir(&pdir_vaddr);

	/* Allocate pages for ELF segments */
	for (i = 0; i < elf->elf_header->e_phnum; i++)
		seg_size += elf->p_header[i].p_memsz;

	page_count = seg_size / PAGE_SIZE + 1;
	seg_page_count = page_count;

	tmp_paddr = alloc_phys_pages(page_count);
	seg_paddr = tmp_paddr;

	err = map_pages(page_dir,
			        (void*) elf->p_header[0].p_vaddr,
			        tmp_paddr,
			        page_count,
			        0x07);

	if (err == -1)
	{
		print_text("Memory mapping error...FAIL\n");
		return NULL;
	}

	/* kernel stack */
	stack = (u32int) kmalloc(stack_size);

	/* user stack */
	usr_stack = elf->p_header[0].p_vaddr + page_count*PAGE_SIZE;

	page_count = stack_size / PAGE_SIZE;
	tmp_paddr = alloc_phys_pages(page_count);
	user_stack_paddr = tmp_paddr;
	stack_page_count = page_count;

	err = map_pages(page_dir,
				    (void*) usr_stack,
				    tmp_paddr,
				    page_count,
				    0x07);

	if (err == -1)
	{
		print_text("Memory mapping error...FAIL\n");
		return NULL;
	}

	/* Process heap creation */
	page_count = USER_HEAP_SIZE / PAGE_SIZE;
	heap_page_count = page_count;

	tmp_paddr = alloc_phys_pages(page_count);
	heap_paddr = tmp_paddr;

	err = map_pages(page_dir,
				    USER_HEAP_START,
				    tmp_paddr,
				    page_count,
				    0x07);

	if (err == -1)
	{
		print_text("Memory mapping error...FAIL\n");
		return NULL;
	}

	page_count = USER_HEAP_INFO_SIZE / PAGE_SIZE;
	blocks_page_count = page_count;

	tmp_paddr = alloc_phys_pages(page_count);
	blocks_paddr = tmp_paddr;

	err =map_pages(page_dir,
				    USER_HEAP_BLOKS_INFO,
				    tmp_paddr,
				    page_count,
				    0x07);

	if (err == -1)
	{
		print_text("Memory mapping error...FAIL\n");
		return NULL;
	}

	/* Create process */
	proc = (process_t*) kmalloc(sizeof(process_t));

	proc->page_dir = page_dir;
	proc->pid = get_pid();
	proc->list_item.list = NULL;
	strcpy(proc->name, name);
	proc->suspend = false;
	proc->threads_count = 0;

	proc->page_dir_vaddr = (void*) pdir_vaddr;

	proc->stack_paddr = stack_paddr;
	proc->user_stack_vaddr = (void*) usr_stack;
	proc->user_stack_paddr = user_stack_paddr;
	proc->stack_page_count = stack_page_count;
	proc->seg_paddr = seg_paddr;
	proc->seg_page_count = seg_page_count;
	proc->heap_paddr = heap_paddr;
	proc->heap_page_count = heap_page_count;
	proc->blocks_paddr = blocks_paddr;
	proc->blocks_page_count = blocks_page_count;

	add_process(proc);

	/* Create main thread */
	thread = (thread_t*) kmalloc(sizeof(thread_t));

	thread->id = get_thread_id();
	thread->suspend = false;
	thread->process = proc;
	thread->entry_point = elf->elf_header->e_entry;
	thread->list_item.list = NULL;
	thread->stack = (void*) stack;
	thread->stack_size = stack_size;
	thread->stack_top = stack + stack_size;
	thread->esp = stack + stack_size - 28;

	proc->thread_id[proc->threads_count++] = thread->id;

	u32int* esp = (u32int*) (stack + stack_size);

	eflags = read_eflags();

	eflags |= (1 << 9);

	if (kernel)
	{
		esp[-4] = (u32int) &destroy_proc;
		esp[-5] = elf->elf_header->e_entry;
		esp[-7] = eflags;
	}
	else
	{
		esp[-2] = (u32int) proc;
		esp[-3] = (u32int) elf;
		esp[-5] = (u32int) &start_elf;
		esp[-7] = eflags;
	}

	add_thread(thread);

	return proc;
}

/*-----------------------------------------------------------------------------
 *		Get process ID (PID)
 *---------------------------------------------------------------------------*/
u32int get_pid(void)
{
	return next_pid++;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void destroy_proc(void)
{
	thread_t*	thread;
	process_t*	proc;

	/*asm volatile ("cli");*/
	stop();

	thread = get_current_thread();
	proc = thread->process;

	remove_thread(thread);

	/* Free process memory pages */
	kfree(thread->stack);
	thread->stack = NULL;

	free_phys_pages(proc->user_stack_paddr, proc->stack_page_count);
	free_phys_pages(proc->seg_paddr, proc->seg_page_count);
	free_phys_pages(proc->heap_paddr, proc->heap_page_count);
	free_phys_pages(proc->blocks_paddr, proc->blocks_page_count);

	kfree(thread);
	thread = NULL;

	proc->threads_count--;

	while (proc->threads_count > 0)
	{
		thread = get_thread(proc->thread_id[proc->threads_count - 1]);

		remove_thread(thread);

		thread->process->threads_count--;

		/* Free thread's memory (handler and stack) */
		kfree(thread->stack);
		thread->stack = NULL;
		kfree(thread);
		thread = NULL;
	}

	/* Here we must free memory!!! */

	/*asm volatile ("sti");*/
	start();

	task_switch();
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
process_t* usr_exec(char* name)
{
	return exec_proc(name, false);
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void start_elf(elf_sections_t* elf, process_t* proc)
{
	heap_t* heap = USER_HEAP_START;
	size_t	sz = 0;
	int		i = 0;
	u32int*	esp;
	void*	entry_point = 0;

	for (i = 0; i < elf->elf_header->e_phnum; i++)
	{
		fseek(elf->file, elf->p_header[i].p_offset);

		/* Copy non zero bytes */
		sz = fread(elf->file, (void*) elf->p_header[i].p_vaddr, elf->p_header[i].p_filesz);
		/* Set zero in other segment field */
		memset((void*) (elf->p_header[i].p_vaddr + sz), 0, elf->p_header[i].p_memsz - sz);
	}

	esp = (u32int*) ((u32int) proc->user_stack_vaddr + proc->stack_page_count*PAGE_SIZE - 20);

	esp[0] = (u32int) &exit;

	/* Process heap initialization */
	heap = (heap_t*) USER_HEAP_START;

	heap->start = USER_HEAP_START;
	heap->size = USER_HEAP_SIZE;
	heap->end = heap->start + heap->size;
	heap->count = 1;

	heap->blocks = (memory_block_t*) USER_HEAP_BLOKS_INFO;
	heap->blocks[0].base = heap->start;
	heap->blocks[0].size = sizeof(heap_t);

	entry_point = (void*) elf->elf_header->e_entry;

	kfree(elf->elf_header);
	kfree(elf->p_header);
	kfree(elf->section);
	kfree(elf);

	user_mode_switch(entry_point, (u32int) esp);
}
