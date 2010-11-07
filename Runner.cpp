#include "Runner.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;
using namespace Assembly;

Number* Runner::GetRegValue(std::string txt){
	lowercase(txt);
	if(txt=="reg0")
		return &Reg0;
	else if(txt=="reg1")
		return &Reg1;
	else if(txt=="reg2")
		return &Reg2;
	else if(txt=="reg3")
		return &Reg3;
	else if(txt=="reg4")
		return &Reg4;
	else if(txt=="reg5")
		return &Reg5;
	else if(txt=="reg6")
		return &Reg6;
	else if(txt=="return")
		return &Return;
	else
		return NULL;
}

#define handle_error(...) fprintf(stderr,"From: %s line %d, function %s, file pos %d: ",__FILE__,__LINE__,__FUNCTION__,ftell(f));fprintf(stderr,__VA_ARGS__);if(break_on_error){debug(f,true);}else{exit(1);}

void Runner::do_jmp(FILE *f){
	string token=read_token(f);
	Number *ptr=GetRegValue(token);
	if(ptr){
		fseek(f, *ptr, SEEK_SET);
	}else if(Labels.count(token)){
		lowercase(token);
		fseek(f, Labels[token], SEEK_SET);
	}else{
		handle_error( "Label %s does not exist\n",token.c_str());
	}
}

#define return {break_on_error=old;return;}
void Runner::debug(FILE *source,bool only_check){
	bool old=break_on_error;
	break_on_error=false;
	while(1){
		char *line_buf=readline("debugger>> ");
		if(!line_buf)
			return;
		if(strlen(line_buf)){
			add_history(line_buf);
			string line(line_buf);
			if(!only_check && (line.substr(0,4)=="eval"||line.substr(0,4)=="exec")){
				line=line.substr(4);
				FILE *f=tmpfile();
				fprintf(f,"%s\n",line.c_str());
				fseek(f,0,SEEK_SET);
				exec(f);
				fclose(f);
			}else{
				lowercase(line);
				if(line=="exit"){
					if(only_check){
						exit(0);
					}else{
						return;
					}
				}else if(line=="dumpregisters"||line=="dump_registers"){
					printf("%s: ","Reg0");
					printf("%d\n",Reg0);
					printf("%s: ","Reg1");
					printf("%d\n",Reg1);
					printf("%s: ","Reg2");
					printf("%d\n",Reg2);
					printf("%s: ","Reg3");
					printf("%d\n",Reg3);
					printf("%s: ","Reg4");
					printf("%d\n",Reg4);
					printf("%s: ","Reg5");
					printf("%d\n",Reg5);
					printf("%s: ","Reg6");
					printf("%d\n",Reg6);
					printf("%s: ","Return");
					printf("%d\n",Return);
					printf("%s: ","Stack");
					PrintValueList(&Stack);
					printf("%s: ","Parameters");
					PrintValueList(&Parameters);
					printf("%s: ","LReg0");
					PrintValueList(&LReg0);
					printf("%s: ","LReg1");
					PrintValueList(&LReg1);
					printf("%s: ","LReg2");
					PrintValueList(&LReg2);
					printf("%s: ","LReg3");
					PrintValueList(&LReg3);
				}else if(!only_check && line=="step"){
					next_debug=true;
					return;
				}else if(line=="frame"){
					long cur_pos=ftell(source);
					unsigned int lineno=1;
					char ch;
					fseek(source,0,SEEK_SET);
					while(ftell(source)!=cur_pos){
						ch=fgetc(source);
						if(ch=='\n')
							++lineno;
					}
					printf("#%d: ",lineno);
					fseek(source,0,SEEK_SET);
					unsigned int my_lineno=1;
					while(my_lineno!=lineno){
						ch=fgetc(source);
						if(ch=='\n')
							++my_lineno;
					}
					while((ch=fgetc(source)) && ch!=EOF && ch!='\n'){
						fputc(ch,stdout);
					}
					fputc('\n',stdout);
					fseek(source,cur_pos,SEEK_SET);
				}else{
					char *data=strdup(line.c_str());
					string cmd(strtok(data," "));
					string var(strtok(NULL," "));
					free(data);
					lowercase(cmd);
					if(cmd=="print"){
						if(GetRegValue(var)){
							printf("%d\n",*GetRegValue(var));
						}else if(GetListRegValue(var)){
							PrintValueList(GetListRegValue(var));
						}else if(Labels.count(var)){
							printf("%d\n",Labels[var]);
						}else if(Variables.count(var)){
							if(Variables[var].isList){
								PrintValueList(&(Variables[var].list));
							}else{
								printf("%d\n",Variables[var].num);
							}
						}else{
							fprintf(stderr,"Unknown variable %s\n",var.c_str());
						}
					}else{
						fprintf(stderr,"Unknown debugger command \"%s\"\n",line.c_str());
					}
				}
			}
		}
		free(line_buf);
	}
}
#undef return;

