#include "Assembly.h"

using namespace Assembly;
using namespace std;

#include <string>
#include <stdlib.h>
#include <ctype.h>

string Assembly::read_token(FILE *f){
	string str;
	char ch;
	while((ch=fgetc(f)) && ch!=EOF && isspace(ch)){}
	ungetc(ch, f);
	while((ch=fgetc(f)) && ch!=EOF && (isalnum(ch) || ch=='_' || ch=='-')){
		str+=ch;
	}
	return str;
}

bool Assembly::str_is_num(string str){
	for(int i=0;i<str.size();i++){
		if(!isdigit(str[i]))
			return false;
	}
	return true;
}

Number Assembly::aton(const char* str){return atoi(str);}