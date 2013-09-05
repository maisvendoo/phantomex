/*-----------------------------------------------------------------------------
 *
 * 		Memory manager
 * 		(c) maisvendoo, 30.07.2013
 *
 *---------------------------------------------------------------------------*/

#include		"memory.h"
#include		"text_framebuffer.h"

/*------------------------------------------------------------------------------
//		Any globals
//----------------------------------------------------------------------------*/
u64int				phys_memory_size = 0;						/* Size of physical memory */
memory_map_entry_t*	mentry = 0;									/* Memory map entry */

size_t				free_pages_count = 0;						/* Number of free pages */
physaddr_t			free_phys_memory_pointer = -1;				/* Free physical memory pointer */

physaddr_t			kernel_page_dir = KERNEL_PAGE_TABLE;		/* Kernel page directory address */

size_t				memory_size = KERNEL_BASE + KERNEL_SIZE;	/* Start memory size */

u32int				old_kernel_stack;

mutex_t				phys_memory_mutex;							/* Mutex for physical memory */
																/* synchronize access */

heap_t				kheap;										/* Kernel heap */

/*------------------------------------------------------------------------------
//		Check memory map
//----------------------------------------------------------------------------*/
void check_memory_map(memory_map_entry_t* mmap_addr, u32int length)
{
	int i = 0;
	/* Entries number in memory map structure */
	int n = length / sizeof(memory_map_entry_t);

	/* Set pointer to memory map */
	mentry = mmap_addr;

	/* Print info about physical memory allocation */
	print_text("Physical memory map\n");
	for (i = 0; i < n; i++)
	{
		if ((mentry + i)->type == 1)
			print_text("Available: |");
		else
			print_text("Reserved:  |");

		print_text(" addr: ");
		print_hex_value((mentry + i)->addr);

		print_text(" length: ");
		print_hex_value((mentry + i)->len);

		print_text("\n");

		phys_memory_size += (mentry + i)->len;
	}

	print_text("Installed memory size: ");
	print_dec_value(phys_memory_size / 1024);
	print_text(" KB\n");
}

/*------------------------------------------------------------------------------
//		Check memory map
//----------------------------------------------------------------------------*/
void temp_map_page(physaddr_t addr)
{
	u32int table_idx = TEMP_PAGE >> 22;
	u32int page_idx = (TEMP_PAGE >> 12) & 0x3FF;
	u32int* pages = (u32int*) (KERNEL_PAGE_TABLE + PAGE_SIZE);

	*(pages + (table_idx << 10) + page_idx) = (u32int) addr | PAGE_PRESENT | PAGE_WRITEABLE;

	asm volatile ("invlpg (,%0,)"::"a"(TEMP_PAGE));
}

/*------------------------------------------------------------------------------
//		Allocate page in physical memory
//----------------------------------------------------------------------------*/
physaddr_t alloc_phys_pages(size_t count)
{
	physaddr_t result = -1;
	physmemory_pages_block_t* tmp_block;

	/*mutex_get(&phys_memory_mutex, true);*/
	/*lock(get_current_thread()->id);*/

	/* Number of free pages less than count - error!*/
	if (free_pages_count < count)
		return -1;

	if (free_phys_memory_pointer != -1)
	{

		physaddr_t	cur_block = free_phys_memory_pointer;

		do
		{
			/* Get current free block*/
			tmp_block = get_free_block(cur_block);

			/* If block and count have same size*/
			if ( tmp_block->size == count )
			{
				/* Remember next and previous */
				physaddr_t next = tmp_block->next;
				physaddr_t prev = tmp_block->prev;

				/* Get next block */
				tmp_block = get_free_block(next);

				/* For next block - previous is previous for allocated block */
				tmp_block->prev = prev;

				/* Get previous block*/
				tmp_block = get_free_block(prev);

				/* For next block - next is next for allocated block */
				tmp_block->next = next;

				/* If it was penultimate free block */
				if (cur_block == free_phys_memory_pointer)
				{
					/* Check next */
					free_phys_memory_pointer = next;

					/* If next block is current - we have no free memory */
					if (cur_block == free_phys_memory_pointer)
					{
						free_phys_memory_pointer = -1;
					}
				}

				/* Result address of allocated block */
				result = cur_block;
				break;
			}

			/* If current free block more than count of pages */
			if ( tmp_block->size > count )
			{
				/* Reduce size of current block */
				tmp_block->size -= count;

				/* Result address - current block's address + size of allocated block*/
				result = cur_block + (tmp_block->size << PAGE_OFFSET_BITS);
				break;
			}

			/* Get next free block */
			cur_block = tmp_block->next;

		} while (cur_block != free_phys_memory_pointer);

		/* If we have correct address */
		if (result != -1)
		{
			/* Reduce free page count */
			free_pages_count -= count;
		}
	}

	/*mutex_release(&phys_memory_mutex);*/
	/*unlock(get_current_thread()->id);*/

	return result;
}