ValueList* Runner::GetListRegValue(string txt){
	lowercase(txt);
	if(txt=="params")
		return &Parameters;
	else if(txt=="stack")
		return &Stack;
	else if(txt=="lreg0")
		return &LReg0;
	else if(txt=="lreg1")
		return &LReg1;
	else if(txt=="lreg2")
		return &LReg2;
	else if(txt=="lreg3")
		return &LReg3;
	else
		return NULL;
}

void Runner::PrintValueList(ValueList *lptr,int indent){
	ValueList::iterator iter=lptr->begin();
	int i;
	for(i=0;i<indent;i++)
		fputc(' ',stdout);
	printf("LIST:\n");
	while(iter!=lptr->end()){
		for(i=0;i<indent;i++)
			fputc(' ',stdout);
		if((*iter).isList){
			fputc('*',stdout);
			fputc(' ', stdout);
			PrintValueList(&(*iter).list, indent+4);
		}else{
			printf("%d\n",(*iter).num);
		}
		iter++;
	}
}

void Runner::lowercase(std::string &token){
	for(int i=0;i<token.size();i++){
		if(isalpha(token[i]))
			token[i]=tolower(token[i]);
	}
}

/*
Instructions:
	Label     Print        Mov_lr    Pop_Front    Racc    Eq     Gt       Debugger
	Jmp       Exit         Mov_vr    Pop_Back     Add     Or     Lt
	Cndjmp    Mov          Size      Push         Minus   And    Empty
	Var       Mov_Const    Mov_rv    Not          Div     Mult   Mov_rr
*/

