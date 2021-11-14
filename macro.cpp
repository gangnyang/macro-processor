/*
2020111983 전동원 매크로 프로세서 메인 파일
*/
#define _CRT_SECURE_NO_WARNINGS
#include "table.cpp"
#define MAXSIZE 10
#define MAXLINE 100
#define MAXARG 30

using namespace std;

int hashing(string s);
void insertnam(string nam);
void insertindex(string nam, int s, int e);
bool findnam(string nam, int& s, int& e);
void setargtab(string s);
void mgetline(int &index);
void processline();
void define();
void expand(int s, int e);
void setargtab(string s);
void setargvalue(string s);
void uniqueval(int s, int e, int macronum, int level);
void substitute(string& s);
void setval(string s1, string s2);
bool condition(string s);

int index = 0; // 읽는 줄 수 
bool expanding = false; // 확장 중인지
bool set = false; // set 구문인지
ifstream inpf;
ofstream outpf;
string line; // 출력할 문장
string element[MAXARG]; // keyword 방식을 위함
string replacement[MAXARG]; // keyword 방식을 위함
string label[MAXLINE];
string opcode[MAXLINE];
string operand[MAXLINE];
string backup[MAXLINE]; // operand를 unique label로 교체하기 위한 변수

NAMTAB* namta[MAXSIZE];
DEFTAB defta;
ARGTAB argta[MAXARG];

int backupindex = 0; // backup변수의 index
int defindex = 0; // deftab에 넣기 위한 index
int argindex = 0; // argtab에 넣는 index
int argend=0; // keyword 방식을 위함
int macronum = 0; // 현재 몇 번째 macro인지(nested X)

int main(int argc, char** argv) {

	if (argc == 2) {
		inpf.open(argv[1]);
	}
	else inpf.open("macro");
	if (inpf.fail()) {
		cout << "File Error!" << endl;
		exit(100);
	}
	outpf.open("output"); // 출력파일
	while (opcode[index] != "end") {
		mgetline(index);
		processline();
	}
	inpf.close();
	outpf.close();
}

int hashing(string s) {//해시값을 얻는 함수
	int hash = 0;
	const char* k = s.c_str();//string에서 char로 변환하여 해시값을 얻는다
	while (*k != NULL) {
		hash = hash + (int)(*k);
		k++;
	}
	return hash % MAXSIZE;
}

void insertnam(string nam) { // namtab에 name값을 넣는 함수
	int index = hashing(nam);
	NAMTAB* newtab = new NAMTAB;
	newtab->setname(nam);
	if (namta[index] == NULL) {
		namta[index] = newtab;
	}
	else {
		NAMTAB* cursor = namta[index];
		while (cursor->next != NULL) {
			cursor = cursor->next;
		}
		cursor->next = newtab;
	}
}

void insertindex(string nam, int s, int e) { // namtab에 name을 대조해서 startindex와 endindex 삽입
	int index = hashing(nam);
	if (namta[index]->name == nam) {
		namta[index]->setindex(s, e);
	}
	else {
		NAMTAB* cursor = namta[index];
		while (cursor->next != NULL) {
			cursor = cursor->next;
			if (cursor->name == nam)
				cursor->setindex(s, e);
		}
	}
}

bool findnam(string nam, int& s, int& e) {//NAMTAB에 존재하는지 확인
	int index = hashing(nam);//해시값을 구하고
	NAMTAB* cursor = namta[index];
	while (cursor != NULL) {
		if (cursor->name == nam) {//name 존재 시 true 반환
			s = cursor->startindex;
			e = cursor->endindex;
			return true;
		}
		cursor = cursor->next;// 다를 경우 다음 주소로 이동
	}
	return false; //없을 경우 false 반환
}

