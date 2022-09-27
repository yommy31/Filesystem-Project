#include "filesystem.h"
using namespace std;
#pragma warning(disable:4996)

char blank[SYSTEM_SIZE];



Filesystem::Filesystem()
{
	initialize();
	welcome();
}

Filesystem::~Filesystem()
{
	fclose(fp);
}

void Filesystem::initialize()
{
	ifstream file;
	file.open(HOME, ios::in);
	if (!file)
	{
		//Get Basic Infomatioin
		Super_Block.initialize();

		//initialize file system space
		cout << "Creating filesystem file: ./Myfilesystem.os..." << endl;
		fp = fopen(HOME, "wb+");
		//memset(fill, 0, sizeof(fill));
		if (fp == NULL) {
			cout << "error" << endl;
			return;
		}
		fwrite(blank, Super_Block.System_Size, 1, fp);

		//initialize super block space
		cout << "Creating super block..." << endl;
		fseek(fp, 0, SEEK_SET);
		fwrite(&Super_Block, sizeof(SUPER_BLOCK), 1, fp);

		//initialize inode bitmap
		cout << "Creating inode bitmap..." << endl;
		fseek(fp, Super_Block.Inode_Bitmap_Start, SEEK_SET);
		for (int i = 0; i < Super_Block.Inode_Bitmap_Size; i++) {
			unsigned char byte = 0;
			fwrite(&byte, sizeof(unsigned char), 1, fp);
		}

		
		//initialize block bitmap
		cout << "Creating block bitmap..." << endl;
		fseek(fp, Super_Block.Block_Bitmap_Start, SEEK_SET);
		for (int i = 0; i < Super_Block.Block_Bitmap_Size; i++) {
			unsigned char byte = 0;
			fwrite(&byte, sizeof(unsigned char), 1, fp);
		}


		//here make a change to block bitmap,because the former thing takes blocks
		for (int i = 0; i < Super_Block.Data_Block_Start / 1024; i++)
		{
			Block_Bitmap_Modify(i);
		}



		//initialize the inode table 
		for (int i = 0; i < Super_Block.Inode_Number; i++)
		{
			Inode temp;
			temp.id = i;
			fwrite(&temp, sizeof(temp), 1, fp);
		}

		//initialize the root directory
		cout << "Initialize root directory" << endl;
		int root_id = FindAvailableInode();//first time should be 0
		Dentry root(root_id, -1, "~");//-1 means no fathe inode
		WriteIntoSystem(root);

		//root_inode & current inode &cur_path
		fseek(fp, Super_Block.Inode_Table_Start, SEEK_SET);
		fread(&root_inode, sizeof(Inode), 1, fp);


		cur_inode = root_inode;
		strcpy_s(cur_path, "~");


		//waiting



	}
	else
	{
		//get info from existing system
		cout << "Reloading system..." << endl;
		fp = fopen(HOME, "rb+");
		//get super block
		fseek(fp, 0, SEEK_SET);
		fread(&Super_Block, sizeof(SUPER_BLOCK), 1, fp);
		//root_inode & current inode
		fseek(fp, Super_Block.Inode_Table_Start, SEEK_SET);
		fread(&root_inode, sizeof(Inode), 1, fp);
		cur_inode = root_inode;
		strcpy_s(cur_path, "~");

		//Inode_Bitmap_Modify(0);
		//Inode_Bitmap_Modify(0);
		
		//int a = FindAvailableInode();
		//int b = FindAvailableAdddress();
		//cout << endl;

		//waiting



		
	}
	//root_path = "/";
	//strcpy_s(cur_path, root_path);
}

void Filesystem::refresh()
{
	//refresh inode info
	int new_root, new_cur;
	new_root = root_inode.id;
	new_cur = cur_inode.id;
	root_inode = FindInodeFromID(new_root);
	cur_inode = FindInodeFromID(new_cur);

	//get current path from cur_inode
	string temp="";
	int node = cur_inode.id;
	while (node != -1)
	{
		Inode a = FindInodeFromID(node);
		Dentry b = FindDentryFromInode(a);
		temp = b.Dentryname + temp;
		if (node != 0)
			temp = "/" + temp;
		node = a.father_id;
	}
	if (temp.length() > Super_Block.Max_Path_Length)
		cout << "Too long path" << endl;
	strcpy_s(cur_path, temp.c_str());
}

void Filesystem::Block_Bitmap_Modify(int pos)
{
	int byte = pos / 8;
	int offset = pos % 8;
	unsigned char change;
	fseek(fp, Super_Block.Block_Bitmap_Start+byte, SEEK_SET);
	fread(&change, sizeof(unsigned char), 1, fp);
	change = change ^ (1 << (7 - offset));
	fseek(fp, Super_Block.Block_Bitmap_Start+byte, SEEK_SET);
	fwrite(&change, sizeof(unsigned char), 1, fp);
}

void Filesystem::Inode_Bitmap_Modify(int pos)
{
	int byte = pos / 8;
	int offset = pos % 8;
	unsigned char change;
	fseek(fp, Super_Block.Inode_Bitmap_Start+byte, SEEK_SET);
	fread(&change, sizeof(unsigned char), 1, fp);
	change = change ^ (1 << (7 - offset));
	fseek(fp, Super_Block.Inode_Bitmap_Start+byte, SEEK_SET);
	fwrite(&change, sizeof(unsigned char), 1, fp);
}

