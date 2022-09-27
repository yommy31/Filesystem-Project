#include "SuperBlock.h"

SUPER_BLOCK::SUPER_BLOCK()
{

}

SUPER_BLOCK::~SUPER_BLOCK()
{

}
void SUPER_BLOCK::initialize()
{
	System_Size = 16 * 1024 * 1024;
	Block_Size = 1024;
	Block_Number = 16 * 1024;
	Used_Block_Number = 2 * 1024 + 5;
	Used_Space = 2 * 1024 + 5;
	Unused_Block_Number = 16 * 1024 - (2 * 1024 + 5);
	Unused_Space = 16 * 1024 - (2 * 1024 + 5);
	Inode_Number = 16 * 1024;
	Used_Inode = 0;
	File_Number = 0;
	Unused_Inode = 16 * 1024;
	Address_Length = 24;
	Max_File_Name_Length = 32;
	Max_Path_Length = 1000;
	Inode_Size = 128;//Unix Inode Size
	Super_Block_Size = 1024;//
	Inode_Bitmap_Size = 2 * 1024;//
	Block_Bitmap_Size = 2 * 1024;
	Inode_Table_Size = 2 * 1024 * 1024;
	Data_Block_Size = ((16 - 2) * 1024 - 5) * 1024;
	Super_Block_Start = 0;
	Inode_Bitmap_Start = 1024;
	Block_Bitmap_Start = 3 * 1024;
	Inode_Table_Start = 5 * 1024;
	Data_Block_Start = (2 * 1024 + 5) * 1024;
	Max_File_Size = 351;//351 blocks = 351 KB
}