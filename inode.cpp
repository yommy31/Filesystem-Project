#include "inode.h"
#include <cstring>

Inode::Inode()
{
	id = -1;
	father_id = -1;
	type = NONE_TYPE;
	ctime = 0;
	mtime = 0;
	memset(direct_address, 0, sizeof(direct_address));
	memset(indirect_address, 0, sizeof(indirect_address));
	file_size = 0;
}

Inode::~Inode()
{

}