#include <iostream>
#include<string>
#include <vector>
#include<windows.h>
#define BLOCK_SIZE 2048
#define BLOCK_NUM 1024
#define NO_TO_BE_USED 65532
#define MAX_LENGTH 100
#define FCB_SIZE 64
#define FREE 0
#define END -1

using namespace std;

struct FATB{
	string designer_name = "Li Longzhen";
	string version = "1.0";
	int root_loc;			//根目录位置
	int blocks_num;			//盘块总数
	int blocks_remain;		//剩余盘块数量
	string dirname_now;		//当前目录
};



 struct FCB{
	string file_name;		//文件名(10字节)
	string file_type;		//文件类型
	string text;			//文件内容
	bool read_only;			//是否只读
	bool hidden;			//是否隐藏
	bool directory;			//是否为目录
	int first_block;		//首块号
	int file_size;			//文件大小
	FCB* father_dir;		//父目录
	FCB* child_dir;			//子目录
	FCB* brother_file;		//同目录文件
};




FATB* block;
FCB* root = NULL;
int FAT[BLOCK_NUM];
FCB* dir_now;




void int_file_sys() {
	int i;
	block = new FATB;
	block->blocks_num = BLOCK_NUM;
	block->blocks_remain = BLOCK_NUM - 3;
	block->root_loc = 2;

	root = new FCB;
	root->file_name = "root";
	root->file_type= "dir";
	root->child_dir = NULL;
	root->father_dir = NULL;
	root->brother_file = NULL;
	root->directory = true;
	root->read_only = true;
	root->hidden = false;
	root->file_size = 0;
	root->first_block = 2;
	FAT[0] = NO_TO_BE_USED;
	FAT[1] = NO_TO_BE_USED;
	FAT[2] = END;
	for (i = 3; i < BLOCK_NUM; i++) {
		FAT[i] = FREE;
	}
	
	block->dirname_now = "/";
	dir_now = root;
}


int get_freeblock() {
	for (int i = 3; i < BLOCK_NUM; i++) {
		if (FAT[i] == FREE) {
			return i;
		}
	}
	return 0;
}



void new_block(FCB* path) {
	int num, new_number;
	new_number = get_freeblock();
	num = path->first_block;
	while (FAT[num != END]) {
		num = FAT[num];
	}
	FAT[num] = new_number;
	FAT[new_number] = END;
	block->blocks_remain--;
}



int block_isfull(FCB* dir) {
	int count = 0,num ;
	num = dir->first_block;
	while (num != END) {
		count++;
		num = FAT[num];
	}
	if ((dir->file_size) / BLOCK_SIZE > count) {
		return 1;
	}
	else{
		return 0;
	}
}



void split_path(string order,vector<string>&dir_name,string a) {
	string::size_type pos1, pos2;
	pos2 = order.find(a);
	pos1 = 0;
	while (string::npos != pos2) {
		dir_name.push_back(order.substr(pos1, pos2 - pos1));
		pos1 = pos2 + 1;
		pos2 = order.find(a, pos1);
	}
	dir_name.push_back(order.substr(pos1));
}

