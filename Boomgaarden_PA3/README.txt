CSPB 3753 FALL 2020
University of Colorado Boulder
Programming Assignment 3

By: Monica Boomgaarden

Files:	
	multi-lookup.c multi-lookup.h and Makefile were created by Monica Boomgaarden all other files were provided as course materials

Build: 
	make

Clean: 
	make clean

Run:
	./multi-lookup <num of parser threads> <num of converter threads> <file with domain names1> <file with domain names2> ....<file with domain names 10> <results file> <parser log>

Run and check for memory leaks: 
	valgrind ./multi-lookup <num of parser threads> <num of converter threads> <file with domain names1> <file with domain names2> ....<file with domain names 10> <results file> <parser log>



