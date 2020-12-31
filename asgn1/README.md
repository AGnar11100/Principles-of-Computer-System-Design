README
CSE130 - Assgn1
adjnaran - Adrian Naranjo
afengine - Abbas Engineer


Files:
	README.md : This file contains explains the files and the commands used for this program 
	httpserver.cpp : This file contains the source code for this project. Both client and server side.
	Makefile : This file contains the compiling options.
	DESIGN.pdf : This pdf contains the design used to create this project and defines ways this project can be recreated. 


Getting Started

	In the command like run the following to _____ program:
		
		Run : 
			$ make 

		Clean: 
			$ make clean 

		Execute:
			$ ./httpserver HOST-OR-IP
			$ ./httpserver HOST-OR-IP CUSTOM-PORT

	These commands are strictly for TESTING, cd in seperate terminal where pwd is different that the one used in the above commands.

			TESTING PUT 
			curl -v -T tosend.txt http://127.0.0.1:8888/ --request-target ABCDEFG123
			note: must create file 'tosend.txt' in testing pwd so contents are sent. 

			TESTING GET
			curl -v http://127.0.0.1:8888/ --request-target ABCDEFG123
 


References used (Work Cited):

sockaddr_in :  https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
htons : https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-htons



