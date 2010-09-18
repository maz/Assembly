#include "Runner.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

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
	else
		return NULL;
}

#define print_error(...) fprintf(stderr,"From: %s line %d, function %s, file pos %d: ",__FILE__,__LINE__,__FUNCTION__,ftell(f));fprintf(stderr,__VA_ARGS__)

void Runner::do_jmp(FILE *f){
	string token=read_token(f);
	if(token.substr(0, 3)=="reg"){
		fseek(f, *GetRegValue(token), SEEK_SET);
	}else if(Labels.count(token)){
		lowercase(token);
		fseek(f, Labels[token], SEEK_SET);
	}else{
		print_error( "Label %s does not exist\n");
		exit(1);
	}
}

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
	Label     Print        Mov_lr    Pop_Front    Racc    Eq     Gt
	Jmp       Exit         Mov_vr    Pop_Back     Add     Or     Lt
	Cndjmp    Mov          Size      Push         Minus   And
	Var       Mov_Const    Mov_rv    Not          Div     Mult
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
		string token=read_token(f);
		lowercase(token);
		if(token=="jmp"){
			do_jmp(f);
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
				print_error( "Unknown numerical register %s\n",token.c_str());
				exit(1);
			}
		}else if(token=="var"){
			string name=read_token(f);
			string type=read_token(f);
			lowercase(type);
			Variables[name]=Value((type=="list"));
		}else if(token=="label"){
			read_token(f);
		}else if(token=="exit"){
			kill(getpid(),SIGINT);
		}else if(token=="mov"){
			string str="";
			token=read_token(f);
			if(str_is_num(token)){
				str="mov_const "+token;
			}else if(Labels.count(token)){
				str="mov_lr "+token;
			}else if(GetRegValue(token)||GetListRegValue(token)){
				str="mov_rv "+token;
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
				print_error( "Unknown numerical register %s\n",token.c_str());
				exit(1);
			}
		}else if(token=="mov_lr"){
			token=read_token(f);
			if(Labels.count(token)){
				Number* ptr=GetRegValue(token);
				if(ptr){
					*ptr=Labels[token];
				}else{
					print_error( "Unknown numerical register %s\n",token.c_str());
					exit(1);
				}
			}else{
				print_error("The label %s does not exist\n",token.c_str());
				exit(1);
			}
		}else if(token=="mov_vr"){
			token=read_token(f);
			if(!Variables.count(token)){
				print_error( "Unknown variable %s\n",token.c_str());
				exit(1);
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
					print_error( "Unknown numerical register %s\n",token.c_str());
					exit(1);
				}
			}
		}else if(token=="size"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				print_error( "Unknown list register %s\n",token.c_str());
				exit(1);
			}
			token=read_token(f);
			Number* ptr=GetRegValue(token);
			if(ptr){
				*ptr=lptr->size();
			}else{
				print_error( "Unknown numerical register %s\n",token.c_str());
				exit(1);
			}
		}else if(token=="mov_rv"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			string name=read_token(f);
			if(!Variables.count(name)){
				print_error( "Unknown variable %s\n",name.c_str());
				exit(1);
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
					print_error( "Unknown numerical register %s\n",token.c_str());
					exit(1);
				}
			}
		}else if(token=="not"){
			token=read_token(f);
			Number *a=GetRegValue(token);
			if(!a){
				print_error( "Unknown numerical register %s\n",token.c_str());
				exit(1);
			}
			token=read_token(f);
			Number *b=GetRegValue(token);
			if(!b){
				print_error( "Unknown numerical register %s\n",token.c_str());
				exit(1);
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
					print_error( "Unknown numerical register %s\n",token.c_str());
					exit(1);
				}
				printf("%d\n",*ptr);
			}
		}else if(token=="push"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				print_error( "Unknown list register %s\n",token.c_str());
				exit(1);
			}
			token=read_token(f);
			ValueList *nptr=GetListRegValue(token);
			if(nptr){
				print_error("multi-dimentional lists not yet fully implemented\n");
				exit(1);
			}else{
				Number* ptr=GetRegValue(token);
				if(!ptr){
					print_error( "Unknown numerical register %s\n",token.c_str());
					exit(1);
				}
				Value val(false);
				val.num=*ptr;
				lptr->push_back(val);
			}
		}else if(token=="pop"||token=="pop_back"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				print_error( "Unknown list register %s\n",token.c_str());
				exit(1);
			}
			lptr->pop_back();
		}else if(token=="pop_front"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				print_error( "Unknown list register %s\n",token.c_str());
				exit(1);
			}
			lptr->pop_front();
		}else if(token=="racc"){
			token=read_token(f);
			ValueList *lptr=GetListRegValue(token);
			if(!lptr){
				print_error( "Unknown list register %s\n",token.c_str());
				exit(1);
			}
			token=read_token(f);
			Number* ptr=GetRegValue(token);
			if(ptr){
				int num=*ptr;
				if(num>=lptr->size()){
					print_error("Index %d exceeded list size: %d\n",num,lptr->size());
					exit(1);
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
						print_error( "Unknown list register %s\n",token.c_str());
						exit(1);
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
						print_error( "Unknown numerical register %s\n",token.c_str());
						exit(1);
					}
					*ptr=(*iter).num;
				}
			}else{
				print_error( "Unknown numerical register %s\n",token.c_str());
				exit(1);
			}
		}
#define MATH_OP(name,op) else if(token==name){ \
		token=read_token(f); \
		Number *a=GetRegValue(token); \
		if(!a){ \
			print_error( "Unknown numerical register %s\n",token.c_str()); \
			exit(1); \
		} \
		token=read_token(f); \
		Number *b=GetRegValue(token); \
		if(!b){ \
			print_error( "Unknown numerical register %s\n",token.c_str()); \
			exit(1); \
		} \
		token=read_token(f); \
		Number *c=GetRegValue(token); \
		if(!c){ \
			print_error( "Unknown numerical register %s\n",token.c_str()); \
			exit(1); \
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
			print_error("Commnad %s does not exist\n",token.c_str());
			exit(1);
		}
	}
}