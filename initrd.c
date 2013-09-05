/*-----------------------------------------------------------------------------
 *
 * 		Initialization RAM FS driver
 * 		(c) maisvendoo, 29.08.2013
 *
 *---------------------------------------------------------------------------*/

#include	"initrd.h"

vfs_node_t*				initrd_root = 0;		/* Root directory */
vfs_node_t*				root_nodes = 0;			/* Nodes in root directory */

initrd_file_header_t*	file_alloc_table = 0;

u32int read_rd(vfs_node_t* node, u32int offset, u32int size, u8int* buffer);
u32int write_rd(vfs_node_t* node, u32int offset, u32int size, u8int* buffer);

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
vfs_node_t* init_ramdisk(u32int location)
{
	int	num_files = 0;
	int	i = 0;

	/* Read files number */
	num_files = *(int*)(location);

	/* Read files allocation table */
	file_alloc_table = (initrd_file_header_t*) (location + sizeof(int));

	/* Root file system init */
	initrd_root = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));

	strcpy(initrd_root->name, "initrd");

	initrd_root->other_x = 0;
	initrd_root->other_w = 1;
	initrd_root->other_r = 1;

	initrd_root->reserved = 0;

	initrd_root->uid = 0;
	initrd_root->gid = 0;
	initrd_root->inode = 0;
	initrd_root->size = 0;

	initrd_root->flags = VFS_DIRECTORY;
	initrd_root->read_vfs = 0;
	initrd_root->write_vfs = 0;
	initrd_root->open_vfs = 0;
	initrd_root->close_vfs = 0;

	initrd_root->seek = 0;

	root_nodes = (vfs_node_t*) kmalloc(sizeof(vfs_node_t)*num_files);

	/* Check all files in root directory */
	for (i = 0; i < num_files; i++)
	{
		file_alloc_table[i].offset += location;

		strcpy(root_nodes[i].name, file_alloc_table[i].name);

		root_nodes[i].other_x = 0;
		root_nodes[i].other_w = 0;
		root_nodes[i].other_r = 1;

		root_nodes[i].reserved = 0;

		root_nodes[i].uid = 0;
		root_nodes[i].gid = 0;
		root_nodes[i].size = file_alloc_table[i].size;
		root_nodes[i].inode = i;
		root_nodes[i].flags = VFS_FILE;
		root_nodes[i].ptr = NULL;

		root_nodes[i].read_vfs = (read_t) &read_rd;
		root_nodes[i].write_vfs = (write_t) &write_rd;
		root_nodes[i].open_vfs = 0;
		root_nodes[i].close_vfs = 0;

		root_nodes[i].seek = 0;
	}

	initrd_root->ptr = root_nodes;
	initrd_root->num_nodes = num_files;

	return initrd_root;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
u32int read_rd(vfs_node_t* node, u32int offset, u32int size, u8int* buffer)
{
	initrd_file_header_t file_header = file_alloc_table[node->inode];

	if (offset > file_header.size)
		return 0;

	if (offset + size > file_header.size)
		size = file_header.size - offset;

	memcpy(buffer, (u8int*)(file_header.offset + offset), size);

	return size;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
u32int write_rd(vfs_node_t* node, u32int offset, u32int size, u8int* buffer)
{
	return 0;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
vfs_node_t* get_root(void)
{
	return initrd_root;
}
