README
CSE130 - asgn3
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

        These commands are for TESTING:
        - cd in a separate terminal where the pwd is different from the one that is runnning the server.
        - Use commands such as the following to test special requests, or put them in a script to run for you
              follow that by chmod +x |ScriptName.sh|
              then use:
              ./|ScriptName.sh|

              For Backups: 
                  curl localhost:8080/b -v
              
              For Recovery: 
                  curl localhost:8080/r -v

              List of Backups:
                  curl localhost:8080/l -v
              
              Recovery Specific Backup:
                  curl localhost:8080/r/backup-[timestamp] -v  // timestamp does not actually have brackets
              
              Other cases to test for:
                  curl -T t1 localhost:8080/file1 -v 
                  curl -T t2 localhost:8080/file2 -v


References used (Work Cited):

sockaddr_in :  https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
htons : https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-htons