void Filesystem::WriteIntoSystem(Dentry item)//id should be an empty inode
{
	//write self inode iformation,inode bitmap,block bitmap
	Inode temp;
	temp.id = item.inode_id;
	temp.father_id = item.father_id;
	temp.type = DIRECTORY_TYPE;
	temp.ctime = time(0);
	temp.mtime = time(0);
	temp.file_size = 1;//sizeof(Dentry)=616B
	Inode_Bitmap_Modify(temp.id);//inode_bimap
	int pos = FindAvailableAdddress();
	Block_Bitmap_Modify(pos);//block bitmap
	Address address = int2address(pos);
	addrcpy(temp.direct_address[0], address);
	WriteInode(temp);//write into table


	//write block(including address of inode)
	fseek(fp, pos * Super_Block.Block_Size, SEEK_SET);
	fwrite(&item, sizeof(item), 1, fp);//dentry is always smaller than 1 KB

	SUPER_BLOCK a;
	fseek(fp, 0, SEEK_SET);
	fread(&a, sizeof(SUPER_BLOCK), 1, fp);
	cout << endl;

	//modify super block
	Super_Block.Used_Block_Number += 1;
	Super_Block.Unused_Block_Number -= 1;
	Super_Block.Used_Space += 1;
	Super_Block.Unused_Space -= 1;
	Super_Block.Used_Inode += 1;
	Super_Block.Unused_Inode -= 1;
	//Super_Block.File_Number += 1;
	fseek(fp, Super_Block.Super_Block_Start, SEEK_SET);
	fwrite(&Super_Block, sizeof(SUPER_BLOCK), 1, fp);

	//modify father inode backward
	int father = temp.father_id;
	while (father != -1)
	{
		//modify father inode
		Inode father_temp = FindInodeFromID(father);
		father_temp.file_size += 1;
		//modify father block
		if (father == temp.father_id)
		{
			father_temp.mtime = time(0);
			Dentry a = FindDentryFromInode(father_temp);
			int father_pos = address2int(father_temp.direct_address[0]);
			a.file_num += 1;
			a.item_list[a.file_num - 1].first = item.inode_id;
			strcpy(a.item_list[a.file_num - 1].second , item.Dentryname);
			fseek(fp, father_pos * Super_Block.Block_Size, SEEK_SET);
			fwrite(&a, sizeof(Dentry), 1, fp);
		}
		WriteInode(father_temp);
		father = father_temp.father_id;
	}
	

	refresh();

}

void Filesystem::WriteIntoSystem(int id, int father_id, std::string filename, int filesize)
{
	//write self inode iformation,inode bitmap,block bitmap
	Inode temp;
	temp.id = id;
	temp.father_id = father_id;
	temp.type = FILE_TYPE;
	temp.ctime = time(0);
	temp.mtime = time(0);
	temp.file_size = filesize;
	Inode_Bitmap_Modify(temp.id);//inode_bimap
	//arrange address
	int blocks = temp.file_size;
	if (blocks > 10)temp.file_size += 1;
	//write file here
	bool flag = false;
	for (int i = 0; i < blocks; i++)
	{
		if (i < 10)
		{
			int pos = FindAvailableAdddress();
			Block_Bitmap_Modify(pos);//block_bitmap
			Address address = int2address(pos);
			addrcpy(temp.direct_address[i], address);
			WriteRandomStringIntoBlock(pos);//write into block
		}
		else
		{
			if (!flag)
			{
				int indir_addr = FindAvailableAdddress();
				Block_Bitmap_Modify(indir_addr);//block_bitmap
				Address address = int2address(indir_addr);
				addrcpy(temp.indirect_address[0], address);
				flag = true;
			}
			int pos = FindAvailableAdddress();
			Block_Bitmap_Modify(pos);//block_bitmap
			Address address = int2address(pos);
			//addrcpy(temp.direct_address[i], address);
			WriteAddressIntoBlock(address, i-10,address2int(temp.indirect_address[0]));//write to address block
			WriteRandomStringIntoBlock(pos);//write into block
		}
	}
	WriteInode(temp);


	//modify super block
	Super_Block.Used_Block_Number += temp.file_size;
	Super_Block.Unused_Block_Number -= temp.file_size;
	Super_Block.Used_Space += temp.file_size;
	Super_Block.Unused_Space -= temp.file_size;
	Super_Block.Used_Inode += 1;
	Super_Block.Unused_Inode -= 1;
	Super_Block.File_Number += 1;
	fseek(fp, Super_Block.Super_Block_Start, SEEK_SET);
	fwrite(&Super_Block, sizeof(SUPER_BLOCK), 1, fp);


	//modify father inode backward
	int father = temp.father_id;
	while (father != -1)
	{
		//modify father inode
		Inode father_temp = FindInodeFromID(father);
		father_temp.file_size += temp.file_size;
		//modify father block
		if (father == temp.father_id)
		{
			father_temp.mtime = time(0);
			Dentry a = FindDentryFromInode(father_temp);
			a.file_num += 1;
			a.item_list[a.file_num - 1].first = temp.id;
			strcpy(a.item_list[a.file_num - 1].second, filename.c_str());
			int pos = address2int(father_temp.direct_address[0]);
			fseek(fp, pos * Super_Block.Block_Size, SEEK_SET);
			fwrite(&a, sizeof(Dentry), 1, fp);
		}
		WriteInode(father_temp);
		father = father_temp.father_id;
	}

	refresh();//system refresh
}

void Filesystem::WriteInode(Inode inode)
{
	int id = inode.id;
	fseek(fp, Super_Block.Inode_Table_Start+id*sizeof(Inode), SEEK_SET);
	fwrite(&inode, sizeof(Inode), 1, fp);
}