void md(FCB* fa_dir, string dirname) {
	vector<string> vs;
	split_path(dirname, vs,"/");
	if (vs.size() == 1) {

		FCB* new_dir;
		if (fa_dir == nullptr) {
			cout << "ERROR:directory does not exist\n" << endl;
			return;
		}
		new_dir = new FCB;
		new_dir->file_name = dirname.substr(0, 10);
		new_dir->file_size = 0;
		new_dir->text = "";
		new_dir->first_block = get_freeblock();
		new_dir->read_only = 0;
		new_dir->directory = 1;
		new_dir->child_dir = NULL;
		new_dir->father_dir = fa_dir;
		new_dir->brother_file = NULL;
		if (fa_dir->child_dir == NULL) {
			fa_dir->child_dir = new_dir;
		}
		else {
			FCB* a;
			a = fa_dir->child_dir;
			while (a->brother_file != nullptr) {
				a = a->brother_file;
			}
			a->brother_file = new_dir;
		}
		fa_dir->file_size += sizeof(FCB);
		if (block_isfull(fa_dir)) {
			new_block(fa_dir);
		}
		FAT[new_dir->first_block] = END;
		block->blocks_remain--;
		return;
	}
	else if (vs[0] == "..") {
		FCB* new_dir;
		new_dir = new FCB;
		new_dir->file_name = vs[vs.size()-1].substr(0, 10);
		new_dir->file_size = 0;
		new_dir->first_block = get_freeblock();
		new_dir->read_only = 0;
		new_dir->directory = 1;
		new_dir->child_dir = NULL;
		new_dir->brother_file = NULL;
		FCB* target_dir = fa_dir->child_dir;
		for (int i = 1; i < vs.size()-1; i++) {
			while (target_dir != nullptr) {
				if (target_dir->file_name == vs[vs.size() - 2]) {
					new_dir->father_dir = target_dir;
					if (target_dir->child_dir == NULL) {
						target_dir->child_dir = new_dir;
					}
					else {
						FCB* a;
						a = target_dir->child_dir;
						while (a->brother_file != nullptr) {
							a = a->brother_file;
						}
						a->brother_file = new_dir;
					}
					target_dir->file_size += sizeof(FCB);
					if (block_isfull(target_dir)) {
						new_block(target_dir);
					}
					FAT[new_dir->first_block] = END;
					block->blocks_remain--;
					return;

				}
				if (target_dir->file_name == vs[i]) {
					target_dir = target_dir->child_dir;
				}
				target_dir = target_dir->brother_file;
			}
		}
		cout << "相对路径不存在" << endl;
		return;
	}
	else {
		int sign;
		FCB* disp = root;
		for (int i = 1; i < vs.size()-1; i++) {
			disp = disp->child_dir;
			while (disp != nullptr) {
				if (disp->file_name == vs[i]) {
					sign = 1;
					break;
				}
				disp = disp->brother_file;
			}
			if (sign == 1) {
				sign = 0;
				continue;
			}
			else {
				cout << "目录中有目录不可用" << endl;
				return;
			}
			
		}
		FCB* new_dir;
		if (disp == nullptr) {
			cout << "ERROR:directory does not exist\n" << endl;
			return;
		}
		new_dir = new FCB;
		new_dir->file_name = vs[vs.size()-1].substr(0, 10);
		new_dir->file_size = 0;
		new_dir->first_block = get_freeblock();
		new_dir->read_only = 0;
		new_dir->directory = 1;
		new_dir->child_dir = NULL;
		new_dir->father_dir = disp;
		new_dir->brother_file = NULL;
		if (disp->child_dir == NULL) {
			disp->child_dir = new_dir;
		}
		else {
			FCB* a;
			a = disp->child_dir;
			while (a->brother_file != nullptr) {
				a = a->brother_file;
			}
			a->brother_file = new_dir;
		}
		disp->file_size += sizeof(FCB);
		if (block_isfull(disp)) {
			new_block(disp);
		}
		FAT[new_dir->first_block] = END;
		block->blocks_remain--;
		return;
	}

}

void dir(string order) {
	vector<string> vs;
	split_path(order, vs,"/");
	if (vs.size() == 1 || vs[0] == ".") {
		string target_dir;
		if (vs[0] == ".") {
			target_dir = vs[1];
		}
		else {
			target_dir = vs[0];
		}
		FCB* disp = dir_now->child_dir;
		while (disp != nullptr) {
			if (disp->file_name == target_dir) {
				disp = disp->child_dir;
				if (disp == nullptr) {
					cout << "当前目录下无子目录" << endl;
					return;
				}
				else
				{
					cout << "." << "<dir>" << endl;
					cout << ".." << "<dir>" << endl;
					while (disp != nullptr) {
						if (disp->directory) {
							cout << disp->file_name << "<dir>" << endl;
						}
						else {
							cout << disp->file_name << "  " << endl;
						}
						disp = disp->brother_file;
					}
					return;
				}
			}
			disp = disp->brother_file;
		}
		cout << "该目录不存在" << endl;
		return;
	}
	else {
		FCB* disp = root;
		int sign = 0;
		for (int i = 1; i < vs.size(); i++) {
			disp = disp->child_dir;
			while (disp != nullptr) {
				if (disp->file_name == vs[i]) {
					sign = 1;
					break;
				}
				disp = disp->brother_file;
			}
			if (sign == 1) {
				sign = 0;
				continue;
			}
			else {
			cout << "该目录中有目录不存在" << endl;
			return;
			}
		}
		disp = disp->child_dir;
		if (disp == nullptr) {
			cout << "当前目录下无子目录" << endl;
			return;
		}
		else {
			cout << "." << "<dir>" << endl;
			cout << ".." << "<dir>" << endl;
			while (disp != nullptr) {
				if (disp->directory) {
					cout << disp->file_name << "<dir>" << endl;
				}
				else {
					cout << disp->file_name << "  " << endl;
				}
				disp = disp->brother_file;
			}
			return;
		}
	}
}

