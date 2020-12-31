README
CSE130 - Assgn0 
adjnaran - Adriel Naranjo
afengine - Abbas Engineer

Description: Implement the basic cat program - the code wil copy data from eachof the files and render it to the standard ouput.
If '-' is given as a filename 'dog' will render it to standard output. 
If no file is specified, render standard input to standard output. 

1) cd into the correct folder assignment 

2) to compile:
	
	make
	./dog   // renders standard input to standard output 
	./dog - // ^       ^        ^     ^  ^        ^
	./dog file1 file2 file3 //reads from each file and renders to standard output one at a time, should skip over files that cause errors. 