void setargtab(string s) { // argtab에 argument를 넣는 함수
	istringstream iss(s);
	int i = 0;
	string temp1;
	string temp2;
	if (s.find("=") == string::npos) { // position 방식일 경우
		while (getline(iss, element[i], ',')) {
			argta[argindex].setarg(element[i++]);
			argindex++;
		}
	}
	else { // keyword 방식일 경우
		while (getline(iss, element[i], ',')) {
			temp1 = "";
			temp2 = "";
			temp1 = element[i].substr(0, element[i].find("="));
			if (element[i].back() != '=') { // = 뒤에 인수가 있는 경우 value로 설정
				temp2 = element[i].substr(element[i].find("=") + 1);
				argta[argindex].setarg(temp1);
				argta[argindex].setvalue(temp2);
				i++;
				argindex++;
			}
			else { // = 로 끝날 경우 argument만 set
				argta[argindex].setarg(temp1);
				argta[argindex].setvalue(" ");
				i++;
				argindex++;
			}
		}
	}
	argend = argindex;
}

void setargvalue(string s) { // argtab에 value를 넣는 함수
	istringstream iss(s);
	int i=0, j=0;
	string temp;
	string temp1;
	string temp2;
	if (s.find("=") == string::npos) { // position 방식인 경우
		for (int a = 0; a < MAXARG; a++) {
			if (argta[a].is_val_empty()) {
				j = a;
				break;
			}
		}
		while (getline(iss, replacement[i], ',')) {
			if (replacement[i].find("(") != string::npos) { // 배열로 입력받는 경우 예외처리
				while (1) {
					temp = "";
					getline(iss, temp, ',');
					replacement[i].append(temp);
					if (temp.find(")") != string::npos)
						break;
				}
			}
			argta[j].setvalue(replacement[i++]);
			j++;
		}
	}
	else {
		while (getline(iss, replacement[i], ',')) {
			temp1 = replacement[i].substr(0, replacement[i].find("=")); // = 을 기준으로 반으로 가름
			temp2 = "";
			if (replacement[i].back() != '=') {
				temp2= replacement[i].substr(replacement[i].find("=") + 1);
				for (int a = 0; a < MAXARG; a++) {
					if (argta[a].argument.find(temp1) != string::npos) { // temp1을 argument에서 찾으면 value에 temp2 set
						argta[a].setvalue(temp2);
					}
				}
			}
			else {
				temp2 = " ";
				for (int a = 0; a < MAXARG; a++) {
					if (argta[a].argument.find(temp1) != string::npos) { // temp1을 argument에서 찾으면 value에 temp2 set
						argta[a].setvalue(temp2);
					}
				}
			}
			i++;
		}
	} // keyword 방식인 경우
	for (int i = 0; i < argend; i++) { // 입력받지 않았을 경우 공백 처리
		if (argta[i].is_val_empty())
			argta[i].setvalue(" ");
	}
}
// keyword 와 position을 섞는 경우는 구현하지 않았음

void uniqueval(int s, int e, int macronum, int level) { // operand 부분을 unique value로 바꾸는 함수
	string temp="&&";
	temp[0] = (char)(96 + macronum); // unique label을 위함
	temp[1] = (char)(96 + level);
	for (int i = s; i < e; i++) {
		for (int j = 0; j < backupindex; j++) {
			if (defta.moperand[i] == backup[j]) {
				if (defta.moperand[i][0] == '$')
					defta.moperand[i].insert(1, temp);
				else
					defta.moperand[i].insert(0, temp);
			}
		}
	}
}

void substitute(string &s) { // argument를 vaule로 바꾸는 함수
	string::size_type a;
	for (int i = 0; i < argend; i++) {
		string temp = argta[i].argument;
		if (s.find(temp) != string::npos) {
			if(s.substr(s.find(temp)+temp.length(), 1)=="'"|| s.substr(s.find(temp) + temp.length(), 1) == ","|| \
				s.substr(s.find(temp) + temp.length(), 1) == " "|| s.substr(s.find(temp) + temp.length(), 1).empty())
				s.replace(s.find(temp), temp.length(), argta[i].value);
			if (s.find("->") != string::npos) {
				a = s.find("->");
				s.erase(a, 2); // -> 제거
			}
		}
	}
}

