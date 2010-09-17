#include "Compiler.h"

using namespace std;

void priv_compile(FILE *f);

void Assembly::Compile(FILE *f){
	priv_compile(f);
}

using namespace Assembly;

void priv_compile(FILE *f){
	/*while(!feof(f)){
		string token=read_token(f);
		if(token=="var"){
			printf("var %s %s\n",read_token(f).c_str(),read_token(f).c_str());
		}
	}*/
}