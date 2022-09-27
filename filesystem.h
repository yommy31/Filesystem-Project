#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include "inode.h"
#include "SuperBlock.h"
#include "dentry.h"
#include "file.h"

#define MAX_PATH 1000
#define SYSTEM_SIZE (16*1024*1024)
#define HOME "Myfilesystem.os"


class Filesystem
{
public:
	void initialize();
	SUPER_BLOCK Super_Block;
	Inode root_inode;
	Inode cur_inode;
	char cur_path[MAX_PATH];
	//const char* root_path;
	//char cur_path[MAX_PATH];
	std::FILE* fp;

	void refresh();

	void WriteInode(Inode inode); // 
	void Block_Bitmap_Modify(int pos);
	void Inode_Bitmap_Modify(int pos);

	void WriteIntoSystem(Dentry item);//write directory
	void WriteIntoSystem(int id, int father_id,std::string filename,int filesize);//write file
	void DeleteFileFromInode(Inode inode);
	void DeleteDentryFromInode(Inode inode);
	void CopyFile(Inode file1_inode, int id, int father_id, std::string filename);

	void ReadFile(Inode inode);

	int FindAvailableInode();
	int FindAvailableAdddress();
	Dentry FindDentryFromInode(Inode inode);
	Inode FindInodeFromID(int id);

	Address int2address(int pos);
	int address2int(Address address);
	void addrcpy(Address& add1, Address add2);
	std::string get_time_string(time_t t);

	void WriteRandomStringIntoBlock(int pos);
	void CopyString(int block1,int block2);
	void ClearBlock(int pos);
	void ReadBlock(int pos);
	void WriteAddressIntoBlock(Address address,int pos,int addr);
	int GetAddressFromBlock(int pos, int addr);


	void createFile(std::string filename, int filesize);
	void deleteFile(std::string filename);
	void createDir(std::string dirname);
	void deleteDir(std::string dirname);
	void changeDir(std::string dirname);
	void dir();
	void cp(std::string filename1, std::string filename2);
	void sum();
	void cat(std::string filename);

	void welcome();
	void exit();


	Filesystem();
	~Filesystem();
};



#endif // !FILESYSTEM_H

