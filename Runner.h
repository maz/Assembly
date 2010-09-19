#ifndef RUNNER_H
#define RUNNER_H

#include "Assembly.h"
#include <stdio.h>
#include <map>
#include <list>
#include <string>

namespace Assembly {
	class Value;
	typedef std::list<Value> ValueList;
	class Value{
	public:
		Value(bool isl=false){num=0;isList=isl;}
		~Value(){}
		bool isList;
		Number num;
		ValueList list;
	};
	
	class Runner{
	public:
		Runner(bool boe=false){next_debug=false;break_on_error=boe;}
		virtual ~Runner(){}
		virtual void exec(FILE *f);
	private:
		Number* GetRegValue(std::string txt);
		ValueList* GetListRegValue(std::string txt);
		std::map<std::string,Value> Variables;
		std::map<std::string,int> Labels;
		Number Reg0,Reg1,Reg2,Reg3,Reg4,Reg5,Reg6,Return;
		ValueList Stack,Parameters,LReg0,LReg1,LReg2,LReg3;
		void do_jmp(FILE *f);
		void PrintValueList(ValueList *lptr,int indent=0);
		void lowercase(std::string &token);
		void debug(FILE *source,bool only_check=false);
		bool next_debug;
		bool break_on_error;
	};
}

#endif