#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <stdio.h>
#include <string>
#include <list>

namespace Assembly {
	std::string read_token(FILE *f);
	bool str_is_num(std::string str);
	
	typedef int Number;
	Number aton(const char* str);
	
	template<class T>
	class PointerIntern{
	public:
		PointerIntern(){count=1;value=NULL;}
		T *value;
		unsigned int count;
		~PointerIntern(){delete value;}
	};
	
	template<class T>
	class Pointer {
	public:
		Pointer(T *ptr=NULL){
			intern=new PointerIntern<T>();
			intern->value=ptr;
		}
		Pointer(const Pointer<T> &p){
			intern=p.intern;
			retain();
		}
		~Pointer(){
			release();
		}
		Pointer& operator=(const Pointer<T> &p){
			release();
			intern=p.intern;
			retain();
		}
		Pointer& operator=(T* p){
			release();
			intern=new PointerIntern<T>();
			intern->value=p;
		}
		T* operator->(){return intern->value;}
		T* operator*(){return *(intern->value);}
	private:
		void retain();
		void release(){
			if(intern){
				intern->count--;
				if(!intern->count)
					delete intern;
			}
			intern=NULL;
		}
		PointerIntern<T> *intern;
	};
}

#endif