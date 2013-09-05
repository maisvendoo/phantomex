/*-----------------------------------------------------------------------------
 *
 * 		System calls user library
 * 		(c) maisvendoo, 03.09.2013
 *
 *-----------------------------------------------------------------------------*/

#include	"system.h"

/*-----------------------------------------------------------------------------
 * 		System calls definition
 *---------------------------------------------------------------------------*/
DEFN_SYSCALL0(exit, SYSCALL_EXIT)
DEFN_SYSCALL0(thread_exit, SYSCALL_THREAD_EXIT)

DEFN_SYSCALL1(exec, SYSCALL_EXEC, char*)

DEFN_SYSCALL2(vprint, SYSCALL_VPRINT, vscreen_t*, char*)

DEFN_SYSCALL3(thread_create, SYSCALL_THREAD_CREATE, void*, size_t, bool)