void rd(string order) {
	vector<string> vs;
	split_path(order, vs,"/");
	if (vs.size() == 1) {
		string target_dir;
		target_dir = vs[0];
		FCB* spid = dir_now->child_dir;
		int sign;
		while (spid != nullptr) {
			if (spid->file_name == target_dir) {
				if (spid->child_dir == nullptr) {
					FCB* targit_dir= spid->father_dir;
					if (targit_dir->child_dir->file_name == spid->file_name) {
						targit_dir->child_dir = spid->brother_file;
						targit_dir->file_size -= sizeof(FCB);
						block->blocks_remain++;
						FAT[spid->first_block] = FREE;
						return;
					}
					else{
						targit_dir = targit_dir->child_dir;
						while (targit_dir->brother_file->file_name != vs[vs.size()-1]) {
							targit_dir = targit_dir->brother_file;
						}
						targit_dir->brother_file = spid->brother_file;
						targit_dir->file_size -= sizeof(FCB);
						block->blocks_remain++;
						FAT[spid->first_block] = FREE;
						return;
					}
				}
				else{
					cout << "该目录不为空!" << endl;
					return;
				}
			}
			spid = spid->brother_file;
		}
		cout << "该目录不存在" << endl;
		return;
	}
	else {

		FCB* disp = root;
		int sign = 0;
		for (int i = 1; i < vs.size(); i++) {
			disp = disp->child_dir;
			while (disp != nullptr) {
				if (disp->file_name == vs[i]) {
					sign = 1;
					break;
				}
				disp = disp->brother_file;
			}
			if (sign == 1) {
				sign = 0;
				continue;
			}
			else {
				cout << "该目录中有目录不存在" << endl;
				return;
			}
		}
		if (disp->child_dir == nullptr) {
			FCB* targit_dir = disp->father_dir;
			if (targit_dir->child_dir->file_name == disp->father_dir->file_name) {
				targit_dir->child_dir = disp->brother_file;
				targit_dir->file_size -= sizeof(FCB);
				block->blocks_remain++;
				FAT[disp->first_block] = FREE;
				return;
			}
			else {
				targit_dir = targit_dir->child_dir;
				while (targit_dir->brother_file->file_name != vs[vs.size()-1]) {
					targit_dir = targit_dir->brother_file;
				}
				targit_dir->brother_file = disp->brother_file;
				targit_dir->file_size -= sizeof(FCB);
				block->blocks_remain++;
				FAT[disp->first_block] = FREE;
				return;
			}
		}
		else {
			cout << "该目录不为空!" << endl;
			return;
		}
	}
}

void dir_crr() {
	FCB* disp = dir_now->child_dir;
	if (disp == nullptr) {
		cout << "当前目录下无子目录" << endl;
	}
	else {
		cout << "." << "<dir>" << endl;
		cout << ".." << "<dir>" << endl;
		while (disp != nullptr) {
			if(disp->directory){
			cout << disp->file_name << "<dir>" << endl;
			}
			else{
				cout << disp->file_name << "  " << endl;
			}
			disp = disp->brother_file;
		}
		return;
	}

}

void cd(string order) {
	vector<string> vs;
	split_path(order, vs, "/");
	if (vs.size() == 1) {
		string target_dir = vs[0];
		FCB* disp = dir_now->child_dir;
		while (disp != nullptr) {
			if (disp->file_name == target_dir) {
				dir_now = disp;
				return;
			}
			disp = disp->brother_file;
		}
		cout << "该目录不存在!" << endl;
		return;
	}
	else{
		FCB* disp = root;
		int sign = 0;
		for (int i = 1; i < vs.size(); i++) {
			disp = disp->child_dir;
			while (disp != nullptr) {
				if (disp->file_name == vs[i]) {
					sign = 1;
					break;
				}
				disp = disp->brother_file;
			}
			if (sign == 1) {
				sign = 0;
				continue;
			}
			else {
				cout << "该目录中有目录不存在" << endl;
				return;
			}
		}
		dir_now = disp;
		return;
	}
}

