#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "comm.h"

/* -------------------------Main function for the client ----------------------*/
void main(int argc, char * argv[]) {

	int pipe_user_reading_from_server[2], pipe_user_writing_to_server[2];

	// You will need to get user name as a parameter, argv[1].

	if(connect_to_server("YOUR_UNIQUE_ID", argv[1], pipe_user_reading_from_server, pipe_user_writing_to_server) == -1) {
		exit(-1);
	}

	/* -------------- YOUR CODE STARTS HERE -----------------------------------*/

	
	// poll pipe retrieved and print it to sdiout
	if(pipe(pipe_user_reading_from_server) < 0 || pipe(pipe_user_writing_to_server) < 0) {
		perror("fail to create pipe for client");
		exit(-1);
	}
		if(pipe(pipe_user_reading_from_server) < 0 || pipe(pipe_user_writing_to_server) < 0) {
		perror("fail to create pipe for client");
		exit(-1);
	}
	
	// poll pipe retrieved and print it to sdiout
	pid_t server = fork();
	if(server==0){//child, 
		//wait input for the server

//terminate child when somthing happen to the server 

	}else{//parent
	//fork another child 
		pid_t s = fork();

		if(s==0){//child
			//handle user
			
		}else{
			//wait for any of those child
			//if any of them finish, signal the other to finish 
		}


	//wait for child to finish
	//signal to finish other child 
	//quit
	}

	// Poll stdin (input from the terminal) and send it to server (child process) via pipe

		//
	/* -------------- YOUR CODE ENDS HERE -----------------------------------*/
	
}

/*--------------------------End of main for the client --------------------------*/


