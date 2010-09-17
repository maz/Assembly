#include <iostream>
#include <stdio.h>
#include <string.h>
#include "Compiler.h"
#include "Runner.h"

using namespace std;

int main (int argc, char * const argv[]) {
    if(argc<3){
		fprintf(stderr, "At least two arguments required\n");
		return 1;
	}
	if(!strcmp(argv[1],"compile")){
		FILE *f=fopen(argv[2],"r");
		if(!f){
			perror("Assembly");
			return 1;
		}
		Assembly::Compile(f);
		fclose(f);
	}else if(!strcmp(argv[1],"run")){
		FILE *f=fopen(argv[2],"r");
		if(!f){
			perror("Assembly");
			return 1;
		}
		Assembly::Runner run;
		run.exec(f);
		fclose(f);
	}else{
		return 1;
	}
    return 0;
}
