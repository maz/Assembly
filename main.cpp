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
	FILE *f=fopen(argv[1],"r");
	if(!f){
		perror("Assembly");
		return 1;
	}
	Assembly::Runner run;
	run.exec(f);
	fclose(f);
    return 0;
}