/*------------------------------------------------------------------------------
//		Get free block
//----------------------------------------------------------------------------*/
physmemory_pages_block_t* get_free_block(physaddr_t paddr)
{
	temp_map_page(paddr);

	return (physmemory_pages_block_t*) TEMP_PAGE;
}

/*------------------------------------------------------------------------------
//		Free pages in physical memory
//----------------------------------------------------------------------------*/
void free_phys_pages(physaddr_t base, size_t count)
{
	physmemory_pages_block_t* tmp_block;

	/*mutex_get(&phys_memory_mutex, true);*/
	/*lock(get_current_thread()->id);*/

	/* There are no free blocks */
	if (free_phys_memory_pointer == -1)
	{
		tmp_block = get_free_block(base);

		tmp_block->prev = base;
		tmp_block->next = base;
		tmp_block->size = count;

		free_phys_memory_pointer = base;
	}
	else
	{
		/* Get first free block address */
		physaddr_t cur_block = free_phys_memory_pointer;

		do
		{
			tmp_block = get_free_block(cur_block);

			/* If address after current block */
			if (base == cur_block + (tmp_block->size << PAGE_OFFSET_BITS))
			{
				tmp_block->size += count;

				/* If after new block is other free block*/
				if (tmp_block->next == base + (count << PAGE_OFFSET_BITS) )
				{
					/* Remember next block address*/
					physaddr_t next_old = tmp_block->next;

					/* Get next block*/
					tmp_block = get_free_block(next_old);

					/* Get new next block address and size */
					physaddr_t next_new = tmp_block->next;
					size_t new_count = tmp_block->size;

					/* Set current block as previous and */
					/* new next as next */
					tmp_block = get_free_block(next_new);

					tmp_block->prev = cur_block;

					tmp_block = get_free_block(cur_block);

					tmp_block->next = next_new;
					tmp_block->size += new_count;
				}

				break;
			}

			/* If address before current block */
			if (cur_block == base + (count << PAGE_OFFSET_BITS))
			{
				/* Remember old size and next and previous */
				size_t old_count = tmp_block->size;
				physaddr_t next = tmp_block->next;
				physaddr_t prev = tmp_block->prev;

				/* Get next block */
				tmp_block = get_free_block(next);

				/* New block is previous */
				tmp_block->prev = base;

				/* Get previous block*/
				tmp_block = get_free_block(prev);

				/* New block is next too */
				tmp_block->next = base;

				/* Get new block */
				tmp_block = get_free_block(base);

				tmp_block->next = next;
				tmp_block->prev = prev;
				tmp_block->size += old_count;

				break;
			}

			/* If address between free blocks */
			if ( cur_block > base  )
			{
				physaddr_t prev = tmp_block->next;
				tmp_block->prev = base;

				tmp_block = get_free_block(prev);
				tmp_block->next = base;

				tmp_block = get_free_block(base);
				tmp_block->next = cur_block;
				tmp_block->prev = prev;
				tmp_block->size = count;

				break;
			}

			/* or single free block */
			if (tmp_block->next == free_phys_memory_pointer)
			{
				tmp_block->prev = base;
				physaddr_t next = tmp_block->next;

				tmp_block->next = base;

				tmp_block = get_free_block(base);

				tmp_block->prev = cur_block;
				tmp_block->next = cur_block;
				tmp_block->size = count;

				break;
			}

			cur_block = tmp_block->next;

		} while ( cur_block != free_phys_memory_pointer );

		if (base < free_phys_memory_pointer)
		{
			free_phys_memory_pointer = base;
		}
	}

	free_pages_count += count;

	/*mutex_release(&phys_memory_mutex);*/
	/*unlock(get_current_thread()->id);*/
}