void Runner::exec(FILE *f){
	while(!feof(f)){
		string token=read_token(f);
		lowercase(token);
		if(token=="label"){
			token=read_token(f);
			lowercase(token);
			Labels[token]=ftell(f);
		}
	}
	fseek(f, 0, SEEK_SET);
	while(!feof(f)){
		if(next_debug){
			next_debug=false;
			debug(f);
		}
		string token=read_token(f);
		lowercase(token);
		if(token=="jmp"){
			do_jmp(f);
		}else if(token=="halt" || token=="exit"){
			token=read_token(f);
			int retCode=0;
			if(token.size())
				retCode=atoi(token.c_str());
			exit(retCode);
		}else if(token=="cndjmp"||token=="condjmp"||token=="cond_jmp"){
			token=read_token(f);
			int* ptr=GetRegValue(token);
			if(ptr){
				if(*ptr){
					do_jmp(f);
				}else{
					read_token(f);//get rid of the label to jump to
				}
			}else{
				handle_error( "Unknown numerical register %s\n",token.c_str());
			}
		}else if(token=="empty"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(lptr){
				while(lptr->size())
					lptr->pop_front();
			}else{
				handle_error( "Unknown list register %s\n",token.c_str());
			}
		}else if(token=="var"){
			string name=read_token(f);
			string type=read_token(f);
			lowercase(type);
			Variables[name]=Value((type=="list"));
		}else if(token=="label"){
			read_token(f);
		}else if(token=="debugger"){
			next_debug=true;
		}else if(token=="mov"){
			string str="";
			token=read_token(f);
			if(str_is_num(token)){
				str="mov_const "+token;
			}else if(Labels.count(token)){
				str="mov_lr "+token;
			}else if(GetRegValue(token)||GetListRegValue(token)){
				string n=read_token(f);
				if(GetRegValue(n)||GetListRegValue(n)){
					str="mov_rr "+token+" "+n+"\n";
				}else{
					str="mov_rv "+token+" "+n+"\n";
				}
			}else{
				str="mov_vr "+token;
			}
			ungetc(' ',f);
			for(int i=str.size()-1;i>-1;i--){
				ungetc(str[i],f);
			}
		}else if(token=="mov_const"){
			token=read_token(f);
			Number num=aton(token.c_str());
			token=read_token(f);
			Number* ptr=GetRegValue(token);
			if(ptr){
				*ptr=num;
			}else{
				handle_error( "Unknown numerical register %s\n",token.c_str());
			}
		}else if(token=="mov_rr"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(lptr){
				token=read_token(f);
				ValueList *lptr2=GetListRegValue(token);
				if(!lptr2){
					handle_error("Unknown list register %s\n",token.c_str());
				}
				while(lptr2->size())
					lptr2->pop_front();
				lptr2->insert(lptr2->begin(),lptr->begin(),lptr->end());
			}else{
				Number* ptr=GetRegValue(token);
				if(ptr){
					token=read_token(f);
					Number* ptr1=GetRegValue(token);
					if(ptr1){
						*ptr1=*ptr;
					}else{
						handle_error( "Unknown numerical register %s\n",token.c_str());
					}
				}else{
					handle_error( "Unknown numerical register %s\n",token.c_str());
				}
			}
		}else if(token=="mov_lr"){
			token=read_token(f);
			lowercase(token);
			if(Labels.count(token)){
				string reg=read_token(f);
				Number* ptr=GetRegValue(reg);
				if(ptr){
					*ptr=Labels[token];
				}else{
					handle_error( "Unknown numerical register %s\n",token.c_str());
				}
			}else{
				handle_error("The label %s does not exist\n",token.c_str());
			}
		}else if(token=="mov_vr"){
			token=read_token(f);
			if(!Variables.count(token)){
				handle_error( "Unknown variable %s\n",token.c_str());
			}
			string name=token;
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(lptr){
				while(lptr->size())
					lptr->pop_front();
				ValueList::iterator iter=Variables[name].list.begin();
				while(iter!=Variables[name].list.end()){
					lptr->push_back(*iter);
					iter++;
				}
			}else{
				int* ptr=GetRegValue(token);
				if(ptr){
					*ptr=Variables[name].num;
				}else{
					handle_error( "Unknown numerical register %s\n",token.c_str());
				}
			}
		}else if(token=="size"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				handle_error( "Unknown list register %s\n",token.c_str());
			}
			token=read_token(f);
			Number* ptr=GetRegValue(token);
			if(ptr){
				*ptr=lptr->size();
			}else{
				handle_error( "Unknown numerical register %s\n",token.c_str());
			}
		}else if(token=="mov_rv"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			string name=read_token(f);
			if(!Variables.count(name)){
				handle_error( "Unknown variable %s\n",name.c_str());
			}
			if(lptr){
				while(Variables[name].list.size())
					Variables[name].list.pop_front();
				ValueList::iterator iter=lptr->begin();
				while(iter!=lptr->end()){
					Variables[name].list.push_back(*iter);
					iter++;
				}
			}else{
				Number* ptr=GetRegValue(token);
				if(ptr){
					Variables[name].num=*ptr;
				}else{
					handle_error( "Unknown numerical register %s\n",token.c_str());
				}
			}
		}else if(token=="not"){
			token=read_token(f);
			Number *a=GetRegValue(token);
			if(!a){
				handle_error( "Unknown numerical register %s\n",token.c_str());
			}
			token=read_token(f);
			Number *b=GetRegValue(token);
			if(!b){
				handle_error( "Unknown numerical register %s\n",token.c_str());
			}
			*b=!*a;
		}else if(token=="print"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(lptr){
				PrintValueList(lptr);
			}else{
				Number* ptr=GetRegValue(token);
				if(!ptr){
					handle_error( "Unknown numerical register %s\n",token.c_str());
				}
				printf("%d\n",*ptr);
			}
		}else if(token=="push"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				handle_error( "Unknown list register %s\n",token.c_str());
			}
			token=read_token(f);
			ValueList *nptr=GetListRegValue(token);
			if(nptr){
				handle_error("multi-dimentional lists not yet fully implemented\n");
			}else{
				Number* ptr=GetRegValue(token);
				if(!ptr){
					handle_error( "Unknown numerical register %s\n",token.c_str());
				}
				Value val(false);
				val.num=*ptr;
				lptr->push_back(val);
			}
		}else if(token=="pop"||token=="pop_back"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				handle_error( "Unknown list register %s\n",token.c_str());
			}
			lptr->pop_back();
		}else if(token=="pop_front"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				handle_error( "Unknown list register %s\n",token.c_str());
			}
			lptr->pop_front();
		}else if(token=="racc"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				handle_error( "Unknown list register %s\n",token.c_str());
			}
			token=read_token(f);
			Number* ptr=GetRegValue(token);
			if(ptr){
				int num=*ptr;
				if(num>=lptr->size()){
					handle_error("Index %d exceeded list size: %d\n",num,lptr->size());
				}
				ValueList::iterator iter=lptr->begin();
				while(num){
					num--;
					iter++;
				}
				token=read_token(f);
				if((*iter).isList){
					ValueList *nptr=GetListRegValue(token);
					if(!nptr){
						handle_error( "Unknown list register %s\n",token.c_str());
					}
					lptr=&((*iter).list);
					while(nptr->size())
						nptr->pop_front();
					iter=lptr->begin();
					while(iter!=lptr->end()){
						nptr->push_back(*iter);
						iter++;
					}
				}else{
					Number* ptr=GetRegValue(token);
					if(!ptr){
						handle_error( "Unknown numerical register %s\n",token.c_str());
					}
					*ptr=(*iter).num;
				}
			}else{
				handle_error( "Unknown numerical register %s\n",token.c_str());
			}
		}
#define MATH_OP(name,op) else if(token==name){ \
		token=read_token(f); \
		Number *a=GetRegValue(token); \
		if(!a){ \
			handle_error( "Unknown numerical register %s\n",token.c_str()); \
		} \
		token=read_token(f); \
		Number *b=GetRegValue(token); \
		if(!b){ \
			handle_error( "Unknown numerical register %s\n",token.c_str()); \
		} \
		token=read_token(f); \
		Number *c=GetRegValue(token); \
		if(!c){ \
			handle_error( "Unknown numerical register %s\n",token.c_str()); \
		} \
		*c=(*a op *b); \
		}
		
		MATH_OP("add", + )
		MATH_OP("minus", - )
		MATH_OP("div", / )
		MATH_OP("mult", * )
		MATH_OP("and", && )
		MATH_OP("or", || )
		MATH_OP("equal", == )
		MATH_OP("eq", == )
		MATH_OP("gt", > )
		MATH_OP("lt", < )
#undef MATH_OP
		else{
			handle_error("Commnad %s does not exist\n",token.c_str());
		}
	}
}