void Filesystem::DeleteFileFromInode(Inode inode)
{
	//get nessecery info
	int id = inode.id;
	int father_id = inode.father_id;
	Inode father_inode = FindInodeFromID(father_id);
	Dentry father_dentry = FindDentryFromInode(father_inode);

	//delete block field
	int blocks = inode.file_size;
	bool flag = false;
	for (int i = 0; i < blocks; i++)
	{
		if (i < 10)
		{
			int pos = address2int(inode.direct_address[i]);
			Block_Bitmap_Modify(pos);//block_bitmap
			ClearBlock(pos);
		}
	}
	if (blocks > 10)
	{
		blocks -= 1;//larger than 10 blocks means that a block for the indirect address exists
		int indir_addr_num = blocks - 10;//number of indirect address
		int indir_addr = address2int(inode.indirect_address[0]);
		for (int i = 0; i < indir_addr_num; i++)
		{
			int pos = GetAddressFromBlock(i, indir_addr);//get additional addresses from indirect address
			Block_Bitmap_Modify(pos);//block_bitmap
			ClearBlock(pos);//clear file content
		}
		Block_Bitmap_Modify(indir_addr);//block_bitmap fot indirect address
		ClearBlock(indir_addr);//clear block for indirect address
	}
	
	//modify super_block
	Super_Block.Used_Block_Number -= inode.file_size;
	Super_Block.Unused_Block_Number += inode.file_size;
	Super_Block.Used_Space -= inode.file_size;
	Super_Block.Unused_Space += inode.file_size;
	Super_Block.Used_Inode -= 1;
	Super_Block.Unused_Inode += 1;
	Super_Block.File_Number -= 1;
	fseek(fp, Super_Block.Super_Block_Start, SEEK_SET);
	fwrite(&Super_Block, sizeof(SUPER_BLOCK), 1, fp);


	//modify father inode and dentry backward
	int father = inode.father_id;
	while (father != -1)
	{
		//modify father inode
		Inode father_temp = FindInodeFromID(father);
		father_temp.file_size -= inode.file_size;
		//modify father block
		if (father == inode.father_id)
		{
			father_temp.mtime = time(0);//modify mtime
			Dentry a = FindDentryFromInode(father_temp);//get father dentry
			a.file_num -= 1;
			//iteratly change the dentry item
			int rec = 0;
			for (int i = 0; i <= a.file_num; i++)
			{
				if (a.item_list[i].first == inode.id)
				{
					rec = i;//record the pos
					for (int j = rec; j <= a.file_num; j++)
					{
						if (j == a.file_num)
						{
							a.item_list[j].first = -1;
							strcpy_s(a.item_list[j].second, "");
							break;
						}
						a.item_list[j].first = a.item_list[j + 1].first;
						strcpy_s(a.item_list[j].second, a.item_list[j + 1].second);
					}
					break;
				}
			}
			int pos = address2int(father_temp.direct_address[0]);
			fseek(fp, pos * Super_Block.Block_Size, SEEK_SET);
			fwrite(&a, sizeof(Dentry), 1, fp);
		}
		WriteInode(father_temp);
		father = father_temp.father_id;
	}

	//clear the inode itself
	Inode temp;
	temp.id = inode.id;
	WriteInode(temp);//inode table 
	Inode_Bitmap_Modify(temp.id);//inode bitmap


	refresh();//system refresh
	return;

}

void Filesystem::DeleteDentryFromInode(Inode inode)
{
	//get nessecery info
	int id = inode.id;
	int father_id = inode.father_id;
	Inode father_inode = FindInodeFromID(father_id);
	Dentry father_dentry = FindDentryFromInode(father_inode);
	Dentry target_dentry = FindDentryFromInode(inode);

	//recursively delete files
	for (int i = 0; i < target_dentry.file_num; i++)
	{
		int id_to_delete = target_dentry.item_list[i].first;
		if (id_to_delete != id && id_to_delete != father_id)
		{
			Inode a = FindInodeFromID(id_to_delete);
			if (a.type == FILE_TYPE)
				DeleteFileFromInode(a);
			else
				DeleteDentryFromInode(a);
		}
	}

	//refresh inode
	inode = FindInodeFromID(inode.id);

	//clear block field
	int pos = address2int(inode.direct_address[0]);
	Block_Bitmap_Modify(pos);
	ClearBlock(pos);

	//modify super block
	Super_Block.Used_Block_Number -= 1;
	Super_Block.Unused_Block_Number += 1;
	Super_Block.Used_Space -= 1;
	Super_Block.Unused_Space += 1;
	Super_Block.Used_Inode -= 1;
	Super_Block.Unused_Inode += 1;
	//Super_Block.File_Number -= 1;
	fseek(fp, Super_Block.Super_Block_Start, SEEK_SET);
	fwrite(&Super_Block, sizeof(SUPER_BLOCK), 1, fp);


	//modify father inode and dentry backward
	int father = inode.father_id;
	while (father != -1)
	{
		//modify father inode
		Inode father_temp = FindInodeFromID(father);
		father_temp.file_size -= inode.file_size;
		//modify father block
		if (father == inode.father_id)
		{
			father_temp.mtime = time(0);//modify mtime
			Dentry a = FindDentryFromInode(father_temp);//get father dentry
			a.file_num -= 1;
			//iteratly change the dentry item
			int rec = 0;
			for (int i = 0; i <= a.file_num; i++)
			{
				if (a.item_list[i].first == inode.id)
				{
					rec = i;//record the pos
					for (int j = rec; j <= a.file_num; j++)
					{
						if (j == a.file_num)
						{
							a.item_list[j].first = -1;
							strcpy_s(a.item_list[j].second, "");
							break;
						}
						a.item_list[j].first = a.item_list[j + 1].first;
						strcpy_s(a.item_list[j].second, a.item_list[j + 1].second);
					}
					break;
				}
			}
			int pos = address2int(father_temp.direct_address[0]);
			fseek(fp, pos * Super_Block.Block_Size, SEEK_SET);
			fwrite(&a, sizeof(Dentry), 1, fp);
		}
		WriteInode(father_temp);
		father = father_temp.father_id;
	}


	//clear the inode itself
	Inode temp;
	temp.id = inode.id;
	WriteInode(temp);//inode table 
	Inode_Bitmap_Modify(temp.id);//inode bitmap


	refresh();//system refresh
	return;

}

