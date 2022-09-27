# Filesystem-Project
根据Ext2相关原理实现的简易文件系统，包含基本的文件组织实现、文件/目录的增删查改操作；
* 文件组织实现
   * 本地创建Myfilesystem.os文件；
   * superblock，超级块，包含文件系统信息；
   * inode，索引节点，与文件/目录一一对应，包含文件的基本信息；
   * dentry，目录，包含该目录下目录项的信息（inode id & 文件名）；
   * 以bitmap维护data block和inode id的占用情况；
   * 实现superblock-inodeBitmap-blockBitmap-inodetable-data的基本分区形式（这个的blockBitmap能维护的空间大小有限，较大的文件系统中应该以多块组的形式组织，每个块组中有自己的bitmap）；
   * 实现基本的非连续文件页组织形式，代码里一页为 1 KB，仅实现直接索引+ 一级间接索引（通常应该有三级），对应的最大文件大小为 351 KB；
* 文件增删查改
   * 支持相对路径绝对路径
   * dir：列举目录项；
   * sum：显示文件系统使用情况；
   * exit：退出；
   * createFile [filename] [size]: 创建文件并指定文件大小，作为demo不考虑文件类型，所以这里写入的是指定长度的随机字符串;
   * deleteFile [filename]：删除文件；
   * createDir [path]： 创建目录；
   * changeDir [path]： 相当于 cd；
   * cat [filename]：输出文件内容;
   * cp [file1] [file2]: 拷贝文件;

***本科os做的项目，仅作学习参考*** 
