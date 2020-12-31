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
#include <ctype.h> 
#include <sys/stat.h>
#include <pthread.h>
#include <queue>
#include <vector>
#include <string>
#include <iostream>
#include <dirent.h>
#include <unordered_map>

#define ERR -1
#define BUFFER 4096


using namespace std;


pthread_mutex_t mutex;                               // queue for holding socket file descriptors
pthread_cond_t cond;
queue<int>myqueue;
bool is_redundant;

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
        else {  is_valid = 0; }


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


void * workerThread(void* pclient){
	int new_socket = 0;
	free(pclient);



	while(1){

		pthread_mutex_lock(&mutex);                                         // Put threads to sleep until a socket fd is pushed onto the queue
        	while(myqueue.empty() == 1){
                	pthread_cond_wait(&cond, &mutex);
        	}
        	new_socket = myqueue.front();                                       // Get the oldest socket fd off the queue to use as new_socket
       	 	myqueue.pop();
        	pthread_mutex_unlock(&mutex);

		const uint64_t buffer_size = BUFFER;
        	char buffer[buffer_size];
		char request[3];
		char file[50];
        	char protocol[10];
		int store;
		ssize_t fd, file_input, file_output;

		read(new_socket, buffer, BUFFER);
       		// printf("The buffer shows this: %s \n", buffer);


		// extracts information from formatted message in buffer
        	sscanf(buffer, "%s %s %s", request, file, protocol);

		// trimming the name so fd can be found
		size_t size = strlen(file);
		if(strchr((char*)&file[0], '/') != NULL){	
			//printf("%s", file); 	
			char temp[10]; 
			sscanf(file, "/%s", temp); 
			printf("%s", temp);
			strncpy(file, temp, size);
		}
		// printf("This is updated file from temp: %s\n", file); 	
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
                else if(request_type == 'P' && is_redundant == false){

				// parse the head so we can get the length of the content
				/*	
                                char *token = strtok(buffer, "\r\n");
                                int length = 0;
                                while(token != NULL){
                                        sscanf(token, "Content-Length: %d", &length);
                                        token = strtok(NULL, "\r\n");
                                }*/

				// thread safe strtok_r: parse the head so we can get the length of the content
				char*token;
				int length = 0;
				char*request_buffer = buffer;
				while((token = strtok_r(request_buffer,"\r\n", &request_buffer))){
					sscanf(token, "Content-Length: %d", &length);
					token = strtok_r(NULL, "\r\n", &request_buffer);
				
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

				//close(fd);

                }// finally check for anything other messages that are other than PUT or GET.
                else if (request_type != 'P' && request_type != 'G' && is_redundant == false) {
                      	send(new_socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n", 47, 0);
                        close(new_socket);
			continue;

                }

		else if (request_type == 'P' && is_redundant == true){
			// When a PUT request is received, the httpserver program would write to all three copies of the file.
			// This is a really cheap way of doing redundancy.. "ew, but it works tho? ;)"
			char buf[32768];

			ssize_t fd1, fd2, fd3;

			int received = recv(new_socket, buf, 32768, 0);
			
			if (received == ERR){
				perror("couldnt receive message from socket");
			}

			string filepath1 = "./copy1/" + string(file);
			string filepath2 = "./copy2/" + string(file);
			string filepath3 = "./copy3/" + string(file);

		
			fd1 = open(filepath1.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 00700);
                        if(fd1 == ERR){
				dprintf(new_socket, "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n");
                                close(new_socket);
                                continue;
			}
			ssize_t check1 = write(fd1, buf, 32768);
			close(fd1);

			fd2 = open(filepath2.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 00700);
                        if(fd2 == ERR){
                                dprintf(new_socket, "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n");
                                close(new_socket);
                                continue;
                        }
                        ssize_t check2 = write(fd2, buf, 32768);
                        close(fd2);

			fd3 = open(filepath3.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 00700);
                        if(fd3 == ERR){
                                dprintf(new_socket, "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n");
                                close(new_socket);
                                continue;
                        }
                        ssize_t check3 = write(fd3, buf, 32768);
                        close(fd3);
			
			if(check1 == ERR && check2 == ERR && check3 == ERR){
				dprintf(new_socket, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
                                close(new_socket);
				continue;
			}
                        if(check1 != ERR && check2 != ERR && check3 != ERR){
				dprintf(new_socket, "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n");
				close(new_socket);
				continue;
			}

		}
		// else if (request_type == "G" && is_redundant == true){
		// When a GET request is received, the httpserver first checks if the three files are identical. If
		// they are, then the content is returned. If two of them are identical but the third is different, then
		// one of the two identical copies is returned. If all three copies are different from each other, then an error message (error code 500) is returned.
		// }
		//
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);
		// pthread_mutex_lock(&mutex);
        	close(new_socket);
		return NULL;

	}
}



int main(int argc, char*argv[]){
	
	
	int opt, num_of_threads;
	int flag = 0;
	// Check to see if -N and/or -r is in the command line options
	// getopt permutes through the command line options (argv)
	// colon ensures we get N  
	while((opt = getopt(argc, argv, "N:r")) != -1){ 
		switch(opt){

			// if N is in the command line then we attempt to set num_of_threads to the preceding argument
			// if an int N is not found then we will flag so default num_of_threads is invoked
			case 'N':
				flag = 1;
				num_of_threads = int(atoi(optarg));
				break;
			// checks if there is redundancy
			case 'r':
				is_redundant = true;
				break;
		}

	}


	// optind is like an alias to argc but handles the parsing
	// get host from argv
	char const * host = argv[optind];
	// get the port
	char const * port = argv[optind + 1];

	// if optind finds no port to bind with host, set the default to 80
	if(port == NULL){
		port = "80";
	}
	
	// if number of threads doesnt precede N, then set to default num_of_threads
	if(flag == 0){
		num_of_threads = 4;
	}


	//printf("Number of threads: %d\tIs redundant: %d\tPort Number: %s\tHost: %s", num_of_threads, is_redundant, port, host);

	struct addrinfo *addrs, hints;

	getaddrinfo(host, port, &hints, &addrs);
	int main_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(main_socket == 0){
		perror("unable to build socket");
		exit(0);
	}

	int enable = 1;
	int sock_opt = setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

	if(sock_opt < 0){
		perror("unable to set sock option");
		exit(0);
	}

	struct sockaddr_in address;

	int p = int (atoi(port));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(p);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;


	if((bind(main_socket, (struct sockaddr *)&address, sizeof(address))) < 0){
		perror("unable to bind");
		exit(0);
	}

	if((listen(main_socket, num_of_threads)) < 0){
		perror("listen error");
		exit(0);
	}

	vector<pthread_t>dispatcherThread(num_of_threads);

	for (int i = 0; i < num_of_threads; i++){
		pthread_create(&dispatcherThread[i], NULL, &workerThread, NULL);
	}

	while(true){

		int addrlen = sizeof(address);
		int new_socket = accept(main_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen);

		if(new_socket < 0){
			perror("accept error");
			exit(0);
		}

		pthread_mutex_lock(&mutex);
		myqueue.push(new_socket);
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);

	}

}