void Filesystem::CopyFile(Inode file1_inode, int id, int father_id, std::string filename)
{
	//get filesize
	int filesize = file1_inode.file_size;

	//write self inode iformation,inode bitmap,block bitmap
	Inode temp;
	temp.id = id;
	temp.father_id = father_id;
	temp.type = FILE_TYPE;
	temp.ctime = time(0);
	temp.mtime = time(0);
	temp.file_size = filesize;
	Inode_Bitmap_Modify(temp.id);//inode_bimap
	//arrange address
	int blocks = temp.file_size;
	if (blocks > 10)blocks -= 1;
	//write file here
	bool flag = false;
	for (int i = 0; i < blocks; i++)
	{
		if (i < 10)
		{
			int pos = FindAvailableAdddress();
			Block_Bitmap_Modify(pos);//block_bitmap
			Address address = int2address(pos);
			addrcpy(temp.direct_address[i], address);

			int origin_pos = address2int(file1_inode.direct_address[i]);

			CopyString(origin_pos,pos);//copy block



		}
		else
		{
			if (!flag)
			{
				int indir_addr = FindAvailableAdddress();
				Block_Bitmap_Modify(indir_addr);//block_bitmap
				Address address = int2address(indir_addr);
				addrcpy(temp.indirect_address[0], address);
				flag = true;
			}
			int pos = FindAvailableAdddress();
			Block_Bitmap_Modify(pos);//block_bitmap
			Address address = int2address(pos);
			//addrcpy(temp.direct_address[i], address);
			WriteAddressIntoBlock(address, i - 10, address2int(temp.indirect_address[0]));//write to address block
			int indirect_addr_origin = address2int(file1_inode.indirect_address[0]);

			int origin_pos = GetAddressFromBlock(i - 10, indirect_addr_origin);

			CopyString(origin_pos, pos);//copy block

		}
	}
	WriteInode(temp);


	//modify super block
	Super_Block.Used_Block_Number += temp.file_size;
	Super_Block.Unused_Block_Number -= temp.file_size;
	Super_Block.Used_Space += temp.file_size;
	Super_Block.Unused_Space -= temp.file_size;
	Super_Block.Used_Inode += 1;
	Super_Block.Unused_Inode -= 1;
	Super_Block.File_Number += 1;
	fseek(fp, Super_Block.Super_Block_Start, SEEK_SET);
	fwrite(&Super_Block, sizeof(SUPER_BLOCK), 1, fp);


	//modify father inode backward
	int father = temp.father_id;
	while (father != -1)
	{
		//modify father inode
		Inode father_temp = FindInodeFromID(father);
		father_temp.file_size += temp.file_size;
		//modify father block
		if (father == temp.father_id)
		{
			father_temp.mtime = time(0);
			Dentry a = FindDentryFromInode(father_temp);
			a.file_num += 1;
			a.item_list[a.file_num - 1].first = temp.id;
			strcpy(a.item_list[a.file_num - 1].second, filename.c_str());
			int pos = address2int(father_temp.direct_address[0]);
			fseek(fp, pos * Super_Block.Block_Size, SEEK_SET);
			fwrite(&a, sizeof(Dentry), 1, fp);
		}
		WriteInode(father_temp);
		father = father_temp.father_id;
	}

	refresh();//system refresh
}

void Filesystem::ReadFile(Inode inode)
{
	//read block field
	int blocks = inode.file_size;
	bool flag = false;
	for (int i = 0; i < blocks; i++)
	{
		if (i < 10)
		{
			int pos = address2int(inode.direct_address[i]);
			ReadBlock(pos);
		}
	}
	if (blocks > 10)
	{
		blocks -= 1;//larger than 10 blocks means that a block for the indirect address exists
		int indir_addr_num = blocks - 10;//number of indirect address
		int indir_addr = address2int(inode.indirect_address[0]);
		for (int i = 0; i < indir_addr_num; i++)
		{
			int pos = GetAddressFromBlock(i, indir_addr);//get additional addresses from indirect address
			ReadBlock(pos);//clear file content
		}
	}
}

int Filesystem::FindAvailableInode()
{
	fseek(fp, Super_Block.Inode_Bitmap_Start, SEEK_SET);
	int pos = -1;
	for (int i = 0; i < Super_Block.Inode_Bitmap_Size;i++ )
	{
		unsigned char byte;
		fread(&byte, sizeof(unsigned char), 1, fp);
		for (int j = 0; j < 8; j++) 
		{
			if (((byte >> (7-j)) & 1) == 0) 
			{
				pos = i * 8 + j;
				break;
			}
		}
		if (pos != -1) break;
	}
	return pos;
	//fseek(fp, Super_Block.Inode_Bitmap_Start + pos * sizeof(Inode), SEEK_SET);
	//Inode temp;
	//fread(&temp, sizeof(Inode), 1, fp);
	//return temp;
}

int Filesystem::FindAvailableAdddress()
{
	fseek(fp, Super_Block.Block_Bitmap_Start, SEEK_SET);
	int pos = -1;
	for (int i = 0; i < Super_Block.Block_Bitmap_Size; ++i) {
		unsigned char byte;
		fread(&byte, sizeof(unsigned char), 1, fp);
		for (int j = 0; j < 8; ++j) {
			if (((byte >> (7-j)) & 1) == 0) {
				pos = i * 8 + j;
				break;
			}
		}
		if (pos != -1) break;
	}
	return pos;
}

