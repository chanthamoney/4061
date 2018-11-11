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
#include <signal.h>

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

	int close1 = close(pipe_user_reading_from_server[1]);
	int close2 = close(pipe_user_writing_to_server[0]);
	if (close1 < 0 || close2 < 0) {
		perror("Fail to close unused user pipes for client");
	}

	print_prompt(argv[1]);
	char buf[MAX_MSG];
	char buf_err[MAX_MSG]; // copy of buf for error checking.

	// Set up SIGINT (CTRL+C) to our signal handler.
	// Because we CANNOT use globals for this Project, we decided to make the signal handler a nested function.
	auto void intHandler();
	void intHandler(int sig_num)
	{
		signal(SIGINT, intHandler);
		write(pipe_user_writing_to_server[1], "\\exit", MAX_MSG);
		printf("\n");
	}
	signal(SIGINT, intHandler);

	auto void segHandler();
	void segHandler(int sig_num)
	{
		signal(SIGSEGV, segHandler);
		write(pipe_user_writing_to_server[1], "\\exit", MAX_MSG);
		printf("You have been kicked out because of a segmentation fault.\n");
		exit(1);
	}
	signal(SIGSEGV, segHandler);

	/* -------------- YOUR CODE STARTS HERE -----------------------------------*/

	// poll pipe retrieved and print it to sdiout
	while (1) {
		// -------------------------------------------------------------------- //
		// -------------------------- POLL ON STDIN. -------------------------- //
		// -------------------------------------------------------------------- //

		// Poll stdin (input from the terminal) and send it to server (child process) via pipe
			memset(buf, 0, sizeof(buf));
			memset(buf_err, 0, sizeof(buf_err));
	    int stdinStatus = read(0, buf, MAX_MSG);
			strcpy(buf_err, buf);
	    	//printf("status of STDIN: %d\n", status);
	    if ( (stdinStatus < 0) && (errno == EAGAIN) )
	    {
				// No message to be read from STDIN. Pass.
	    }
	    else if (stdinStatus != 0)
	    {
				// Message received from STDIN.
				if (strcmp(strtok(buf_err, " "), "\n") == 0)
				{
					// USER sent an 'empty' message.
				}
				else if (strcmp(strtok(buf_err, " "), "\\seg\n") == 0)
				{
					char *n = NULL;
					*n = 1;
				}
				else if (write(pipe_user_writing_to_server[1], strtok(buf, "\n"), MAX_MSG) < 0)
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

			// FORMATTING BELOW TO LOOK NICE
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
