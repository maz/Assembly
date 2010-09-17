#!/bin/sh
if /usr/bin/env g++ -g -DDEBUG -o assembly *.cpp
	then
	echo Build Successful
fi