void new_(string order){
	vector<string> vs;
	vector<string> name;
	vs.clear(); name.clear();
	split_path(order, vs,"/");
	if (vs.size() == 1) {
		split_path(vs[0],name,".");
		FCB* new_file;
		new_file = new FCB;
		new_file->file_name = name[0];
		new_file->file_type = name[1];
		new_file->file_size = 0;
		new_file->text = "";
		new_file->first_block = get_freeblock();
		new_file->directory = false;
		new_file->read_only =false;
		new_file->hidden = false;
		new_file->father_dir = dir_now;
		new_file->child_dir = NULL;
		new_file->brother_file = NULL;
		if (dir_now->child_dir == NULL) {
			dir_now->child_dir = new_file;
		}
		else {
			FCB* disp = dir_now->child_dir;
			while (disp->brother_file != nullptr) {
				disp = disp->brother_file;
			}
			disp->brother_file = new_file;
		}
		dir_now->file_size += sizeof(FCB);
		FAT[new_file->first_block] = END;
		block->blocks_remain--;
		return;
	}
	else{
		int sign;
		FCB* disp = root;
		for (int i = 1; i < vs.size() - 1; i++) {
			disp = disp->child_dir;
			while (disp != nullptr) {
				if (disp->file_name == vs[i]) {
					sign = 1;
					break;
				}
				disp = disp->brother_file;
			}
			if (sign == 1) {
				sign = 0;
				continue;
			}
			else {
				cout << "目录中有目录不可用" << endl;
				return;
			}

		}
	split_path(vs[vs.size()-1], name, ".");
	FCB* new_file;
	new_file = new FCB;
	new_file->file_name = name[0];
	new_file->file_type = name[1];
	new_file->file_size = 0;
	new_file->first_block = get_freeblock();
	new_file->directory = '0';
	new_file->read_only = '0';
	new_file->father_dir = disp;
	new_file->child_dir = NULL;
	new_file->brother_file = NULL;
	if (disp->child_dir == NULL) {
		disp->child_dir = new_file;
	}
	else {
		FCB* target = disp->child_dir;
		while (target->brother_file != nullptr) {
			target = target->brother_file;
		}
		target->brother_file = new_file;
	}

	dir_now->file_size += sizeof(FCB);
	FAT[new_file->first_block] = END;
	block->blocks_remain--;
	return;
	}
}

void del(string order) {
	vector<string> name;
	vector<string> vs;
	split_path(order, vs, "/");
	if (vs.size() == 1) {
		split_path(vs[0], name, ".");
		string target_dir;
		target_dir = name[0];
		FCB* spid = dir_now->child_dir;
		int sign;
		while (spid != nullptr) {
			if (spid->file_name == target_dir) {
				if (spid->child_dir == nullptr) {
					FCB* targit_dir = spid->father_dir;
					if (targit_dir->child_dir->file_name == spid->file_name) {
						targit_dir->child_dir = spid->brother_file;
						targit_dir->file_size -= sizeof(FCB);
						block->blocks_remain++;
						FAT[spid->first_block] = FREE;
						return;
					}
					else {
						targit_dir = targit_dir->child_dir;
						while (targit_dir->brother_file->file_name != vs[vs.size() - 1]) {
							targit_dir = targit_dir->brother_file;
						}
						targit_dir->brother_file = spid->brother_file;
						targit_dir->file_size -= sizeof(FCB);
						block->blocks_remain++;
						FAT[spid->first_block] = FREE;
						return;
					}
				}
				else {
					cout << "这是一个目录！" << endl;
					return;
				}
			}
			spid = spid->brother_file;
		}
		cout << "该文件不存在" << endl;
		return;
	}
	else {
		split_path(vs[vs.size()-1], name, ".");
		vs[vs.size() - 1] = name[0];
		FCB* disp = root;
		int sign = 0;
		for (int i = 1; i < (vs.size()-1); i++) {
			disp = disp->child_dir;
			while (disp != nullptr) {
				if (disp->file_name == vs[i]) {
					sign = 1;
					break;
				}
				disp = disp->brother_file;
			}
			if (sign == 1) {
				sign = 0;
				continue;
			}
			else {
				cout << "该路径中有目录不存在" << endl;
				return;
			}
		}
		if (disp->child_dir == nullptr) {
			FCB* targit_dir = disp->father_dir;
			if (targit_dir->child_dir->file_name == disp->father_dir->file_name) {
				targit_dir->child_dir = disp->brother_file;
				targit_dir->file_size -= sizeof(FCB);
				block->blocks_remain++;
				FAT[disp->first_block] = FREE;
				return;
			}
			else {
				targit_dir = targit_dir->child_dir;
				while (targit_dir->brother_file->file_name != vs[vs.size() - 1]) {
					targit_dir = targit_dir->brother_file;
				}
				targit_dir->brother_file = disp->brother_file;
				targit_dir->file_size -= sizeof(FCB);
				block->blocks_remain++;
				FAT[disp->first_block] = FREE;
				return;
			}
		}
		else {
			cout << "这是一个目录！" << endl;
			return;
		}
	}
}

