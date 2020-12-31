#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

/*
 *	This code was written and designed by:			Class: Fall 2020 CSE 130-01 Princ. of Computer System Design
 *		Adriel Naranjo					Professor: Faisal Nawab
 *		Abbas Engineer 					University of California, Santa Cruz 
 */





#define STDIN 0
#define STDOUT 1

using namespace std; 


int main(int argc, char* argv[]) {

   /*		func1()
    *		Handles the case in which dog followed by file names are entered.
    *		If a file does not exist in the directory an error will be occur for that unknown file. 
    *		If the file does exist the contents of the file whether text or binary will be printed to standard output (1). 
    */


   // Checks if there are any arguments in cmd and makes sure argv[1] isnt a -
   if (argc > 1 && (strchr(argv[1] , '-')) == NULL){
	
	//get number of files entered after dog to STDOUT iteratively. 
	int number_of_files = argc - 1; 
	
	// go through each file
	for (int i = 1; i <= number_of_files; i++){

	    int fd;
	    //if file doesnt exist then throw an error and continue
	    if ((fd = open(argv[i], O_RDONLY)) == -1) { cout << "dog: "<< argv[i] <<": No such file or directory\n"; }
	
	    // if the file does exist
	    else{
		
		// int n to make sure there are file contents
		// and buffer of course to read into and write out of
	    	int n;
    		char buf[BUFSIZ];


    		//read and write using the system calls.

    		while ((n = read(fd, buf, BUFSIZ)) > 0)
        		if (write(STDOUT, buf, n) != n){ cout << "dog: write error\n"; }
		//close that file
		close(fd);

		//contin..
	    	}

	}

	return 0; 
   }




   /*		func2()
    *		Handles the case in which dog is entered either accompanied with or w/o a '-' symbol.
    *		The result of this allows the user of the GUI to enter text and print out text to standard output 
    *		being that no files were entered. 
    */  
   int ch;
   while ((ch = getchar()) != EOF || (strchr(argv[1] , '-')) != NULL)
      putchar(ch);



 
}
