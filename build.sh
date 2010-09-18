#!/bin/sh
cd `dirname '$0'`
if /usr/bin/env flex assembly.l
	then
	if /usr/bin/env bison -d assembly.y
		then
		if /usr/bin/env gcc -g -o compile *.c
			then
			echo Compiler Build Successful
			if /usr/bin/env g++ -g -DDEBUG -o assembly -lreadline *.cpp
				then
				echo Runtime Build Successful
			fi
		fi
	fi
fi