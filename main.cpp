#include <iostream>
#include <stdio.h>
#include <string.h>
#include "Runner.h"

using namespace std;

int main (int argc, char * const argv[]) {
    if(argc<2){
		fprintf(stderr, "At least one argument required\n");
		return 1;
	}
	bool debug=false;
	int i;
	for(i=1;i<argc;i++){
		if(!strcmp(argv[i],"-debug")){
			debug=true;
		}else{
			break;
		}
	}
	FILE *f=fopen(argv[i],"r");
	if(!f){
		perror("Assembly");
		return 1;
	}
	Assembly::Runner run(debug);
	run.exec(f);
	fclose(f);
    return 0;
}
