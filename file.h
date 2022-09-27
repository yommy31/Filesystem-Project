#ifndef FILE_H
#define FILE_H

#include "inode.h"

class File
{
public:
	File();
	~File();
	int inode_id;//default -1
	char filename[64];//max length of name is 20
private:

};

#endif // !FILE_H

