#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

class SUPER_BLOCK
{
public:
	unsigned int System_Size;
	unsigned int Block_Size;
	unsigned int Block_Number;
	unsigned int Used_Block_Number;
	unsigned int Used_Space;
	unsigned int Unused_Block_Number;
	unsigned int Unused_Space;
	unsigned int Inode_Number;
	unsigned int Used_Inode;
	unsigned int File_Number;
	unsigned int Unused_Inode;
	unsigned int Address_Length;
	unsigned int Max_File_Name_Length;
	unsigned int Max_Path_Length;
	unsigned int Inode_Size;
	unsigned int Super_Block_Size;
	unsigned int Inode_Bitmap_Size;
	unsigned int Block_Bitmap_Size;
	unsigned int Inode_Table_Size;
	unsigned int Data_Block_Size;
	unsigned int Super_Block_Start;
	unsigned int Inode_Bitmap_Start;
	unsigned int Block_Bitmap_Start;
	unsigned int Inode_Table_Start;
	unsigned int Data_Block_Start;
	unsigned int Max_File_Size;

	SUPER_BLOCK();
	~SUPER_BLOCK();

	void initialize();
};

#endif
