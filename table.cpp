/*
매크로 프로세서 테이블 cpp 파일
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#define MAXSIZE 10
#define MAXLINE 100
#define MAXARG 30

using namespace std;

class NAMTAB { // 해시 테이블 형태의 NAMTAB
public:
	void setname(string name) {
		this->name = name;
	}
	void setindex(int s, int e) {
		this->startindex = s;
		this->endindex = e;
	}
	string name;
	int startindex = 0;
	int endindex = 0;
	NAMTAB* next = NULL;
};

class DEFTAB { // 배열 방식의 DEFTAB
public:
	void setline(string mline, string mlabel, string mopcode, string moperand, int index) {
		this->mline[index] = mline;
		this->mlabel[index] = mlabel;
		this->mopcode[index] = mopcode;
		this->moperand[index] = moperand;
	}
	string mline[MAXLINE];
	string mlabel[MAXLINE];
	string mopcode[MAXLINE];
	string moperand[MAXLINE];
};

class ARGTAB { // 리스트 방식의 ARGTAB
public:
	void setarg(string arg) {
		this->argument = arg;
	}
	bool getarg(string arg) {
		if (this->argument == arg) return true;
		else return false;
	}
	void setvalue(string val) {
		value = val;
	}
	bool is_val_empty() {
		if (value.empty()) return true;
		else return false;
	}
	string argument;
	string value;
};