/*------------------------------------------------------------------------------
//		Init memory manager
//----------------------------------------------------------------------------*/
void init_memory_manager(u32int stack)
{
	old_kernel_stack = stack;

	/* Switch CPU to page memory mode*/
	switch_page_mode();

	/* Check available memory */
	memory_map_entry_t* entry;

	for (entry = mentry; entry->type; entry++)
	{
		if ( (entry->type == 1) && (entry->addr >= 0x100000) )
		{
			free_phys_pages(entry->addr, entry->len >> PAGE_OFFSET_BITS);
			memory_size += entry->len;
		}
	}

	print_text("Free pages: ");
	print_dec_value(free_pages_count);
	print_text(" pages\n");

	/* Kernel heap creation */

	/* Map kernel heap memory */
	map_pages(KERNEL_PAGE_TABLE,
			  KERNEL_HEAP_BASE,
			  (physaddr_t) (KERNEL_MEMORY_START + KERNEL_SIZE + KERNEL_HEAP_BLOCK_INFO_SIZE),
			  KERNEL_HEAP_SIZE >> PAGE_OFFSET_BITS,
			  0x03);

	/* Map memory blocks info structure */
	map_pages(KERNEL_PAGE_TABLE,
			  (void*) (KERNEL_MEMORY_START + KERNEL_SIZE),
			  (physaddr_t) (KERNEL_MEMORY_START + KERNEL_SIZE),
			  KERNEL_HEAP_BLOCK_INFO_SIZE >> PAGE_OFFSET_BITS,
			  0x03);

	void* new_vram = (void*) 0x15000000;

	map_pages(KERNEL_PAGE_TABLE,
			  new_vram,
			  0xB8000,
			  1,
			  0x03);

	set_video_vaddr(new_vram);

	kheap.blocks = (memory_block_t*) (KERNEL_MEMORY_START + KERNEL_SIZE);

	memset(kheap.blocks, 0, KERNEL_HEAP_BLOCK_INFO_SIZE);

	kheap.count = 0;
	kheap.start = KERNEL_HEAP_BASE;
	kheap.size = KERNEL_HEAP_SIZE;
	kheap.end = kheap.start + kheap.size;
}

/*------------------------------------------------------------------------------
//		Get free memory size
//----------------------------------------------------------------------------*/
size_t get_free_memory_size(void)
{
	return free_pages_count << PAGE_OFFSET_BITS;
}