Dentry Filesystem::FindDentryFromInode(Inode inode)
{
	if (inode.type == FILE_TYPE)
	{
		cout << "Need dentry inode but get file inode" << endl;
	}
	Dentry temp;
	int pos = address2int(inode.direct_address[0]);
	fseek(fp, pos * Super_Block.Block_Size, SEEK_SET);
	fread(&temp, sizeof(Dentry), 1, fp);
	return temp;
}

Inode Filesystem::FindInodeFromID(int id)
{
	Inode temp;
	fseek(fp, Super_Block.Inode_Table_Start + id * sizeof(Inode), SEEK_SET);
	fread(&temp, sizeof(Inode), 1, fp);
	return temp;
}

int Filesystem::address2int(Address address)
{
	int pos=0;
	for (int i = 0; i < 3; i++)
	{
		unsigned char temp = address.address_info[i];
		pos += ((int)temp) * pow(2, 8 * (2 - i));
	}
	//pos -= Super_Block.Data_Block_Start;
	return pos;
}

Address Filesystem::int2address(int pos)
{
	Address address;
	//pos += Super_Block.Data_Block_Start;
	int temp0 = pos / pow(2, 8 * 2);
	address.address_info[0] = (unsigned char)temp0;
	pos -= temp0 * pow(2, 8 * 2);
	int temp1 = pos / pow(2, 8);
	address.address_info[1] = (unsigned char)temp1;
	pos -= temp1 * pow(2, 8);
	int temp2 = pos;
	address.address_info[2] = (unsigned char)temp2;
	return address;
}

void Filesystem::addrcpy(Address& add1, Address add2)
{
	for (int i = 0; i < 3; i++)
	{
		add1.address_info[i] = add2.address_info[i];
	}
}

string Filesystem::get_time_string(time_t t)
{
	tm* ct = localtime(&t);
	string temp = "";
	temp += to_string(ct->tm_year + 1900);
	temp += "/";
	temp += to_string(ct->tm_mon + 1);
	temp += "/";
	temp += to_string(ct->tm_mday);
	temp += " ";
	temp += to_string(ct->tm_hour);
	temp += ":";
	temp += to_string(ct->tm_min);
	temp += ":";
	temp += to_string(ct->tm_sec);
	return temp;
}

void Filesystem::WriteRandomStringIntoBlock(int pos)
{
	srand(time(0));
	fseek(fp, pos * Super_Block.Block_Size, SEEK_SET);
	for (int i = 0; i < Super_Block.Block_Size; i++)
	{
		char a = (rand() % 26) + 'a';
		fwrite(&a, sizeof(char), 1, fp);
	}
}

void Filesystem::CopyString(int block1, int block2)
{
	//read
	char byte;
	fseek(fp, block1 * Super_Block.Block_Size, SEEK_SET);
	string temp = "";
	for (int i = 0; i < Super_Block.Block_Size; i++)
	{
		fread(&byte, sizeof(char), 1, fp);
		temp += byte;
	}

	//Then write
	fseek(fp, block2 * Super_Block.Block_Size, SEEK_SET);
	for (int i = 0; i < Super_Block.Block_Size; i++)
	{
		char a = temp[i];
		fwrite(&a, sizeof(char), 1, fp);
	}

}

void Filesystem::ClearBlock(int pos)
{
	unsigned char byte;
	fseek(fp, pos * Super_Block.Block_Size, SEEK_SET);
	for (int i = 0; i < Super_Block.Block_Size; i++)
	{
		fwrite(&byte, sizeof(char), 1, fp);
	}
}

void Filesystem::ReadBlock(int pos)
{
	char byte;
	fseek(fp, pos * Super_Block.Block_Size, SEEK_SET);
	string temp="";
	for (int i = 0; i < Super_Block.Block_Size; i++)
	{
		fread(&byte, sizeof(char), 1, fp);
		temp += byte;
	}
	cout << temp;
	return;
}

void Filesystem::WriteAddressIntoBlock(Address address,int pos,int addr)
{
	fseek(fp, addr * Super_Block.Block_Size+pos*sizeof(Address), SEEK_SET);
	fwrite(&address, sizeof(Address), 1, fp);
}

int Filesystem::GetAddressFromBlock(int pos, int addr)
{
	Address address;
	fseek(fp, addr * Super_Block.Block_Size + pos * sizeof(Address), SEEK_SET);
	fread(&address, sizeof(Address), 1, fp);
	return address2int(address);
}