void edit(string order) {
	vector<string> name;
	vector<string> vs;
	split_path(order, vs, "/");
	if (vs.size() == 1) {
		split_path(vs[0], name, ".");
		string target_dir;
		target_dir = name[0];
		FCB* disp = dir_now->child_dir;
		while (disp != nullptr) {
			if (disp->file_name == target_dir) {
				cout << "请输入更改信息:" << endl;
				string str;
				getline(cin, str);
				disp->text = str;
			}
			disp = disp->brother_file;
		}
	}
}

void type(string order) {
	vector<string> name;
	vector<string> vs;
	split_path(order, vs, "/");
	if (vs.size() == 1) {
		split_path(vs[0], name, ".");
		string target_dir;
		target_dir = name[0];
		FCB* disp = dir_now->child_dir;
		while (disp != nullptr) {
			if (disp->file_name == target_dir) {
				cout << disp->text << endl;
				return;
			}
			disp = disp->brother_file;
		}
	}

}

void copy(string order1, string order2) {
	vector<string> name;
	vector<string> vs;
	vector<string> malloc;
	vector<string> Name;
	string text;
	split_path(order2, vs, "/");
	if (vs.size() == 1) {
		split_path(vs[0], name, ".");
		string target_dir;
		target_dir = name[0];
		FCB* disp = dir_now->child_dir;
		while (disp != nullptr) {
			if (disp->file_name == target_dir) {
				text = disp->text;
			}
			disp = disp->brother_file;
		}

	}
	new_(order1);
	split_path(order1, malloc, "/");
	if (malloc.size() == 1) {
		split_path(malloc[0], Name, ".");
		string target_dir;
		target_dir = Name[0];
		FCB* disp = dir_now->child_dir;
		while (disp != nullptr) {
			if (disp->file_name == target_dir) {
				disp->text = text;
				return;
			}
			disp = disp->brother_file;
		}
	}
	else {
		split_path(malloc[malloc.size() - 1], name, ".");
		malloc[malloc.size() - 1] = Name[0];
		FCB* Disp = root;
		int sign = 0;
		for (int i = 1; i < (malloc.size()); i++) {
			Disp = Disp->child_dir;
			while (Disp != nullptr) {
				if (Disp->file_name == vs[i]) {
					sign = 1;
					break;
				}
				Disp = Disp->brother_file;
			}
			if (sign == 1) {
				sign = 0;
				continue;
			}
			else {
				cout << "该路径中有目录不存在" << endl;
				return;
			}
		}
		Disp->text = text;
		return;
	}
}

int main() {
	int_file_sys();
	string order;
	string command;
	FCB* stand;
	vector<string>vstrings;
	vector<string>nows;
	cout << "designer_name:Li Longzhen" << endl;
	cout << "--------版本号:1.0--------" << endl;
	while (true) {
		command = "";
		stand = dir_now;
		nows.clear();
		vstrings.clear();
		while (stand->file_name != "root") {
			nows.push_back(stand->file_name);
			stand = stand->father_dir;
		}
		for (int i = nows.size() - 1; i >= 0; i=i-1) {
			command += nows[i];
			command += "/";
		}
		cout << "root" << "/" << command << ">";
		getline(cin, order);
		split_path(order, vstrings," ");
		if (vstrings[0] == "md") {
			md(dir_now, vstrings[1]);
		}
		else if (vstrings[0] == "dir") {
			if (vstrings.size() == 1) {
				dir_crr();
			}
			else {
				dir(vstrings[1]);
			}
		}
		else if (vstrings[0] == "rd") {
				rd(vstrings[1]);
		}
		else if (vstrings[0] == "cd") {
			cd(vstrings[1]);
		}
		else if (vstrings[0] == "exit"){
			exit(0);
		}
		else if (vstrings[0] == "cls") {
			system("cls");
		}
		else if (vstrings[0] == "new"){
			new_(vstrings[1]);
		}
		else if (vstrings[0] == "del") {
			del(vstrings[1]);
		}
		else if (vstrings[0] == "edit") {
			edit(vstrings[1]);
		}
		else if (vstrings[0] == "type") {
			type(vstrings[1]);
		}
		else if (vstrings[0]== "copy")
		{
			copy(vstrings[1], vstrings[2]);
		}
	}
}
