#include "dentry.h"
using namespace std;

Dentry::Dentry(int id, int fatherid,string name)
{
	if (name == "" && id != 0)
	{
		cout << "no name!" << endl;
		return;
	}
	if (name.length() > 64)
	{
		cout << "too long" << endl;
		return;
	}
	inode_id = id;
	strcpy_s(Dentryname, name.c_str());
	file_num = 1;

	//the file list should contain the dentry itself(".") first
	item_list[0].first = inode_id;
	strcpy_s(item_list[0].second, ".");

	//the father dentry("..") should also be contained
	if (id != 0)
	{
		file_num += 1;
		item_list[1].first = fatherid;
		strcpy_s(item_list[1].second, "..");
	}
	father_id = fatherid;
}

Dentry::Dentry()
{

}

Dentry::~Dentry()
{

}