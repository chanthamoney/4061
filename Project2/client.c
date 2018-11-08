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

	if (argc < 2)
	{
		printf("Missing username\n");
		exit(-1);
	}

	char * YOUR_UNIQUE_ID = "CSCI4061_39";
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

	close(pipe_user_reading_from_server[1]);
	close(pipe_user_writing_to_server[0]);

	print_prompt(argv[1]);
	char buf[MAX_MSG];

	/* -------------- YOUR CODE STARTS HERE -----------------------------------*/

	// poll pipe retrieved and print it to sdiout
	while (1) {
		// -------------------------------------------------------------------- //
		// -------------------------- POLL ON STDIN. -------------------------- //
		// -------------------------------------------------------------------- //

		// Poll stdin (input from the terminal) and send it to server (child process) via pipe
		memset(buf, 0, sizeof(buf));
	    int stdinStatus = read(0, buf, MAX_MSG);
	    	//printf("status of STDIN: %d\n", status);
	    if ( (stdinStatus < 0) && (errno == EAGAIN) )
	    {
				// No message to be read from STDIN. Pass.
	    }
	    else if (stdinStatus != 0)
	    {
				// Message received from STDIN.
				if (write(pipe_user_writing_to_server[1], strtok(buf, "\n"), MAX_MSG) < 0)
				{
					printf("ERROR: Failed to write to CHILD process of SERVER.\n");
					exit(-1);
				}
				print_prompt(argv[1]);
	    }
	    else
	    {
				// ERROR occured.
				printf("ERROR: Failed to read from STDIN.\n");
				exit(-1);
	    }


		// ------------------------------------------------------------------------------------------ //
		// -------------------------- POLL ON CHILD PROCESS OF THE SERVER. -------------------------- //
		// ------------------------------------------------------------------------------------------ //

		memset(buf, 0, sizeof(buf));
		int childServerStatus = read(pipe_user_reading_from_server[0], buf, MAX_MSG);
		//printf("READ END: %d\n", pipe_user_reading_from_server[0]);
		//printf("status of STDIN: %d\n", status);
		if ( (childServerStatus < 0) && (errno == EAGAIN) )
		{
			// No message to be read from STDIN. Pass.
		}
		else if (childServerStatus == 0)
		{
			// Pipe closed! Terminate USER!
			for (int i = 0; i < strlen(argv[1])+5; i++)
			{
				printf("\b"); // Removes and replaces the user prompt with message from server.
			}
			exit(0);
		}
		else if (childServerStatus != 0)
		{
			// Message received from STDIN.

			// FORMATTING TO LOOK NICE.
			// BEWARE YOUR EYES. THIS IS BRUTE FORCE CODING.
			int length = strlen(argv[1])+5;
			for (int i = 0; i < length; i++)
			{
				printf("\b");
			}
			for (int i = 0; i < length; i++)
			{
				printf(" ");
			}
			for (int i = 0; i < length; i++)
			{
				printf("\b");
			}
			// END OF FORMATTING.

			printf("%s\n", buf); // Print message.
			print_prompt(argv[1]); // Prints user prompt to screen again.
		}
		else
		{
			// ERROR occured.
			printf("ERROR: Failed to read from STDIN.\n");
			exit(-1);
		}

		/* -------------- YOUR CODE ENDS HERE -----------------------------------*/
		usleep(waitTime);
	}
}

/*--------------------------End of main for the client --------------------------*/