void setval(string s1, string s2) { // set 일 때 value 설정 숫자나 변수만 받는다고 가정
	for (int i = 0; i < argend; i++) {
		if (argta[i].argument == s1) {
			argta[i].setvalue(s2);
		}
	}
}

bool condition(string s) { // if 문이나 while문 비교
	stringstream ss(s);
	string arg1;
	string cond;
	string arg2;
	string temp;
	ss >> arg1;
	ss >> cond;
	ss >> arg2;
	ss >> temp;
	if (arg1.front() == '(') {
		arg1.erase(0, 1);
	}
	if (arg1.empty()) arg1 = " ";
	if (!temp.empty()) arg2.append(temp);
	if (arg2 == "'')") {
		arg2 = " ";
	}
	else if (arg2 == ")") {
		arg2 = " ";
	}
	if (arg2.back() == ')') {
		arg2.pop_back();
	}
	if (cond == "lt") { // le와 ge는 구현하지 않고 lt와 gt는 숫자일 때만 작동한다
		if (stoi(arg1) < stoi(arg2)) return true;
		else return false;
	}
	else if (cond == "eq") {
		if (arg1 == arg2) return true;
		else return false;
	}
	else if (cond == "ne") {
		if (arg1 != arg2) return true;
		else return false;
	}
	else if (cond == "gt") {
		if (stoi(arg1) > stoi(arg2)) return true;
		else return false;
	}
	else {
		outpf << "Condition Error" << endl;
		return false;
	}
}

void mgetline(int &index) { // getline
	string s;
	string temp = "";
	string tlabel;
	string topcode;
	string toperand;
	if (expanding) { // 확장 중일 경우 deftab에서 읽어옴
		line = "";
		tlabel = defta.mlabel[index];
		topcode = defta.mopcode[index];
		toperand = defta.moperand[index];
		substitute(toperand); // 대체
		if (topcode == "set") { // set일 경우 value set
			set = true;
			setval(tlabel, toperand);
		}
		if (topcode == "if") { //if문일 경우
			if (condition(toperand)) { // 조건을 따져보고
				index++;
				while (defta.mopcode[index] != "else" && defta.mopcode[index] != "endif") { // else나 endif까지
					mgetline(index);
					processline();
					index++;
				}
				if (defta.mopcode[index] == "else") { // 조건이 맞았으므로 else쪽은 작동하지 않는다
					while (defta.mopcode[index] != "endif") {
						index++;
					}
				}
			}
			else { 
				while (defta.mopcode[index]!="else"&&defta.mopcode[index] != "endif") {
					index++;
				}
				if (defta.mopcode[index] == "else") { // 틀릴 경우 else쪽을 실행
					index++;
					while (defta.mopcode[index] != "endif") {
						mgetline(index);
						processline();
						index++;
					}
				}
			}
			set = true; // if문 출력 예외처리
		}
		/*
		LOOP는 구현하였으나 배열과 사칙연산이 미구현 상태이기 때문에
		실행 과정은 존재하지 않는다.
		*/
		else if (topcode == "while") { 
			int b_index = index;
			if (condition(toperand)) { // 조건을 미리 따져보고
				while (condition(toperand)) { // 반복
					index++;
					while (defta.mopcode[index] != "endw") { // while문이 나가질 때까지 계속 반복
						mgetline(index);
						processline();
						index++;
					}
					index = b_index; // while문 시작 index
				}
				
			}
			else { // 아닐 경우 while문 넘기기 위해 줄넘김
				while (defta.mopcode[index] != "endw") {
					index++;
				}
			}
			set = true; // while문 출력 예외처리
		}
		line = tlabel + " " +  topcode + " " + toperand; // processline 함수에서 출력하기 위해 line에 append
	}
	else {
		getline(inpf, line);//한 줄씩 읽어간다
		stringstream ss(line);//쪼개기 위해 stringstream 이용
		s = line.front();//label이 없는 것을 구분하기 위함
		if (s.compare(" ") == 0) {//라벨이 없다면
			label[index] = s;//공백 저장
			ss >> opcode[index];
			ss >> operand[index];
			ss >> temp;
			while (!temp.empty()||temp!="") {
				operand[index].append(" ");
				operand[index].append(temp); // 콤마 뒤에 띄어쓰기가 있을 경우 처리
				temp = "";
				ss >> temp;
			}
		}
		else {//라벨이 있다면 순차적으로 저장
			ss >> label[index];
			ss >> opcode[index];
			ss >> operand[index];
			ss >> temp;
			while (!temp.empty()||temp!="") {
				operand[index].append(temp); // 콤마 뒤에 띄어쓰기가 있을 경우 처리
				operand[index].append(" ");
				temp = "";
				ss >> temp;
			}
		}
	}
}

