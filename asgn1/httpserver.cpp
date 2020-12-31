/*
 *      This code was written and designed by:                  Class: Fall 2020 CSE 130-01 Princ. of Computer System Design
 *              Adriel Naranjo                                  Professor: Faisal Nawab
 *              Abbas Engineer                                  University of California, Santa Cruz 
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream> 
#include <ctype.h>
#include <string> 


#define BUFFER 4096
#define ERR -1


using namespace std;


// helper functions for checking the custom port number is valid
void check_alphabet(const char str[]){
	int ends = strlen(str); 
	int alpha_exists = 0; 
	for (int i = 0; i <= ends; i++) {
		if (isalpha(str[i])){ alpha_exists++; }
       	}
	if (alpha_exists > 0){
		perror("invalid port specified");
	}
}

// returns false if the resource name is invalid
// true if the resource name is valid
bool is_valid_resource_name(char file[]){

	int resource_length = strlen(file);
	int is_valid; 
	if(strchr((char*)&file[0] , '/')){
        	is_valid = 1;
	}
	else {	is_valid = 0; }


	//printf("Filename: %s\tResource name length: %d\t\n", file, resource_length); 

        for (int i = 0; i <= resource_length; i++) {
                if (isalpha(file[i]) || isdigit(file[i])) { is_valid++; }
        }
        if (is_valid != resource_length){
		//printf("is_valid:%d\tresource_length:%d\t\n", is_valid, resource_length);
		return false;
       	}
	else return true;
}


// helper functions for decided the type of request is being made
char get_request_type(char request[]){
	char garbage;
        //cout << "The request is acutally this: " << request << "\n";	
	strcmp(request, "GETHTTP/1.1") == 0? garbage = 'G' : garbage = 'P';
	//cout << " The request type is: " << garbage << "\n";
	return garbage; 	
}


int main(int argc , char *argv[]){



	// needed for initial set up for server-side 
	int root_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	const char *preferred_port;
        const char *hostname = argv[1];
        bool is_default_port;

	// needed for new connections and identifying request along w/ checking for bad requests
	int new_socket;
        const uint64_t buffer_size = BUFFER;
        char buffer[buffer_size];
        char request[3];
	char file[50]; 
        char protocol[10];

	int store;
	ssize_t fd, file_input, file_output; 

	// checking host server network attributes
        argc == 2? is_default_port = true : is_default_port = false;

	//fun lil' switch for simplicity
        switch (is_default_port){
           case 0:
                  preferred_port = argv[2];
                  check_alphabet(preferred_port);
                  break;

           default: preferred_port = "80";
                  break;
        }


	// very important for initializing our network attributes
        struct addrinfo *addrs, hints;
        getaddrinfo(hostname,preferred_port, &hints, &addrs);


        // create root_socket so we can make new connections
        root_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(root_socket == 0){
                perror("unable to build socket");
                exit(0);
        }

        // just followed the default socket options for root_socket
        int enable = 1;
        int sock_opt = setsockopt(root_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
        if(sock_opt < 0){
                perror("unable to set sock option");
                exit(0);
        }

	// cout << "The socket_opt: " << sock_opt << "\n";


        // sockaddr_in struct init 
        memset(address.sin_zero, '\0', sizeof(address.sin_zero));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        int root_network_address = int (atoi(preferred_port));
        address.sin_port = htons(root_network_address); // convert to network btye order
        
	
	// addrinfo struct init 
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        ///printf("%d", root_network_address);
        //printf("%d", root_socket);

	// bind
        if((bind(root_socket, (struct sockaddr *)&address, sizeof(address))) < 0){
                perror("unable to bind");
                exit(0);
        }
	
	// officially listening to new_connections
        if((listen(root_socket, 10)) < 0){
                perror("listen error");
                exit(0);
        }


	
        while(1){
		
		// new connection is made this is accepted
        	new_socket = accept(root_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen);  // Accept the socket for communication
		// cout << "The new socket is: " << new_socket << "\n";
        	if(new_socket < 0){
                	perror("accept error");
                	exit(0);
        	} 

		// see whats coming through the new_connection
        	read(new_socket, buffer, BUFFER);

       	 	// printf("The buffer shows this: %s \n", buffer);

		// extracts information from formatted message in buffer
        	sscanf(buffer, "%s %s %s", request, file, protocol);

		// trimming the name so fd can be found
		size_t size = strlen(file);
	       	if(strchr((char*)&file[0], '/') != NULL){	
			//const char *temp = file + 1;
			//strncpy(file, file - 1 , size);
			//printf("%s", file); 	
			char temp[10]; 
			sscanf(file, "/%s", temp); 
		      	printf("%s", temp);
			strncpy(file, temp, size); 	
	        }


		// strcpy(file, temp_name);  
		// printf("This is updated file from temp: %s\n", file); 	
		// printf("Strlen of file:%ld\n",size); 
		// makes sure our file is under 10 characters long
        	if(size > 11 || size - 1 == 1){
                	send(new_socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n", 47, 0);
                	// cout << "ERROR: >10 statement \n";
			close(new_socket);
			continue;
        	}

		// makes sure our file has a valid name: 0-9 a-z A-Z
        	if(!is_valid_resource_name(file)){
			send(new_socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n", 47, 0);
                	// cout << "ERROR: resource name is not valid \n ";
			close(new_socket);
			continue;
        	}

		// get the type of request
                const char request_type = get_request_type(request);

                if (request_type == 'G'){
                                  char buff[32768];
			          // stat manpage for more details, holds info about our file
                                  struct stat st; 
                                  stat(file, &st);
                                  store = st.st_size;
				  int flag = 0; 


				  // attempt to open fd, if permissions dont allows 403 else: 404. Either forbidden or not found
                                  fd = open(file, O_RDONLY, S_IRWXG);
                                  if(fd == ERR){
					  store = flag; 
                                        if(errno == EACCES){
                                                dprintf(new_socket, "HTTP/1.1 403 Forbidden\r\nContent-Length: %d\r\n\r\n", store);
                                                close(new_socket);
                                                continue;
                                        }
                                        else{
                                                dprintf(new_socket, "HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\n\r\n", store);
                                                close(new_socket);
                                                continue;
                                                }

                                        }

				  // fd opened succesfully
                                  if(fd != ERR){
                                        dprintf(new_socket, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", store);
                                        }

				  // read from fd read file contents from the file directory into the buffer
				  // Then write buffer contents to the socket to give the client the requested data 
                                  while((file_input = read(fd, buff, 32768)) != 0){
                                        file_output = write(new_socket, buff, file_input);
                                        if(file_output != file_input){
						// if some read/write error happens its bound to be caught.
                                                dprintf(new_socket, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\n", store);
                                                close(new_socket);
                                                continue;
                                                }
                                  }

                }
                else if(request_type == 'P'){

				// parse the head so we can get the length of the content
                                char *token = strtok(buffer, "\r\n");
                                int length = 0;
                                while(token != NULL){
                                        sscanf(token, "Content-Length: %d", &length);
                                        token = strtok(NULL, "\r\n");
                                }

                                char buf[32768];
				// open a file and give it all the permissions it needs to do the PUT. 
                                fd = open(file, O_WRONLY | O_TRUNC | O_CREAT, 00700);

				// if the file was unable to open throw a 403
                                if(fd == ERR){
                                        dprintf(new_socket, "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n");
                                        close(new_socket);
                                        continue;
                                }


                                ssize_t plus = 0;

				// write the contents until there is none left 
                                while((file_input = recv(new_socket, buf, 32768, 0)) != 0){
                                        plus += write(fd, buf, file_input);
                                        if(plus == length){
                                                break;
                                        }
                                }

				// extra case handling for server content
				// these will handle if the file was corrupted or if 
				// any other internal errors occured, this server will act accordingly
                                if(file_input == ERR){
                                        dprintf(new_socket, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
                                        close(new_socket);
                                        continue;
                                }

                                if(file_input != ERR){
                                        dprintf(new_socket, "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n");
                                        close(new_socket);
                                        continue;
                                }
                                close(fd);

                }// finally check for anything other messages that are other than PUT or GET. 
                else if (request_type != 'P' && request_type != 'G') {  
                      	send(new_socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n", 47, 0);
                        close(new_socket);
			continue; 

                }

        	//close(new_socket);

        }
}

