%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "assembly.tab.h"
	
	void switch_files(FILE *f){
		yyin=f;
		yy_switch_to_buffer(yy_create_buffer( yyin, YY_BUF_SIZE ) );
	}
	void count();
%}

%x COMMENT
%%
<<EOF>>						{count();yyterminate();}
<COMMENT>\n					{count();BEGIN(INITIAL);}
<COMMENT>[^\n]+				{count();}
<INITIAL>";"				{count();BEGIN(COMMENT);}
<INITIAL>"//"				{count();BEGIN(COMMENT);}
end							count();return END;
func						count();return FUNC;
var							count();return VAR;
print						count();return PRINT;
return						count();return RETURN;
if							count();return IF;
while						count();return WHILE;
else						count();return ELSE;
"("							count();return OPAREN;
")"							count();return CPAREN;
","							count();return COMMA;
[a-zA-Z_][a-zA-Z_0-9]*		{count();yylval.txt=strdup(yytext);return IDENTIFIER;}
-?[0-9]+					{count();yylval.txt=strdup(yytext);return NUMBER;}
"=="						count();return EQUAL_EQUAL;
"="							count();return EQUAL;
"+"							count();return ADD;
"-"							count();return SUB;
"*"							count();return MULT;
"/"							count();return DIV;
"<"							count();return LT;
">"							count();return GT;
"||"						count();return OR;
"&&"						count();return AND;
"!"							count();return NOT;
[\n \t  ]					count();
%%
int column = 0;
//char *current_line=NULL;

void count()
{
	/*int i;
	if(!current_line)
		current_line=calloc(1,sizeof(char));
	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n'){
			column = 0;
			free(current_line);
			current_line=calloc(1,sizeof(char));
		}else if (yytext[i] == '\t'){
			column += 8 - (column % 8);
			int len=strlen(current_line);
			current_line=realloc(current_line,len+2);
			current_line[len]='\t';
			current_line[len+1]='\0';
		}else{
			column++;
			int len=strlen(current_line);
			current_line=realloc(current_line,len+2);
			current_line[len]='\t';
			current_line[len+1]='\0';
		}*/
		int i;

			for (i = 0; yytext[i] != '\0'; i++)
				if (yytext[i] == '\n')
					column = 0;
				else if (yytext[i] == '\t')
					column += 8 - (column % 8);
				else
					column++;

			//fwrite( yytext, yyleng, 1, stderr );
}