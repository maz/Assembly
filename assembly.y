%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BLANKET_RETURN ("SIZE stack Reg0\nNOT Reg0 Reg0\nCONDJMP Reg0 exit\nSIZE stack Reg0\nMOV 1 Reg1\nMINUS Reg0 Reg1 Reg0\nRACC stack Reg0 Reg0\nPOP stack\nJMP Reg0\n")

int current_label=0;
 
int yywrap()
{
	return 1;
} 

void switch_files(FILE* f);

int main(int argc, char **argv)
{
	if(argc<2)
		fprintf(stderr,"At least one argument required\n");
	FILE *f=fopen(argv[1],"r");
	if(!f){
		perror("compiler");
		return 1;
	}
	switch_files(f);
	printf("JMP func_main\nLABEL exit\nexit\n");
	yyparse();
	return 0;
} 
%}

%union{
	char *txt;
	struct Register{
		char *setup;
		char *name;
	} reg;
}

%token<txt> PRINT RETURN END IDENTIFIER FUNC VAR OPAREN CPAREN COMMA IF
%token<txt> NUMBER EQUAL ADD SUB MULT DIV LT GT EQUAL_EQUAL NOT OR AND WHILE ELSE
%right EQUAL EQUAL_EQUAL LT GT NOT OR AND
%left SUB ADD
%left MULT DIV
%debug
%error-verbose
%type<txt> globals global expr_list statement statements function variable parameter parameters
%type<reg> expr

%%
globals: globals global
	| {$$=NULL;}
	;
global: function
	| variable{
		printf("%s\n",$1);
	}
	;
expr_list: expr{$$=NULL;}
	| expr_list COMMA expr{$$=NULL;}
	;
expr: expr SUB expr{
		asprintf(&$$.setup,"%s\nmov %s Reg5\n%s\nmov %s Reg6\n%s Reg5 Reg6 Reg0\n",$1.setup,$1.name,$3.setup,$3.name,"SUB");asprintf(&$$.name,"Reg0");
	}
	|NUMBER{
		asprintf(&$$.setup,"MOV %s Reg0\n",$1);
		asprintf(&$$.name,"Reg0");
	}
	| IDENTIFIER OPAREN expr_list CPAREN{$$.setup=NULL;$$.name=NULL;}
	| IDENTIFIER OPAREN CPAREN{
		int num=current_label++;
		asprintf(&$$.setup,"EMPTY params\nMOV_lr label_%d Reg0\nPUSH stack Reg0\nJMP func_%s\nLABEL label_%d\n",num,$1,num);
		asprintf(&$$.name,"return");
	}
	| IDENTIFIER{
		asprintf(&$$.setup,"mov %s Reg0",$1);
		asprintf(&$$.name,"Reg0");
	}
	| expr ADD expr{
		asprintf(&$$.setup,"%s\nmov %s Reg5\n%s\nmov %s Reg6\n%s Reg5 Reg6 Reg0\n",$1.setup,$1.name,$3.setup,$3.name,"ADD");asprintf(&$$.name,"Reg0");
	}
	| OPAREN expr CPAREN{
		$$=$2;
	}
	| expr MULT expr{
		asprintf(&$$.setup,"%s\nmov %s Reg5\n%s\nmov %s Reg6\n%s Reg5 Reg6 Reg0\n",$1.setup,$1.name,$3.setup,$3.name,"MULT");asprintf(&$$.name,"Reg0");
	}
	| expr DIV expr{
		asprintf(&$$.setup,"%s\nmov %s Reg5\n%s\nmov %s Reg6\n%s Reg5 Reg6 Reg0\n",$1.setup,$1.name,$3.setup,$3.name,"DIV");asprintf(&$$.name,"Reg0");
	}
	| expr LT expr{
		asprintf(&$$.setup,"%s\nmov %s Reg5\n%s\nmov %s Reg6\n%s Reg5 Reg6 Reg0\n",$1.setup,$1.name,$3.setup,$3.name,"LT");asprintf(&$$.name,"Reg0");
	}
	| expr GT expr{
		asprintf(&$$.setup,"%s\nmov %s Reg5\n%s\nmov %s Reg6\n%s Reg5 Reg6 Reg0\n",$1.setup,$1.name,$3.setup,$3.name,"GT");asprintf(&$$.name,"Reg0");
	}
	| expr EQUAL_EQUAL expr{
		asprintf(&$$.setup,"%s\nmov %s Reg5\n%s\nmov %s Reg6\n%s Reg5 Reg6 Reg0\n",$1.setup,$1.name,$3.setup,$3.name,"EQ");asprintf(&$$.name,"Reg0");
	}
	| NOT expr{
		asprintf(&$$.setup,"%s\nmov %s Reg0\nNOT Reg0 Reg0\n",$2.setup,$2.name);
		asprintf(&$$.name,"Reg0");
	}
	| expr OR expr{
		asprintf(&$$.setup,"%s\nmov %s Reg5\n%s\nmov %s Reg6\n%s Reg5 Reg6 Reg0\n",$1.setup,$1.name,$3.setup,$3.name,"OR");asprintf(&$$.name,"Reg0");
	}
	| expr AND expr{
		asprintf(&$$.setup,"%s\nmov %s Reg5\n%s\nmov %s Reg6\n%s Reg5 Reg6 Reg0\n",$1.setup,$1.name,$3.setup,$3.name,"AND");asprintf(&$$.name,"Reg0");
	}
	;
statement: variable
	| PRINT expr
	{
		asprintf(&$$,"%s\nPRINT %s\n",$2.setup,$2.name);
	}
	| RETURN expr{
		asprintf(&$$,"%s\nMOV %s return\n%s\n",$2.setup,$2.name,BLANKET_RETURN);
	}
	| RETURN{
		asprintf(&$$,"%s\n",BLANKET_RETURN);
	}
	| IDENTIFIER EQUAL expr{
		asprintf(&$$,"%s\nMOV %s %s",$3.setup,$1,$3.name);
	}
	| expr{
		asprintf(&$$,"%s\n",$1.setup);
	}
	| WHILE expr statements END{
		int cur=current_label++;
		int after=current_label++;
		asprintf(&$$,"LABEL label_%d\n%s\nCNDJMP %s label_%d\n%s\nJMP label_%d\nLABEL label_%d",cur,$2.setup,$2.name,after,$3,cur,after);
	}
	;
statements: statements statement{
		asprintf(&$$,"%s\n%s\n",$1,$2);
	}
	| {$$=calloc(1,sizeof(char));}
	;
function: FUNC IDENTIFIER OPAREN parameters CPAREN statements END
	{
		$$=$2;
		printf("LABEL func_%s\n",$2);
		printf("%s\n",$4);
		printf("%s\n",$6);
		free($4);
		free($6);
		printf(BLANKET_RETURN);
	}
	;
variable: VAR IDENTIFIER IDENTIFIER
	{
		asprintf(&$$,"VAR %s %s\n",$2,$3);
	}
	;
parameter: IDENTIFIER IDENTIFIER
	;
parameters: parameter
	| parameters COMMA parameter
	| {$$=calloc(1,sizeof(char));}
	;
%%
extern char yytext[];
extern int column;
extern char *current_line;

yyerror(s)
char *s;
{
	fflush(stdout);
	fflush(stderr);
	fprintf(stderr,"\n%*s\n%*s\n",column, "^", column, s);
}