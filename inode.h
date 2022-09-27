#ifndef INODE_H
#define INODE_H

#include <ctime>
#include "address.h"


#define	NONE_TYPE  -1
#define	DIRECTORY_TYPE  0
#define	FILE_TYPE  1


class Inode
{
public:
	Inode();
	~Inode();
	int id;
	int father_id;
	int type;
	time_t ctime;
	time_t mtime;
	Address direct_address[10];
	Address indirect_address[1];
	int file_size;

};



#endif