/*------------------------------------------------------------------------------
//		Switch page memory mode
//----------------------------------------------------------------------------*/
void switch_page_mode(void)
{
	u64int		vaddr = 0;
	u32int		frame = 0;
	physaddr_t	paddr = 0;
	u32int		table_idx = 0;
	u32int		page_idx = 0;
	u32int		cr0;
	u32int		table_flags = 0;
	u32int		page_flags = 0;

	/* Create kernel page directory */
	u32int* kernel_dir = (u32int*) kernel_page_dir;
	/* Create pointer to page's tables */
	u32int* page_table = (u32int*) (kernel_page_dir + PAGE_SIZE);

	/* Clear kernel directory */
	memset(kernel_dir, 0, PAGE_SIZE);

	/* Map all memory pages for kernel */
	for (vaddr = 0; vaddr < (KERNEL_BASE + KERNEL_SIZE); vaddr += PAGE_SIZE)
	{
		/* Translate virtual address */
		frame = vaddr >> PAGE_OFFSET_BITS;
		table_idx = frame >> PAGE_TABLE_INDEX_BITS;
		page_idx = frame & PAGE_TABLE_INDEX_MASK;

		/* Set table flags */
		table_flags = PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER;

		/* Create page table */
		*(kernel_dir + table_idx) = (u32int) (page_table + table_idx*0x400) | table_flags;

		/* Set page flags */
		page_flags = PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER;

		/* Create page */
		*(page_table + table_idx*0x400 + page_idx) = paddr | page_flags;

		paddr += PAGE_SIZE;
	}

	/* Load kernel directory address to CR3 */
	write_cr3(kernel_page_dir);

	/* Switch to paging mode */
	write_cr0(read_cr0() | PAGE_MODE_MASK);
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
physaddr_t get_page_info(physaddr_t page_dir, void *vaddr)
{
	physaddr_t	page_table = page_dir;
	char		shift;

	for (shift = PHYSADDR_BITS - PAGE_TABLE_INDEX_BITS;
		 shift >= PAGE_OFFSET_BITS;
		 shift -= PAGE_TABLE_INDEX_BITS)
	{
		u32int index = ((size_t) vaddr >> shift) & PAGE_TABLE_INDEX_MASK;
		temp_map_page(page_table);

		if (shift > PAGE_OFFSET_BITS)
		{
			page_table = ((physaddr_t*) TEMP_PAGE)[index];

			if (!(page_table & PAGE_PRESENT))
			{
				return 0;
			}
		}
		else
		{
			page_table = page_table & ~PAGE_OFFSET_MASK;
			temp_map_page(page_table);
			return ((physaddr_t*) TEMP_PAGE)[index];
		}
	}

	return 0;

}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
u8int map_pages(physaddr_t page_dir,	/* Page directory*/
		        void* vaddr,			/* Start virtual address */
		        physaddr_t paddr,		/* Start physical address */
		        size_t count,			/* Size of memory space */
		        u32int flags)			/* Page's flags */
{
	/* Pointer for access to temporary page */
	physaddr_t* tmp_page = (physaddr_t*) TEMP_PAGE;

	physaddr_t table;

	u32int table_flags;

	/* Create pages in cycle */
	for(; count; count--)
	{
		/* Virtual address translation */
		u32int table_idx = (u32int) vaddr >> 22;
		u32int page_idx = ((u32int) vaddr >> PAGE_OFFSET_BITS) & PAGE_TABLE_INDEX_MASK;

		/* Get kernel page directory */
		temp_map_page(page_dir);

		/* Get table by index */
		table = tmp_page[table_idx];

		/* If table is not present */
		if ( !(table & PAGE_PRESENT) )
		{
			/* Allocate page for table */
			physaddr_t addr = alloc_phys_pages(1);

			/* Address is correct? */
			if (addr != -1)
			{
				/* Get physical memory for table */
				temp_map_page(addr);

				/* Clear it */
				memset(tmp_page, 0, PAGE_SIZE);

				/* Get page directory */
				temp_map_page(page_dir);

				/* Set table descriptor */
				table_flags = PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER;
				tmp_page[table_idx] = (addr & ~PAGE_OFFSET_MASK) | table_flags;

				/* New table address */
				table = addr;
			}
			else
			{
				/* Exit with error code */
				return FALSE;
			}
		}

		/* Delete table flags from address */
		table &= ~PAGE_OFFSET_MASK;

		/* Get table's memory */
		temp_map_page(table);

		/* Set page descriptor in table */
		tmp_page[page_idx] = (paddr & ~PAGE_OFFSET_MASK) | flags;

		/* Update TLB */
		asm volatile ("invlpg (,%0,)"::"a"(vaddr));

		/* New addresses */
		vaddr += PAGE_SIZE;
		paddr += PAGE_SIZE;
	}

	/* Return without error */
	return TRUE;
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
u8int move_kernel_stack(void* kernel_stack_vaddr,
		                size_t kernel_stack_size,
		                u32int old_kernel_stack)
{
	u8int	is_ok = TRUE;

	u32int	old_esp;
	u32int	old_ebp;
	u32int	new_esp;
	u32int	new_ebp;
	u32int	offset;
	u32int	i;
	u32int	new_stack_end;
	u32int	npages = 0;

	/* Read old ESP and EBP */
	old_esp = read_esp();
	old_ebp = read_ebp();

	/* Offset between old stack and new stack */
	offset = KERNEL_STACK - old_kernel_stack;

	/* Calculate new ESP and new EBP */
	new_esp = old_esp + offset;
	new_ebp = old_ebp + offset;

	new_stack_end = (u32int) kernel_stack_vaddr - kernel_stack_size;
	npages = (kernel_stack_size >> PAGE_OFFSET_BITS) + 2;

	/* Allocate pages for stack */
	u32int* stack_ptr = kmalloc(kernel_stack_size);

	if (stack_ptr == NULL)
	{
		return FALSE;
	}

	/* Get pointer to both stacks */
	u32int* old_stack = (u32int*) old_kernel_stack;
	u32int* new_stack = (u32int*) kernel_stack_vaddr;

	/* Copy stack byte to byte */
	memcpy(stack_ptr, (void*) (old_kernel_stack - kernel_stack_size), kernel_stack_size);

	/* Change ESP and EBP values in new stack */
	for (i = (u32int) stack_ptr + kernel_stack_size;
		 i >= (u32int) stack_ptr;
		 i -= 4)
	{
		u32int tmp = *(u32int*) i;

		if ( (tmp > old_esp) && ( tmp < old_kernel_stack) )
		{
			u32int* tmp2 = (u32int*) i;

			tmp += offset;

				*tmp2 = tmp;
		}
	}

	/* Switch to new stack */
	asm volatile ("mov %0, %%esp"::"r"(new_esp));
	asm volatile ("mov %0, %%ebp"::"r"(new_ebp));

	return is_ok;
}

/*------------------------------------------------------------------------------
//		Check is blocks overlapped
//----------------------------------------------------------------------------*/
static inline u8int is_blocks_overlapped(void* base1,
										 size_t size1,
										 void* base2,
										 size_t size2)
{
	return (( base1 >= base2 ) && (base1 < base2 + size2)) ||
		   (( base2 >= base1 ) && (base2 < base1 + size1));
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
physaddr_t get_kernel_dir(void)
{
	return kernel_page_dir;
}

/*-----------------------------------------------------------------------------
 *		Allocate memory
 *---------------------------------------------------------------------------*/
void* kmalloc_common(size_t size, bool align, physaddr_t* phys_addr)
{
	void*	vaddr = kheap.start;
	int		i = 0;
	bool	overlap = false;

	/*mutex_get(&kheap.heap_mutex, true);*/
	/*lock(get_current_thread()->id);*/

	/* Check overlapped blocks */
	do
	{
		overlap = false;

		if (align)
		{
			u32int tmp_addr = (u32int) vaddr;

			if (tmp_addr & 0xFFF)
			{
				tmp_addr &= 0xFFFFF000;
				tmp_addr += PAGE_SIZE;

				vaddr = (void*) tmp_addr;
			}
		}


		for (i = kheap.count - 1; i >= 0 ; i--)
		{
			if (is_blocks_overlapped(kheap.blocks[i].base,
									 kheap.blocks[i].size,
									 vaddr,
									 size))
			{
				vaddr = kheap.blocks[i].base + kheap.blocks[i].size;
				overlap = true;
			}
		}
	} while (overlap);

	if (phys_addr)
	{
		physaddr_t tmp_phys_addr = get_page_info(kernel_page_dir, vaddr);
		tmp_phys_addr &= ~PAGE_OFFSET_MASK;

		*phys_addr = tmp_phys_addr;
	}

	for (i = kheap.count - 1; i >= 0; i--)
	{
		kheap.blocks[i+1].base = kheap.blocks[i].base;
		kheap.blocks[i+1].size = kheap.blocks[i].size;
	}

	kheap.count++;
	kheap.blocks[0].base = vaddr;
	kheap.blocks[0].size = size;

	kheap.size -= size;

	/*mutex_release(&kheap.heap_mutex);*/
	/*unlock(get_current_thread()->id);*/

	return vaddr;
}

/*-----------------------------------------------------------------------------
 *		Allocate memory
 *---------------------------------------------------------------------------*/
void* kmalloc(size_t size)
{
	return kmalloc_common(size, false, NULL);
}

/*-----------------------------------------------------------------------------
 *		Allocate memory
 *---------------------------------------------------------------------------*/
void* kmalloc_a(size_t size)
{
	return kmalloc_common(size, true, NULL);
}

/*-----------------------------------------------------------------------------
 *		Allocate memory
 *---------------------------------------------------------------------------*/
void* kmalloc_p(size_t size, physaddr_t* phys_addr)
{
	return kmalloc_common(size, false, phys_addr);
}

/*-----------------------------------------------------------------------------
 *		Allocate memory
 *---------------------------------------------------------------------------*/
void* kmalloc_ap(size_t size, physaddr_t* phys_addr)
{
	return kmalloc_common(size, true, phys_addr);
}

/*-----------------------------------------------------------------------------
 *		Free memory
 *---------------------------------------------------------------------------*/
void kfree(void* vaddr)
{
	int		i = 0;
	int		block_idx = 0;
	size_t	sz = 0;

	/*mutex_get(&kheap.heap_mutex, true);*/
	/*lock(get_current_thread()->id);*/

	/* Return in invalid pointer case */
	if (vaddr == NULL)
	{
		return;
	}

	/* Find block info by virtual address */
	for (i = 0; i < kheap.count; i++)
	{
		if (vaddr == kheap.blocks[i].base)
		{
			block_idx = i;
			sz = kheap.blocks[i].size;
			break;
		}
	}
	
	if (i == kheap.count)
	{
		return;
	}

	/* Shift down blocks info array */
	for (i = block_idx; i < kheap.count - 1; i++)
	{
		kheap.blocks[i].base = kheap.blocks[i+1].base;
		kheap.blocks[i].size = kheap.blocks[i+1].size;
	}

	/* Reduce number of allocated blocks */
	kheap.count--;

	u32int* tmp = (u32int*) &vaddr;

	*tmp = 0;

	kheap.size += sz;

	/*mutex_release(&kheap.heap_mutex);*/
	/*unlock(get_current_thread()->id);*/
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
physaddr_t clone_kernel_dir(u32int* vaddr)
{
	physaddr_t new_dir = 0;

	u32int* vnew_dir = (u32int*) kmalloc_ap(PAGE_SIZE, &new_dir);

	if (vnew_dir == NULL)
		return 0;

	memcpy(vnew_dir, (void*) kernel_page_dir, PAGE_SIZE);

	*vaddr = (u32int) vnew_dir;

	return new_dir;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
size_t get_kernel_heap_free(void)
{
	return kheap.size;
}
