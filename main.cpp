#include "filesystem.h"
using namespace std;
#pragma warning(disable:4996)

#include <typeinfo>
#include <vector>


int main()
{
	Filesystem filesystem;
	while (true)
	{
		cout << "WWY@os_lab:" << filesystem.cur_path << "$ ";
		//get input
		string input;
		getline(cin, input);

		//clip input into a vector
		int len = input.size();
		vector<string> command;
		string temp = "";
		for (int i = 0; i < len; i++) {
			if (input[i] == ' ') {
				if (temp != "") {
					command.push_back(temp);
					temp = "";
				}
				continue;
			}
			temp += input[i];
		}
		if(temp!="")
			command.push_back(temp);

		//compare and execute
		int size = command.size();
		if (size == 0)
		{
			cout << "no command" << endl;
			continue;
		}
		else if (size == 1)
		{
			if (command[0] == "dir")
			{
				filesystem.dir();
			}
			else if (command[0] == "sum")
			{
				filesystem.sum();
			}
			else if (command[0] == "exit" or command[0] == "bye")
			{
				cout << "Bye!" << endl;
				filesystem.exit();
				break;
			}
			else
			{
				cout << "please check your command" << endl;
			} 
		}
		else if (size == 2)
		{
			if (command[0] == "deleteFile")
			{
				filesystem.deleteFile(command[1]);
			}
			else if (command[0] == "createDir")
			{
				filesystem.createDir(command[1]);
			}
			else if (command[0] == "deleteDir")
			{
				filesystem.deleteDir(command[1]);
			}
			else if (command[0] == "changeDir")
			{
				filesystem.changeDir(command[1]);
			}
			else if (command[0] == "cat")
			{
				filesystem.cat(command[1]);
			}
			else
			{
				cout << "please check your command" << endl;
			}
		}
		else if (size == 3)
		{
			if (command[0] == "createFile")
			{
				filesystem.createFile(command[1], stoi(command[2]));
			}
			else if (command[0] == "cp")
			{
				filesystem.cp(command[1], command[2]);
			}
			else
			{
				cout << "please check your command" << endl;
			}
		}


	}
	system("pause");
	return 0;
}