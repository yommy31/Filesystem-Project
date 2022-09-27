#ifndef DENTRY_H
#define DENTRY_H

#include <utility>
#include<string>
#include<array>
#include <iostream>

typedef std::pair<int, char[32]> item;

class Dentry
{
public:
	int inode_id;
	int father_id;
	char Dentryname[32];
	int file_num;
	std::array<item, 16> item_list;

	Dentry(int id, int father_id, std::string name);//if root,then father_id is -1
	Dentry();
	~Dentry();
};

#endif