void processline() { // 확장, 정의로 넘기거나 출력하는 함수
	int s, e;
	if (findnam(opcode[index], s, e)) { // macro 이름과 일치할 경우 expand
		expand(s, e);
	}
	else if (opcode[index] == "macro") { // opcode==macro일 경우 정의
		define();
	}
	else { // 출력
		if (!set) {
			outpf << line << endl;
			cout << line << endl;
		}
		set = false;
	}
}

void define() { // 매크로 정의 함수
	int startindex[MAXSIZE] = { 0 };
	int endindex[MAXSIZE] = { 0 };
	int level = 1;
	macronum++;
	startindex[level] = index;
	insertnam(label[index]);
	defta.setline(line, label[index], opcode[index], operand[index], defindex);
	defindex++;
	setargtab(operand[index]); // argument 설정
	while (level > 0) {
		index++;
		mgetline(index);
		//if not comment line (구현 X)
		if (opcode[index] != "set") {
			string temp = "&&";
			temp[0] = (char)(96 + macronum); // unique label을 위함
			temp[1] = (char)(96 + level);
			if (!label[index].empty() && label[index] != " ") {
				backup[backupindex++] = label[index];
				if (label[index][0] == '$')
					label[index].insert(1, temp);
				else
					label[index].insert(0, temp);
			}
		}
		defta.setline(line, label[index], opcode[index], operand[index], defindex);
		defindex++;
		if (opcode[index] == "set") { // set일 경우 argtab에 넣는다
			argta[argindex].setarg(label[index]);
			argindex++;
			argend++;
		}
		if (opcode[index] == "macro") {
			level++;
			startindex[level] = index;
		}
		else if (opcode[index] == "mend") {
			endindex[level] = index;
			insertindex(label[startindex[level]], startindex[level], endindex[level]); // namtab의 index를 설정
			uniqueval(startindex[level], endindex[level], macronum, level); // unique label을 operand 부분도 처리
			index++;
			level--;
		}
	}
}

void expand(int s, int e) { // 확장 함수
	expanding = true; // 확장 상태
	setargvalue(operand[index]); // value값을 설정
	string temp;
	temp[0] = (char)(65);
	string back = label[index]; // 기존의 label 저장
	if (!label[index].empty() && label[index] != " ") // 비어있지 않은 경우 unique label
		label[index].insert(0, ".");
	outpf << label[index] << " " << defta.mlabel[s] << " " << operand[index] << endl;
	if (!label[index].empty() && label[index] != " ")
		outpf << back;
	cout<< label[index] << " " << defta.mlabel[s] << " " << operand[index] << endl;
	if (!label[index].empty() && label[index] != " ")
		cout << back;
	index++;
	for( int i=s+1; i<e; i++){ // 선언부가 아닌 나머지 line도 출력
		mgetline(i);
		processline();
	}
	expanding = false;
}
