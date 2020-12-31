README
CSE130 - asgn2
adjnaran - Adrian Naranjo
afengine - Abbas Engineer


Files:
	README.md : This file contains explains the files and the commands used for this program 
	httpserver.cpp : This file contains the source code for this project.
	Makefile : This file contains the compiling options.
	DESIGN.pdf : This pdf contains the design used to create this project and defines ways this project can be recreated. 


Getting Started
	chmod +x httpserver.cpp

	In the command like run the following to _____ program:
		Clean :
			make clean

		Run : 
			make

		Execute:
			$ ./httpserver localhost: 8080 or HOST-OR-IP CUSTOM-PORT
			$ ./httpserver localhost: 8080 or HOST-OR-IP CUSTOM-PORT -N THREAD-COUNT    // to speicfy the thread count
			$ ./httpserver localhost: 8080 or HOST-OR-IP CUSTOM-PORT -N THREAD-COUNT -r // for redundancy
	These commands are for TESTING:
	- cd in a separate terminal where the pwd is different from the one that is runnning the server. 
	- Use a shell script that runs the client simultaneously - use "&" at the end of the command to do so
	  this will ensure they run at the same time.
	  
	curl -T t1 http://localhost8080 --request-target FILENAME00 > cmd1.output &
	curl -T t1 http://localhost8080 --request-target FILENAME00 > cmd1.output	
	
	follow that by chmod +x |ScriptName.sh|
	then use:
	./|ScriptName.sh|

References used (Work Cited):

sockaddr_in :  https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
htons : https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-htons

Issues with redundancy: There are currently errors with redundancy, its implementation has
alot of repetative code that could have been efficiently modularized, however due to the time
constraints this was the result. 