void Filesystem::createFile(string filename, int filesize)
{
	//filesize check
	if (filesize > Super_Block.Max_File_Size)
	{
		cout << "Too large!" << endl;
		return;
	}

	//analysis filename
	int len = filename.size();
	Inode basic;
	//type of path
	if (filename[0] == '/') 
		basic = root_inode;
	else 
		basic = cur_inode;
	//get diretories on path
	vector<string> v;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (filename[i] == '/') {
			if (temp != "") {
				v.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += filename[i];
	}
	if (temp == "") {
		cout << "No file name" << endl;
		return;
	}
	if ((int)temp.size() >= Super_Block.Max_File_Name_Length)
	{
		cout << "Length exceed" << endl;
		return;
	}
	v.push_back(temp);

	//reach the directory that the file create
	int num = v.size();
	bool flag = false;
	if (num != 1)
	{
		for (int i = 0; i < num - 1; i++)
		{
			flag = false;
			Dentry temp = FindDentryFromInode(basic);
			for (int j = 0; j < temp.file_num; j++)
			{
				if (strcmp(temp.item_list[j].second, v[i].c_str()) == 0)
				{
					flag = true;
					basic = FindInodeFromID(temp.item_list[j].first);
					break;
				}
				if (flag)break;
			}
			if (!flag)
			{
				cout << "Wrong path" << endl;
				return;
			}
		}
	}

	//find if already exist
	Dentry temp2 = FindDentryFromInode(basic);
	bool flag2 = false;
	for (int i = 0; i < temp2.file_num; i++)
	{
		if (strcmp(temp2.item_list[i].second, v[num-1].c_str()) == 0)
		{
			flag2 = true;
		}
	}
	if (flag2)
	{
		cout << "Object of same name already exist." << endl;
		return;
	}
	if (temp2.file_num == 16)
	{
		cout << "The directory has reached the max file number." << endl;
		return;
	}


	//write file and update system
	int inode_id = FindAvailableInode();
	WriteIntoSystem(inode_id, basic.id, v[num-1], filesize);
}

void Filesystem::deleteFile(std::string filename)
{
	int len = filename.size();
	Inode basic;
	//type of path
	if (filename[0] == '/')
		basic = root_inode;
	else
		basic = cur_inode;
	//get diretories on path
	vector<string> v;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (filename[i] == '/') {
			if (temp != "") {
				v.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += filename[i];
	}
	if (temp == "") {
		cout << "No file name" << endl;
		return;
	}
	if ((int)temp.size() >= Super_Block.Max_File_Name_Length)
	{
		cout << "Length exceed" << endl;
		return;
	}
	v.push_back(temp);

	//reach the directory that the file exist
	int num = v.size();
	bool flag = false;
	if (num != 1)
	{
		for (int i = 0; i < num - 1; i++)
		{
			flag = false;
			Dentry temp = FindDentryFromInode(basic);
			for (int j = 0; j < temp.file_num; j++)
			{
				if (strcmp(temp.item_list[j].second, v[i].c_str()) == 0)
				{
					flag = true;
					basic = FindInodeFromID(temp.item_list[j].first);
					break;
				}
				if (flag)break;
			}
			if (!flag)
			{
				cout << "Wrong path" << endl;
				return;
			}
		}
	}

	//find if exist
	Dentry temp2 = FindDentryFromInode(basic);
	int rec = -1;
	for (int i = 0; i < temp2.file_num; i++)
	{
		if (strcmp(temp2.item_list[i].second, v[num - 1].c_str()) == 0)
		{
			rec = i;
		}
	}
	if (rec == -1)
	{
		cout << "File not exist" << endl;
		return;
	}

	//delete file and update system
	int id = temp2.item_list[rec].first;
	Inode inode = FindInodeFromID(id);
	if (inode.type != FILE_TYPE)
	{
		cout << "Not a file!" << endl;
		return;
	}
	DeleteFileFromInode(inode);
}

void Filesystem::createDir(std::string dirname)
{
	//analysis filename
	int len = dirname.size();
	Inode basic;
	//type of path
	if (dirname[0] == '/')
		basic = root_inode;
	else
		basic = cur_inode;
	//get diretories on path
	vector<string> v;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (dirname[i] == '/') {
			if (temp != "") {
				v.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += dirname[i];
	}
	if (temp == "") {
		cout << "Command is not correct" << endl;
		return;
	}
	if ((int)temp.size() >= Super_Block.Max_File_Name_Length)
	{
		cout << "Length exceed" << endl;
		return;
	}
	v.push_back(temp);

	//reach the directory that the file create
	int num = v.size();
	bool flag = false;
	if (num != 1)
	{
		for (int i = 0; i < num - 1; i++)
		{
			flag = false;
			Dentry temp = FindDentryFromInode(basic);
			for (int j = 0; j < temp.file_num; j++)
			{
				if (strcmp(temp.item_list[j].second, v[i].c_str()) == 0)
				{
					flag = true;
					basic = FindInodeFromID(temp.item_list[j].first);
					break;
				}
				if (flag)break;
			}
			if (!flag)
			{
				cout << "Wrong path" << endl;
				return;
			}
		}
	}

	//find if already exist
	Dentry temp2 = FindDentryFromInode(basic);
	bool flag2 = false;
	for (int i = 0; i < temp2.file_num; i++)
	{
		if (strcmp(temp2.item_list[i].second, v[num - 1].c_str()) == 0)
		{
			flag2 = true;
		}
	}
	if (flag2)
	{
		cout << "Object of same name already exist." << endl;
		return;
	}
	if (temp2.file_num == 16)
	{
		cout << "The current directory has reached the max file number." << endl;
		return;
	}


	//write file and update system
	int inode_id = FindAvailableInode();
	Dentry newDentry(inode_id,basic.id,v[num-1]);
	WriteIntoSystem(newDentry);
}

void Filesystem::deleteDir(std::string dirname)
{
	int len = dirname.size();
	Inode basic;
	//type of path
	if (dirname[0] == '/')
		basic = root_inode;
	else
		basic = cur_inode;
	//get diretories on path
	vector<string> v;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (dirname[i] == '/') {
			if (temp != "") {
				v.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += dirname[i];
	}
	if (temp == "") {
		cout << "No dentry name" << endl;
		return;
	}
	if ((int)temp.size() >= Super_Block.Max_File_Name_Length)
	{
		cout << "Length exceed" << endl;
		return;
	}
	v.push_back(temp);

	//reach the directory that the file exist
	int num = v.size();
	bool flag = false;
	if (num != 1)
	{
		for (int i = 0; i < num - 1; i++)
		{
			flag = false;
			Dentry temp = FindDentryFromInode(basic);
			for (int j = 0; j < temp.file_num; j++)
			{
				if (strcmp(temp.item_list[j].second, v[i].c_str()) == 0)
				{
					flag = true;
					basic = FindInodeFromID(temp.item_list[j].first);
					break;
				}
				if (flag)break;
			}
			if (!flag)
			{
				cout << "Wrong path" << endl;
				return;
			}
		}
	}

	//find if exist
	Dentry temp2 = FindDentryFromInode(basic);
	int rec = -1;
	for (int i = 0; i < temp2.file_num; i++)
	{
		if (strcmp(temp2.item_list[i].second, v[num - 1].c_str()) == 0)
		{
			rec = i;
		}
	}
	if (rec == -1)
	{
		cout << "Dentry not exist" << endl;
		return;
	}

	//check file type and whether it's root
	int id = temp2.item_list[rec].first;
	if (id == 0)
	{
		cout << "The root can not be deleted" << endl;
		return;
	}
	Inode inode = FindInodeFromID(id);
	if (inode.type != DIRECTORY_TYPE)
	{
		cout << "Not a directory" << endl;
		return;
	}

	//the target should not contain current directory
	Inode check_point = cur_inode;
	while (true)
	{
		if (check_point.id == inode.id)
		{
			cout << "This operation is not allowed cause current directory can not be deleted."<<endl;
			return;
		}
		if (check_point.father_id == -1)
			break;
		check_point = FindInodeFromID(check_point.father_id);
	}


	//delete file and update system
	DeleteDentryFromInode(inode);
}

void Filesystem::changeDir(std::string dirname)
{
	int len = dirname.size();
	Inode basic;
	//type of path
	if (dirname[0] == '/')
		basic = root_inode;
	else
		basic = cur_inode;
	//get diretories on path
	vector<string> v;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (dirname[i] == '/') {
			if (temp != "") {
				v.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += dirname[i];
	}
	if (temp == "") {
		cout << "No dentry name" << endl;
		return;
	}
	if ((int)temp.size() >= Super_Block.Max_File_Name_Length)
	{
		cout << "Length exceed" << endl;
		return;
	}
	v.push_back(temp);

	//reach the directory that the file exist
	int num = v.size();
	bool flag = false;
	if (num != 1)
	{
		for (int i = 0; i < num - 1; i++)
		{
			flag = false;
			Dentry temp = FindDentryFromInode(basic);
			for (int j = 0; j < temp.file_num; j++)
			{
				if (strcmp(temp.item_list[j].second, v[i].c_str()) == 0)
				{
					flag = true;
					basic = FindInodeFromID(temp.item_list[j].first);
					break;
				}
				if (flag)break;
			}
			if (!flag)
			{
				cout << "Wrong path" << endl;
				return;
			}
		}
	}

	//find if exist
	Dentry temp2 = FindDentryFromInode(basic);
	int rec = -1;
	for (int i = 0; i < temp2.file_num; i++)
	{
		if (strcmp(temp2.item_list[i].second, v[num - 1].c_str()) == 0)
		{
			rec = i;
		}
	}
	if (rec == -1)
	{
		cout << "Dentry not exist" << endl;
		return;
	}


	//check inode info
	int id = temp2.item_list[rec].first;
	Inode inode = FindInodeFromID(id);
	if (inode.type != DIRECTORY_TYPE)
	{
		cout << "Not a directory" << endl;
		return;
	}

	cur_inode = inode;
	refresh();
}

void Filesystem::dir()
{
	Dentry cur_dir = FindDentryFromInode(cur_inode);
	int num = cur_dir.file_num;

	cout << setw(35) << "File_name" << setw(12) << "Type" << setw(25) << "ctime" << setw(25) << "mtime" << setw(10) << "Size" << endl;
	for (int i = 0; i < num; i++)
	{
		int sub_id = cur_dir.item_list[i].first;
		Inode sub_item = FindInodeFromID(sub_id);
		int a = sub_item.type;
		string file_name = cur_dir.item_list[i].second;
		string type = (a == FILE_TYPE ? "FILE" : "DIRECTORY");
		string ct = get_time_string(sub_item.ctime);
		string mt = get_time_string(sub_item.mtime);
		string size = to_string(sub_item.file_size) + " KB";
		cout << setw(35) << file_name << setw(12) << type << setw(25) << ct
			<< setw(25) << mt << setw(10) << size << endl;
	}
	cout << "\n\t Total:\t" << cur_dir.file_num << " objects\t" << cur_inode.file_size << " KB" << endl;
	return;
}

void Filesystem::cp(std::string filename1, std::string filename2)
{
	int len1 = filename1.size();
	int len2 = filename2.size();
	Inode basic1,basic2;
	/*******************************************************************************************/
	//type of path
	if (filename1[0] == '/')
		basic1 = root_inode;
	else
		basic1 = cur_inode;

	if (filename2[0] == '/')
		basic2 = root_inode;
	else
		basic2 = cur_inode;

	//get diretories on path
	vector<string> v1,v2;
	string temp1 = "",temp2 = "";

	for (int i = 0; i < len1; ++i) {
		if (filename1[i] == '/') {
			if (temp1 != "") {
				v1.push_back(temp1);
				temp1 = "";
			}
			continue;
		}
		temp1 += filename1[i];
	}
	if (temp1 == "") {
		cout << "The first path has no file name" << endl;
		return;
	}
	if ((int)temp1.size() >= Super_Block.Max_File_Name_Length)
	{
		cout << "Length of file1 exceed.(not exist)" << endl;
		return;
	}
	v1.push_back(temp1);

	for (int i = 0; i < len2; ++i) {
		if (filename2[i] == '/') {
			if (temp2 != "") {
				v2.push_back(temp2);
				temp2 = "";
			}
			continue;
		}
		temp2 += filename2[i];
	}
	if (temp2 == "") {
		cout << "The second path has no file name" << endl;
		return;
	}
	if ((int)temp2.size() >= Super_Block.Max_File_Name_Length)
	{
		cout << "Length of file2 exceed" << endl;
		return;
	}
	v2.push_back(temp2);

	/*******************************************************************************************/

	//reach the directory that the file exist
	int num1 = v1.size(),num2 = v2.size();
	bool flag1 = false, flag2 = false;
	if (num1 != 1)
	{
		for (int i = 0; i < num1 - 1; i++)
		{
			flag1 = false;
			Dentry temp1 = FindDentryFromInode(basic1);
			for (int j = 0; j < temp1.file_num; j++)
			{
				if (strcmp(temp1.item_list[j].second, v1[i].c_str()) == 0)
				{
					flag1 = true;
					basic1 = FindInodeFromID(temp1.item_list[j].first);
					break;
				}
				if (flag1)break;
			}
			if (!flag1)
			{
				cout << "The first path is wrong." << endl;
				return;
			}
		}
	}

	if (num2 != 1)
	{
		for (int i = 0; i < num2 - 1; i++)
		{
			flag2 = false;
			Dentry temp2 = FindDentryFromInode(basic2);
			for (int j = 0; j < temp2.file_num; j++)
			{
				if (strcmp(temp2.item_list[j].second, v2[i].c_str()) == 0)
				{
					flag2 = true;
					basic2 = FindInodeFromID(temp2.item_list[j].first);
					break;
				}
				if (flag2)break;
			}
			if (!flag2)
			{
				cout << "The second path is wrong." << endl;
				return;
			}
		}
	}

	/*******************************************************************************************/

	//find if exist
	Dentry temp21 = FindDentryFromInode(basic1);
	int rec1 = -1;

	for (int i = 0; i < temp21.file_num; i++)
	{
		if (strcmp(temp21.item_list[i].second, v1[num1 - 1].c_str()) == 0)
		{
			rec1 = i;
		}
	}
	if (rec1 == -1)
	{
		cout << "File1 not exist" << endl;
		return;
	}

	Dentry temp22 = FindDentryFromInode(basic2);
	bool flag22 = false;
	for (int i = 0; i < temp22.file_num; i++)
	{
		if (strcmp(temp22.item_list[i].second, v2[num2 - 1].c_str()) == 0)
		{
			flag22 = true;
		}
	}
	if (flag22)
	{
		cout << "Object of same name as file2 already exist." << endl;
		return;
	}
	if (temp22.file_num == 16)
	{
		cout << "The directory of path2 has reached the max file number." << endl;
		return;
	}

	/*******************************************************************************************/

	//check inode
	int id1 = temp21.item_list[rec1].first;
	Inode inode1 = FindInodeFromID(id1);
	if (inode1.type != FILE_TYPE)
	{
		cout << "Path1 is not a file." << endl;
		return;
	}

	/*******************************************************************************************/
	//copy file1 to file2
	int id2 = FindAvailableInode();
	CopyFile(inode1, id2, basic2.id, v2[num2 - 1]);
}

void Filesystem::sum()
{
	cout << "Here is the system info:" << endl;
	cout << "System_Size: " << Super_Block.System_Size << endl;
	cout << "Block_Size: " << Super_Block.Block_Size << endl;
	cout << "Block_Number: " << Super_Block.Block_Number << endl;
	cout << "Used_Block_Number: " << Super_Block.Used_Block_Number << endl;
	cout << "Used_Space: " << Super_Block.Used_Space << endl;
	cout << "Unused_Block_Number: " << Super_Block.Unused_Block_Number << endl;
	cout << "Unused_Space: " << Super_Block.Unused_Space << endl;
	cout << "Inode_Number: " << Super_Block.Inode_Number << endl;
	cout << "Used_Inode: " << Super_Block.Used_Inode << endl;
	cout << "Unused_Inode: " << Super_Block.Unused_Inode << endl;
	cout << "Total_File_Number: " << Super_Block.File_Number << endl;
}

void Filesystem::cat(std::string filename)
{
	int len = filename.size();
	Inode basic;
	//type of path
	if (filename[0] == '/')
		basic = root_inode;
	else
		basic = cur_inode;
	//get diretories on path
	vector<string> v;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (filename[i] == '/') {
			if (temp != "") {
				v.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += filename[i];
	}
	if (temp == "") {
		cout << "No file name" << endl;
		return;
	}
	if ((int)temp.size() >= Super_Block.Max_File_Name_Length)
	{
		cout << "Length exceed" << endl;
		return;
	}
	v.push_back(temp);

	//reach the directory that the file exist
	int num = v.size();
	bool flag = false;
	if (num != 1)
	{
		for (int i = 0; i < num - 1; i++)
		{
			flag = false;
			Dentry temp = FindDentryFromInode(basic);
			for (int j = 0; j < temp.file_num; j++)
			{
				if (strcmp(temp.item_list[j].second, v[i].c_str()) == 0)
				{
					flag = true;
					basic = FindInodeFromID(temp.item_list[j].first);
					break;
				}
				if (flag)break;
			}
			if (!flag)
			{
				cout << "Wrong path" << endl;
				return;
			}
		}
	}

	//find if exist
	Dentry temp2 = FindDentryFromInode(basic);
	int rec = -1;
	for (int i = 0; i < temp2.file_num; i++)
	{
		if (strcmp(temp2.item_list[i].second, v[num - 1].c_str()) == 0)
		{
			rec = i;
		}
	}
	if (rec == -1)
	{
		cout << "File not exist" << endl;
		return;
	}

	//get inode and check type
	int id = temp2.item_list[rec].first;
	Inode inode = FindInodeFromID(id);
	if (inode.type != FILE_TYPE)
	{
		cout << "Not a file!" << endl;
		return;
	}

	//output
	cout << v[num - 1] << ": " << endl;
	ReadFile(inode);
	cout << endl;
	return;
}

void Filesystem::welcome()
{
	cout << "Welcome to our file system.\n\ncopyright@Hongrui Wang, Zicong Wu, Xinjie Yao" << endl;
	cout << "\nStudentID:\nHongrui Wang: 20170600496\nZicong Wu:201730600502\nXinjie Yao:201730601028\n" << endl;
	return;
}

void Filesystem::exit()
{
	this->~Filesystem();
}


