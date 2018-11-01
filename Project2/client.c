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
		perror("fail to create pipe for server");
		exit(-1);
	}

	  // Set pipes to NONBLOCKING behaviour.
    if ( (fcntl(pipe_user_reading_from_server, F_SETFL, O_NONBLOCK) < 0) || (fcntl(pipe_user_writing_to_server, F_SETFL, O_NONBLOCK) < 0) ) {
          perror("ERROR: Failed to set NONBLOCKING behaviour on pipes for server: %s.\n", user_id);
          exit(-1);
    }

	// if server coonection sucessful, fork 2 process,
	//1 receiving the input from the server to display it on the screen,
	// the other taking the input from the user and send it to the server

	pid_t pid = fork();
	if(pid < 0 ) {
		//error happen
		perror("Fail to fork");
		exit(-1);
	} else if(pid == 0){//child, receiving the input from the server to display it on the screen
		//wait input for the server

		// Close unused pipes in CHILD process.
          if ( (close(pipe_user_reading_from_server[0]) < 0) || (close(pipe_user_writing_to_server[1]) < 0) )
          {
            perror("ERROR: Failed to close unused SERVER pipes on child procress.");
            exit(-1);
          }


		// read the data into a buffer
		int readStatus = read(pipe_user_reading_from_server, stdin, MAX_MSG);
		if(readStatus >= 0) {//if read return some data
			//send data to the child process ?
			if (write(pipe_user_writing_to_server, stdin, MAX_MSG) == -1) {
				perror("Fail to write to server");
				exit(-1);
			}
		}

//terminate child when somthing happen to the server

	}else{//parent
	//there're 2 ways that the process should end, either the server is being shutdown, or the user wants to leave by themself

		//fork another child
		pid_t s = fork();

		if(s==0){//child
			//handle user

		}else{
			//wait for any of those child
			//if any of them finish, signal the other to finish

		}
		// sleep(waitTime);
	}

	// Poll stdin (input from the terminal) and send it to server (child process) via pipe

	/* -------------- YOUR CODE ENDS HERE -----------------------------------*/

}

/*--------------------------End of main for the client --------------------------*/
