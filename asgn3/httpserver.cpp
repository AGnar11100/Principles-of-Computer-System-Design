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
#include <errno.h>
#include <sys/stat.h>
#include <iostream> 
#include <ctype.h>
#include <string> 
#include <queue>
#include <vector>
#include <dirent.h>
#include <climits>
#include <ctime>
#include <sys/dir.h>
#include <stack>
#include <cstring> 

#define BUFFER 30000
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

	// holds backup file string values
	vector<string>recovery_files;

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
		string buffer_string = string(buffer);

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
		      	// printf("%s", temp);
			strncpy(file, temp, size); 	
	        }


		// one of the the many checks for a backup
	        int position = buffer_string.find("backup-");
		bool is_spec_recovery = false;
	        if (position != -1){
			is_spec_recovery = true;
	        }


		// parse the buffer for the backup name;
	        string backup_name;
	        for(int i = position; i < position + 17; i++){
	                backup_name += buffer[i];
	        }


		//create path for backup file
		string full_backup = "./" + string(backup_name);
	        // printf("Backup_name is: %s\n", full_backup.c_str());



		// will be used to see if file is normal or \b \r \l
		string filestringname = string(file);
		// printf("Filestringname: %s\n", filestringname.c_str());



		//buffer for reading and writing for \b request
		const uint64_t b_buffer_size = BUFFER;
	        char b_buffer[b_buffer_size];
		if(filestringname == "b"){

			// get the current time seconds since 1970 date
			int new_dir;
        		time_t current_time;
        		time(&current_time);


			// give the backup file a path to be reference at
        		string time_as_str = to_string(current_time);
        		string directory_path = "./backup-" + time_as_str;
        		// printf("%s\n", directory_path.c_str());


			// create the directory and give it all permissions
        		new_dir = mkdir(directory_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        		if(new_dir == -1){
                		perror("directory could not be created");
        		}


			// save the backup file names in the vector recovery_files
			recovery_files.push_back(directory_path);


			// needed for parsing through server directory
        		DIR *directory = opendir((char*)".");
        		struct dirent *direntStruct;


			// checks if main directory we are passing through has any files or other directories
        		if (directory != NULL) {
				// while there are directories or files in the main directory
                		while ((direntStruct = readdir(directory))) {

			
					// get the name of the file, the length of the name of the file
					string name = string(direntStruct->d_name);
                        		size_t name_length = name.length();
					// to see if the file is a backup, we dont want backups
                        		int found_position = name.find("backup");

			
					// sanity check so we only get files that are assets to the server
                        		if(name != "DESIGN.pdf" && name != "README.md" 
							&& name != "Makefile" 
							&& name != "httpserver.cpp" && name != "httpserver" 
							&& name != ".." && name != "." 
							&&  name_length < 10 && found_position == -1){
						// printf("File Name: %s\n", name.c_str());

                        
						// create path for backup to pass to destination
                                		string destination = directory_path + "/" + name;
                                		// open source, open dest, read from server source to buf, then buff to backup.
						// continue for rest of files that are assets
                                		int src_fd = open(direntStruct->d_name, O_RDONLY);
                                		int dst_fd = open(destination.c_str(), O_CREAT | O_WRONLY, 00700);

                                		while (1) {
                                        		int err = read(src_fd, b_buffer, buffer_size);
                                        		if (err == -1) {
                                                		perror("Error reading file.\n");
                                                		continue;
                                        		}
                                        		int n = err;

                                        		if (n == 0) break;

                                        		err = write(dst_fd, b_buffer, n);
                                        		if (err == -1) {
                                                		perror("Error writing to file.\n");
                                                		continue;
                                        		}
                                		}
                                		close(src_fd);
                                		close(dst_fd);
                        		}
                		}
        		}
			dprintf(new_socket, "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n");
                        close(new_socket);
        		closedir(directory);
			continue;

		}



		// simply appends all backup names to a string then writes the string out as the response
		string list_of_recoveries;
		if(filestringname == "l"){
			// checks if there are recoveries, if not then not found
			if(recovery_files.size() == 0){
                                dprintf(new_socket, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
                                close(new_socket);
                                continue;
                        }
			for(unsigned int i = 0; i < recovery_files.size(); i++){
				string a_backup = recovery_files.at(i);
				list_of_recoveries += a_backup + "\n";
                	}
			dprintf(new_socket, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", list_of_recoveries.length());
			write(new_socket, list_of_recoveries.c_str(), list_of_recoveries.length());
			close(new_socket);
			continue;
		}



		// --------------------- from here down is all for recovery ------------------------------------------

		// simple check to see if the backup file requested for recover is an actual backup, if so return index
		int target_index;
		bool target_found = false;
		for(unsigned int i = 0; i < recovery_files.size(); i++){
			if((int)recovery_files.at(i).find(full_backup) != -1){
				target_index = (int)i;
				target_found = true;
				// printf("Target index is: %d\n", i);
				break;
			}
			// printf("target_index not found\n");
		}


		// if we didnt find a target backup file for recovery but requested one, not found.
		if(target_found == false && is_spec_recovery == true){
			dprintf(new_socket, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
			close(new_socket);
			continue;
		}

		// buffer for recover read n write
		const uint64_t r_buffer_size = BUFFER;
                char r_buffer[r_buffer_size];
		if(filestringname == "r" || target_found == true){
			// checks if there are recoveries, if not then not found
			if(recovery_files.size() == 0){
				dprintf(new_socket, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
				close(new_socket);
                                continue;
			}
			// int size_of_rec = recovery_files.size();
			// printf("Size of recovery bank:%d\n", size_of_rec);
		

			string str;
			// sets recovery file to most recent if not specified
			if(is_spec_recovery == false){
				str = recovery_files.back();
			}
			// sets recovery file to specified backup name if the backup exists and it was requested
			else if(is_spec_recovery == true && target_found == true){
				str = recovery_files.at(target_index); 
			}


			// needed for parsing through server directory
			DIR *directory = opendir(str.c_str());
        		struct dirent *direntStruct;


			// checks if main directory we are passing through has any files or other directories
        		if (directory != NULL) {

				// while there are directories or files in the main directory
				while ((direntStruct = readdir(directory))) {


					// get the name of the file, the length of the name of the file
					string target_name = string(direntStruct->d_name);
                        		int name_length = target_name.length();
                        		int found_position = target_name.find("backup");


					// sanity check so we only get files that are assets to the server
                        		if(target_name != "DESIGN.pdf" && target_name != "README.md" 
							&& target_name != "Makefile" && target_name != "httpserver.cpp" 
							&& target_name != "httpserver" && target_name != ".." 
							&& target_name != "." 
							&&  name_length < 10 && found_position == -1){
						// printf("File Name: %s\n", target_name.c_str());
                                		// create path for backup to pass to destination
                                		string destination = "./" + target_name;
                                		string source = string(str) + "/" + target_name;
                                		

						// open source, open dest, read from backup source to buf, then buff to server file.
                                                // continue for rest of files that are assets
                                		int src_fd = open(source.c_str(), O_RDONLY);
                                		int dst_fd = open(destination.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 00700);
	
                                		while (1) {
                                        		int err = read(src_fd, r_buffer, buffer_size);
                                        		if (err == -1) {
                                                		perror("Error reading file.\n");
                                                		continue;
                                        		}
                                        		int n = err;

                                        		if (n == 0) break;

                                        		err = write(dst_fd, r_buffer, n);
                                        		if (err == -1) {
                                                		perror("Error writing to file.\n");
                                                		continue;
                                        		}
                                		}
                                		close(src_fd);
                                		close(dst_fd);

                        			}
                		}

        		}
			closedir(directory);
			dprintf(new_socket, "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n");
                        close(new_socket);
			continue;
		}
	
		
	

		// if none of the special requests \b \r \l are requested, handle filename like norma;
		if(filestringname != "b" && filestringname != "r" && filestringname != "l"){
			// printf("File: %s\tFilenamestring: %s\n", file, filestringname.c_str());
		
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
                		else if (request_type != 'P' && request_type != 'G' && filestringname != "b" && filestringname != "r" && filestringname != "l") {  
                      			send(new_socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n", 47, 0);
                        		close(new_socket);
					continue; 

                		}

        			close(new_socket);
		}// if file is a normal file
        }//while
}//main

