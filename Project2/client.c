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
#include "util.h"

/* -------------------------Main function for the client ----------------------*/
void main(int argc, char * argv[]) {

	char * YOUR_UNIQUE_ID = "CSCI_39";
	int waitTime = 1000;
	int pipe_user_reading_from_server[2], pipe_user_writing_to_server[2];

	// You will need to get user name as a parameter, argv[1].
	if(connect_to_server(YOUR_UNIQUE_ID, argv[1], pipe_user_reading_from_server, pipe_user_writing_to_server) == -1) {
		exit(-1);
	}

	// Set pipes to NONBLOCKING behaviour.
	fcntl(pipe_user_reading_from_server[0], F_SETFL, fcntl(pipe_user_reading_from_server[0], F_GETFL)| O_NONBLOCK);
	fcntl(pipe_user_reading_from_server[1], F_SETFL, fcntl(pipe_user_reading_from_server[1], F_GETFL)| O_NONBLOCK);
	fcntl(pipe_user_writing_to_server[0], F_SETFL, fcntl(pipe_user_writing_to_server[0], F_GETFL)| O_NONBLOCK);
	fcntl(pipe_user_writing_to_server[1], F_SETFL, fcntl(pipe_user_writing_to_server[1], F_GETFL)| O_NONBLOCK);
	fcntl(0, F_SETFL, fcntl(0, F_GETFL)| O_NONBLOCK);

	print_prompt(argv[1]);

	/* -------------- YOUR CODE STARTS HERE -----------------------------------*/

	// poll pipe retrieved and print it to sdiout
	while (1) {
			// -------------------------------------------------------------------- //
		  // -------------------------- POLL ON STDIN. -------------------------- //
		  // -------------------------------------------------------------------- //
			char stdinBuffer[MAX_MSG];
			memset(stdinBuffer, 0, sizeof(stdinBuffer));
	    int stdinStatus = read(0, stdinBuffer, MAX_MSG);
	    //printf("status of STDIN: %d\n", status);
	    if ( (stdinStatus < 0) && (errno == EAGAIN) )
	    {
	      // No message to be read from STDIN. Pass.
	    }
	    else if (stdinStatus != 0)
	    {
	      // Message received from STDIN.
				if (write(pipe_user_writing_to_server[1], stdinBuffer, MAX_MSG) < 0)
				{
					printf("ERROR: Failed to write to CHILD process of SERVER.\n");
				}
				print_prompt(argv[1]);
	    }
	    else
	    {
	      // ERROR occured.
	      printf("ERROR: Failed to read from STDIN.\n");
	    }

			// ------------------------------------------------------------------------------------------ //
			// -------------------------- POLL ON CHILD PROCESS OF THE SERVER. -------------------------- //
			// ------------------------------------------------------------------------------------------ //
			char childServerBuffer[MAX_MSG];
			memset(childServerBuffer, 0, sizeof(childServerBuffer));
			int childServerStatus = read(pipe_user_reading_from_server[0], childServerBuffer, MAX_MSG);
		//	printf("READ END: %d\n", pipe_user_reading_from_server[0]);
			//printf("status of STDIN: %d\n", status);
			if ( (childServerStatus < 0) && (errno == EAGAIN) )
			{
				// No message to be read from STDIN. Pass.
			}
			else if (childServerStatus != 0)
			{
				// Message received from STDIN.

				// FORMATTING TO LOOK NICE.
				for (int i = 0; i < strlen(argv[1])+5; i++)
				{
					printf("\b"); // Removes and replaces the user prompt with message from server.
				}
				printf("%s", childServerBuffer);
				print_prompt(argv[1]); // Prints user prompt to screen again.
			}
			else
			{
				// ERROR occured.
				printf("ERROR: Failed to read from STDIN.\n");
			}

		// if server coonection sucessful, fork 2 process,
		//1 receiving the input from the server to display it on the screen,
		// the other taking the input from the user and send it to the server
		// Poll stdin (input from the terminal) and send it to server (child process) via pipe

		/* -------------- YOUR CODE ENDS HERE -----------------------------------*/
		usleep(waitTime);
	}
}

/*--------------------------End of main for the client --------------------------*/
