/*-----------------------------------------------------------------------------
 *
 * 		Standard I/O library
 * 		(c) maisvendoo, 29.08.2013
 *
 *---------------------------------------------------------------------------*/

#include	"stdio.h"

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
FILE* fopen(vfs_node_t* dir, char* file_name, char* mode)
{
	FILE* tmpf = find(dir, file_name);

	if (!tmpf)
		return NULL;

	/*if (mode[0] == 'w')
	{
		tmpf->all_w = 1;
		tmpf->all_r = 0;

		return tmpf;
	}

	if (mode[0] == 'r')
	{
		tmpf->all_w = 0;
		tmpf->all_r = 1;

		return tmpf;
	}*/


	return tmpf;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
FILE* find(vfs_node_t* dir, char* file_name)
{
	int	i = 0;
	int idx = -1;

	for (i = 0; i < dir->num_nodes; i++)
	{
		if (!strcmp(file_name, dir->ptr[i].name))
		{
			idx = i;
			break;
		}
	}

	if (idx == -1)
		return NULL;
	else
		return &dir->ptr[i];
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
size_t fread(FILE* file, void* buf, size_t size)
{
	size_t sz = read((vfs_node_t*) file, file->seek, size, (u8int*) buf);

	file->seek += sz;

	return sz;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void fseek(FILE* file, u32int seek_pos)
{
	file->seek = seek_pos;
}

