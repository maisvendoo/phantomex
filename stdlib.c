/*-----------------------------------------------------------------------------
 *
 *			Standard library
 *			(c) maisvendoo, 31.08.2013
 *
 *---------------------------------------------------------------------------*/

#include	"stdlib.h"

heap_t*		user_heap = USER_HEAP_START; /* Pointer to user space user_heap */

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void memset(void* dest, u8int val, u32int len)
{
    u8int *temp = (u8int*)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void memcpy(void* dest, void* src, u32int size)
{
    u8int* dp = (u8int*) dest;
    u8int* sp = (u8int*) src;
    u32int i = 0;

    for (i = 0; i < size; i++)
    {
    	dp[i] = sp[i];
    }
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
 *		Allocate memory
 *---------------------------------------------------------------------------*/
void* malloc(size_t size)
{
	void*	vaddr = user_heap->start;
	int		i = 0;

	/*mutex_get(&user_heap->user_heap_mutex, true);*/

	/* Check overlapped blocks */
	for (i = user_heap->count - 1; i >= 0 ; i--)
	{
		if (is_blocks_overlapped(user_heap->blocks[i].base,
								 user_heap->blocks[i].size,
								 vaddr,
								 size))
		{
			vaddr = user_heap->blocks[i].base + user_heap->blocks[i].size;
		}
	}

	for (i = user_heap->count - 1; i >= 0; i--)
	{
		user_heap->blocks[i+1].base = user_heap->blocks[i].base;
		user_heap->blocks[i+1].size = user_heap->blocks[i].size;
	}

	user_heap->count++;
	user_heap->blocks[0].base = vaddr;
	user_heap->blocks[0].size = size;

	/*mutex_release(&user_heap->user_heap_mutex);*/

	return vaddr;
}

/*-----------------------------------------------------------------------------
 *		Allocate memory
 *---------------------------------------------------------------------------*/
void free(void* vaddr)
{
	int		i = 0;
	int		block_idx = 0;

	/*mutex_get(&user_heap->user_heap_mutex, true);*/

	/* Return in invalid pointer case */
	if (vaddr == NULL)
	{
		return;
	}

	/* Find block info by virtual address */
	for (i = 0; i < user_heap->count; i++)
	{
		if (vaddr == user_heap->blocks[i].base)
		{
			block_idx = i;
			break;
		}
	}

	if (i == user_heap->count)
	{
		return;
	}

	/* Shift down blocks info array */
	for (i = block_idx; i < user_heap->count - 1; i++)
	{
		user_heap->blocks[i].base = user_heap->blocks[i+1].base;
		user_heap->blocks[i].size = user_heap->blocks[i+1].size;
	}

	/* Reduce number of allocated blocks */
	user_heap->count--;

	/*mutex_release(&user_heap->user_heap_mutex);*/
}
