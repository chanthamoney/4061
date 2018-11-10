#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "comm.h"
#include "util.h"

/* -----------Functions that implement server functionality -------------------------*/

/*
 * Returns the empty slot on success, or -1 on failure
 */
int find_empty_slot(USER * user_list) {
	// iterate through the user_list and check m_status to see if any slot is EMPTY
	// return the index of the empty slot
    int i = 0;
	for(i=0;i<MAX_USER;i++) {
    	if(user_list[i].m_status == SLOT_EMPTY) {
			return i;
		}
	}
	return -1;
}

/*
 * list the existing users on the server shell
 */
int list_users(int idx, USER * user_list)
{
	// iterate through the user list
	// if you find any slot which is not empty, print that m_user_id
	// if every slot is empty, print "<no users>""
	// If the function is called by the server (that is, idx is -1), then printf the list
	// If the function is called by the user, then send the list to the user using write() and passing m_fd_to_user
	// return 0 on success
	int i, flag = 0;
	char buf[MAX_MSG] = {}, *s = NULL;

	/* construct a list of user names */
	s = buf;
	strncpy(s, "---connecetd user list---\n", strlen("---connecetd user list---\n"));
	s += strlen("---connecetd user list---\n");
	for (i = 0; i < MAX_USER; i++) {
		if (user_list[i].m_status == SLOT_EMPTY)
			continue;
		flag = 1;
		strncpy(s, user_list[i].m_user_id, strlen(user_list[i].m_user_id));
		s = s + strlen(user_list[i].m_user_id);
		strncpy(s, "\n", 1);
		s++;
	}
	if (flag == 0) {
		strcpy(buf, "<no users>\n");
	} else {
		s--;
		strncpy(s, "\0", 1);
	}

	if(idx < 0) {
		printf("%s", buf);
		printf("\n");
	} else {
		/* write to the given pipe fd */
		if (write(user_list[idx].m_fd_to_user, buf, strlen(buf) + 1) < 0)
			perror("Fail to writing to server shell");
	}

	return 0;
}

/*
 * broadcast message to all users
 */
int broadcast_msg(USER * user_list, char *buf, char *sender)
{
	//iterate over the user_list and if a slot is full, and the user is not the sender itself,
	//then send the message to that user
	//return zero on success
	char message[MAX_MSG];
	memset(message, 0, sizeof(message));
	if ((strcmp(sender, "")) == 0)
	{
    // SERVER is broadcasting message to all users.
    strcpy(message, "ADMIN-NOTICE: ");
		strcat(message, buf);
	}
  else if ((strcmp(sender, "\n")) == 0)
  {
    // SERVER system notices (NOT SERVER BROADCASTING).
    strcpy(message, buf);
  }
	else
	{
    // Default: USER is sending message to all other users.
    strcpy(message, sender);
		strcat(message, ": ");
		strcat(message, buf);
	}

	for (int i = 0; i<MAX_USER; i++)
	{
		if (user_list[i].m_status != SLOT_EMPTY && user_list[i].m_user_id != sender)
		{
			if (write(user_list[i].m_fd_to_user, message, MAX_MSG) < 0)
			{
				return -1;
			}
		}
	}

	return 0;
}

/*
 * add a new user
 */
int add_user(int idx, USER * user_list, int pid, char * user_id, int pipe_to_child, int pipe_to_parent)
{
	// populate the user_list structure with the arguments passed to this function
	// return the index of user added
	USER newUser = {};
	newUser.m_pid = pid;
	strcpy(newUser.m_user_id, user_id);
	newUser.m_fd_to_user = pipe_to_child;
	newUser.m_fd_to_server = pipe_to_parent;
	newUser.m_status = SLOT_FULL;
	user_list[idx] = newUser;

	return idx;
}

/*
 * Kill a user
 */
void kill_user(int idx, USER * user_list) {
	// kill a user (specified by idx) by using the systemcall kill()
	// then call waitpid on the user
	// kill a user (specified by idx) by using the systemcall kill()

	int ret = kill(user_list[idx].m_pid, SIGTERM);
	if (ret == -1)
	{
		perror("CRITICAL ERROR: CANNOT KICK USER\n");
	}
	// then call waitpid on the user
	int status;
	waitpid(user_list[idx].m_pid, &status, WNOHANG);
}

/*
 * Perform cleanup actions after the used has been killed
 */
void cleanup_user(int idx, USER * user_list)
{
	// m_pid should be set back to -1
	user_list[idx].m_pid = -1;
	// m_user_id should be set to zero, using memset()
	memset(user_list[idx].m_user_id, 0, sizeof(user_list[idx].m_user_id));
	// close all the fd
	int close1 = close(user_list[idx].m_fd_to_user);
	int close2 = close(user_list[idx].m_fd_to_server);
  if(close1 < 0 || close2 < 0) {
    perror("Fail to close on cleanup_user ");
  }
	// set the value of all fd back to -1
	user_list[idx].m_fd_to_user = -1;
	user_list[idx].m_fd_to_server = -1;
	// set the status back to empty
	user_list[idx].m_status = SLOT_EMPTY;
}

/*
 * Kills the user and performs cleanup
 */
void kick_user(int idx, USER * user_list) {
  if (idx < 0)
  {
    printf("Cannot find user\n");
  }
  else
  {
    // Notify the SERVER and all users of the user that has been kicked/exited.
    // Create buffer to send.
    char kickMSG[MAX_MSG];
    strcpy(kickMSG, user_list[idx].m_user_id);
    strcat(kickMSG, " has left the server");

    // Print and broastcast message.
    // FORMATTING TO LOOK NICE.
    // BEWARE YOUR EYES. THIS IS BRUTE FORCE CODING.
    for (int i = 0; i < 10; i++)
    {
      printf("\b");
    }
    for (int i = 0; i < 10; i++)
    {
      printf(" ");
    }
    for (int i = 0; i < 10; i++)
    {
      printf("\b");
    }
    // END OF FORMATTING.

    printf("%s\n", kickMSG);
    broadcast_msg(user_list, kickMSG, "\n");

    // should kill_user()
    kill_user(idx, user_list);
    // then perform cleanup_user()
    cleanup_user(idx, user_list);
  }
}

/*
 * Cleanup user chat boxes
 */
void cleanup_users(USER * user_list)
{
	// go over the user list and check for any empty slots
	// call cleanup user for each of those users.
	for (int i = 0; i < MAX_USER; i++)
	{
		if (user_list[i].m_status == SLOT_EMPTY)
		{
			cleanup_user(i, user_list);
		}
	}
}

/*
 * find user index for given user name
 */
int find_user_index(USER * user_list, char * user_id)
{
	// go over the  user list to return the index of the user which matches the argument user_id
	// return -1 if not found
	int i, user_idx = -1;

	if (user_id == NULL) {
		fprintf(stderr, "NULL name passed.\n");
		return user_idx;
	}
	for (i=0;i<MAX_USER;i++) {
		if (user_list[i].m_status == SLOT_EMPTY)
			continue;
		if (strcmp(user_list[i].m_user_id, user_id) == 0) {
			return i;
		}
	}

	return -1;
}

/*
 * given a command's input buffer, extract name
 */
int extract_name(char * buf, char * user_name)
{
	char inbuf[MAX_MSG];
    char * tokens[16];
    strcpy(inbuf, buf);

    int token_cnt = parse_line(inbuf, tokens, " ");

    if(token_cnt >= 2) {
        strcpy(user_name, tokens[1]);
        return 0;
    }

    return -1;
}

int extract_text(char *buf, char * text)
{
    char inbuf[MAX_MSG];
    char * tokens[16];
    char * s = NULL;
    strcpy(inbuf, buf);

    int token_cnt = parse_line(inbuf, tokens, " ");

    if(token_cnt >= 3) {
        //Find " "
        s = strchr(buf, ' ');
        s = strchr(s+1, ' ');

        strcpy(text, s+1);
        return 0;
    }

    return -1;
}

/*
 * send personal message
 */
void send_p2p_msg(int idx, USER * user_list, char *buf)
{
	// get the target user by name using extract_name() function
	char user_name[MAX_USER_ID];
  memset(user_name, 0, sizeof(user_name));
	extract_name(buf, user_name);
	// find the user id using find_user_index()
	int jdx = find_user_index(user_list, user_name);
	// if user not found, write back to the original user "User not found", using the write()function on pipes.
	if (jdx == -1)
	{
		int write1 = write(user_list[idx].m_fd_to_user, "User not found", MAX_MSG);
    if(write1 < 0) {
      perror("Fail to write back to the original user 'User not found'in send_p2p_msg");
    }
	}
	// if the user is found then write the message that the user wants to send to that user.
	else
	{
    char text[MAX_MSG];
    extract_text(buf, text);
    char message[MAX_MSG];
    strcpy(message, user_list[idx].m_user_id);
    strcat(message, ": ");
		strcat(message, text);
		int write2 = write(user_list[jdx].m_fd_to_user, message, MAX_MSG);
    if (write2 < 0){
      perror("Fail to write the message from user to other user in send_p2p_msg");
    }

    // Now print to SERVER.

    // FORMATTING TO LOOK NICE.
    // BEWARE YOUR EYES. THIS IS BRUTE FORCE CODING.
    for (int i = 0; i < 10; i++)
    {
      printf("\b");
    }
    for (int i = 0; i < 10; i++)
    {
      printf(" ");
    }
    for (int i = 0; i < 10; i++)
    {
      printf("\b");
    }
    // END OF FORMATTING.

    printf("%s whispered to %s: %s\n", user_list[idx].m_user_id, user_list[jdx].m_user_id, text); // Print message.
	}
}

//takes in the filename of the file being executed, and prints an error message stating the commands and their usage
void show_error_message(char *filename)
{
	// TO DO.
}


/*
 * Populates the user list initially
 */
void init_user_list(USER * user_list) {

	//iterate over the MAX_USER
	//memset() all m_user_id to zero
	//set all fd to -1
	//set the status to be EMPTY
	int i=0;
	for(i=0;i<MAX_USER;i++) {
		user_list[i].m_pid = -1;
		memset(user_list[i].m_user_id, '\0', MAX_USER_ID);
		user_list[i].m_fd_to_user = -1;
		user_list[i].m_fd_to_server = -1;
		user_list[i].m_status = SLOT_EMPTY;
	}
}

void cleanup_error(USER * user_list) {
  for (int i = 0; i < MAX_USER; i++)
  {
    if (user_list[i].m_status == SLOT_FULL)
    {
      kick_user(i, user_list);
    }
  }
}

/* ---------------------End of the functions that implementServer functionality -----------------*/


/* ---------------------Start of the Main function ----------------------------------------------*/
int main(int argc, char * argv[])
{
  int waitTime = 1000;
  char * YOUR_UNIQUE_ID = "CSCI4061_39";
	int nbytes;
	setup_connection(YOUR_UNIQUE_ID); // Specifies the connection point as argument.

	USER user_list[MAX_USER];
	init_user_list(user_list);   // Initialize user list

	char buf[MAX_MSG];
  char buf_err[MAX_MSG]; // copy of buf for error checking.
	fcntl(0, F_SETFL, fcntl(0, F_GETFL)| O_NONBLOCK);
	print_prompt("admin");

	//
	while(1) {
		/* ------------------------YOUR CODE FOR MAIN--------------------------------*/

		// Handling a new connection using get_connection
		int pipe_SERVER_reading_from_child[2];
		int pipe_SERVER_writing_to_child[2];
		int pipe_child_reading_from_user[2];
		int pipe_child_writing_to_user[2];

		char user_id[MAX_USER_ID];


		// ------------------------------------------------------------------------------------ //
		// -------------------------- POLL FOR NEW USER CONNECTIONS. -------------------------- //
		// ------------------------------------------------------------------------------------ //

		// Gets connections and internally creates the pipes for communication.
		if (get_connection(user_id, pipe_child_writing_to_user, pipe_child_reading_from_user) != -1)
		{
			// Check max user and same user id
			int idx = find_empty_slot(user_list);
      int existence = find_user_index(user_list, user_id);
			if ( (idx != -1) && (existence < 0) )
			{

				// FORMATTING TO LOOK NICE.
				for (int i = 0; i < 10; i++)
				{
					printf("\b"); // Removes and replaces the admin prompt with user message.
				}
				printf("USER CONNECTED: %s\n", user_id);
				print_prompt("admin");

				// Insert new user if there is space available the user does not already exist.
				// Create pipes from child to USER.
				if ( (pipe(pipe_SERVER_reading_from_child) < 0) || (pipe(pipe_SERVER_writing_to_child) < 0) )
				{
					printf("ERROR: Failed to create pipes for USER: %s.\n", user_id);
					return(-1);
				}

				// Set CHILD-TO-USER pipes to NONBLOCKING behaviour.
				fcntl(pipe_child_reading_from_user[0], F_SETFL, fcntl(pipe_child_reading_from_user[0], F_GETFL)| O_NONBLOCK);
				fcntl(pipe_child_reading_from_user[1], F_SETFL, fcntl(pipe_child_reading_from_user[1], F_GETFL)| O_NONBLOCK);
				fcntl(pipe_child_writing_to_user[0], F_SETFL, fcntl(pipe_child_writing_to_user[0], F_GETFL)| O_NONBLOCK);
				fcntl(pipe_child_writing_to_user[1], F_SETFL, fcntl(pipe_child_writing_to_user[1], F_GETFL)| O_NONBLOCK);
				// Make CHILD-TO-SERVER pipes to NONBLOCKING behaviour.
				fcntl(pipe_SERVER_reading_from_child[0], F_SETFL, fcntl(pipe_SERVER_reading_from_child[0], F_GETFL)| O_NONBLOCK);
				fcntl(pipe_SERVER_reading_from_child[1], F_SETFL, fcntl(pipe_SERVER_reading_from_child[1], F_GETFL)| O_NONBLOCK);
				fcntl(pipe_SERVER_writing_to_child[0], F_SETFL, fcntl(pipe_SERVER_writing_to_child[0], F_GETFL)| O_NONBLOCK);
				fcntl(pipe_SERVER_writing_to_child[1], F_SETFL, fcntl(pipe_SERVER_writing_to_child[1], F_GETFL)| O_NONBLOCK);

				int pid = fork(); // Fork and create a child procress.

				// ERROR.
				if (pid < 0)
				{
				  printf("ERROR: Failed to fork.\n");
          cleanup_error(user_list);
				  return -1;
				}
				// -------------------------------------------------------------------- //
				// -------------------------- Child process. -------------------------- //
				else if (pid == 0)
				{
					// Close unused pipes in CHILD-TO-USER.
					int pipeChildClose1 = close(pipe_child_reading_from_user[1]);
					int pipeChildClose2 = close(pipe_child_writing_to_user[0]);
          if( pipeChildClose1 < 0 || pipeChildClose2 < 0) {
            perror("Fail to close unused pipes in CHILD-TO-USER in Child process");
          }
					// Close unused pipes in CHILD-TO-SERVER.
					int pipeServerClose1 = close(pipe_SERVER_reading_from_child[0]);
					int pipeServerClose2 = close(pipe_SERVER_writing_to_child[1]);
          if( pipeServerClose1 < 0 || pipeServerClose2 < 0 ) {
            perror("Fail to close unused pipes in CHILD-TO-SERVER in Child process");
          }

					// Child process: poll users and SERVER
					while (1)
					{
						// -------------------------- Poll on SERVER. -------------------------- //
						memset(buf, 0, sizeof(buf));
						int serverStatus = read(pipe_SERVER_writing_to_child[0], buf, MAX_MSG);
						if ( (serverStatus < 0) && (errno == EAGAIN) )
						{
							// No message to be read from SERVER. Pass.
						}
						else if (serverStatus != 0)
						{
							// Message read from SERVER.
							if (write(pipe_child_writing_to_user[1], buf, MAX_MSG) < 0)
							{
								printf("ERROR: Failed to write to USER: %s\n", user_id);
								exit(-1);
							}
						}
						else
						{
							// ERROR occured.
							printf("ERROR: Failed to read from SERVER.\n");
							exit(-1);
						}

						// -------------------------- Poll on USER. -------------------------- //
						memset(buf, 0, sizeof(buf));
						int userStatus = read(pipe_child_reading_from_user[0], buf, MAX_MSG);
						if ( (userStatus < 0) && (errno == EAGAIN) )
						{
						  // No message to be read from USER. Pass.
						}
						else if (userStatus != 0)
						{
							// Message read from USER.
							// Forward to SERVER for procressing.
							if (write(pipe_SERVER_reading_from_child[1], buf, MAX_MSG) < 0)
							{
								printf("ERROR: Failed to write to SERVER.\n");
								exit(-1);
							}
						}
						else
						{
							// ERROR occured.
							printf("ERROR: Failed to read from USER: %s.\n", user_id);
							exit(-1);
						}

					usleep(waitTime);
					}
				}
				// -------------------------------------------------------------------- //

				// -------------------------- Parent/SERVER process. -------------------------- //
				else
				{
					// Server process: Add a new user information into an empty slot
					add_user(idx, user_list, pid, user_id, pipe_SERVER_writing_to_child[1], pipe_SERVER_reading_from_child[0]);
					// Close unused pipes in CHILD-TO-USER.
					int childReadClose1 = close(pipe_child_reading_from_user[0]);
					int childReadClose2 = close(pipe_child_reading_from_user[1]);
					int childWriteClose1 = close(pipe_child_writing_to_user[0]);
					int childWriteClose2 = close(pipe_child_writing_to_user[1]);
          if(childReadClose1 < 0 || childReadClose2 < 0 || childWriteClose1 < 0 || childWriteClose2 < 0) {
            perror("Fail to close unused pipes in CHILD-TO-USER in Parent/SERVER process");
          }
					// Close unused pipes in CHILD-TO-SERVER.
					int serverClose1 = close(pipe_SERVER_reading_from_child[1]);
					int serverClose2 = close(pipe_SERVER_writing_to_child[0]);
          if( serverClose1 < 0 || serverClose2 < 0 ) {
            perror("Fail to close unused pipes in CHILD-TO-SERVER in Parent/SERVER process.");
          }
				}
				// ---------------------------------------------------------------------------- //
			}
      else
      {
        if (idx == -1)
        {
          // Send message to USER that SEVER canot accept any more users.
          int write1 = write(pipe_child_writing_to_user[1], "Server is full", MAX_MSG);
          if( write1 < 0 ){
            perror("Fail to write to send message to USER that SEVER canot accept any more users.");
          }
          // Close pipes.
          // Closing these pipes will trigger the EOF in the USER when reading.
          int close1 = close(pipe_child_reading_from_user[0]);
					int close2 = close(pipe_child_reading_from_user[1]);
					int close3 = close(pipe_child_writing_to_user[0]);
					int close4 = close(pipe_child_writing_to_user[1]);
          if (close1 < 0 || close2 < 0 || close3 < 0 || close4 < 0 ) {
            perror("Fail to close pipes when server cannot accept anymore user.");
          }
        }
        else
        {
          // Send message to USER that there already exists a user with the same name.
          // Send message to USER that SEVER canot accept any more users.
          int write2 = write(pipe_child_writing_to_user[1], "User name already exists", MAX_MSG);
          if(write2 < 0){
            perror("Fail to write to send message to USER that SEVER canot accept any more users.");
          }
          // Close pipes.
          // Closing these pipes will trigger the EOF in the USER when reading.
          int close1 = close(pipe_child_reading_from_user[0]);
					int close2 = close(pipe_child_reading_from_user[1]);
					int close3 = close(pipe_child_writing_to_user[0]);
					int close4 = close(pipe_child_writing_to_user[1]);
          if (close1 < 0 || close2 < 0 || close3 < 0 || close4 < 0 ) {
            perror("Fail to close pipes when there already exists a user with the same name.");
          }
        }
      }
		}


		// ---------------------------------------------------------------------- //
		// -------------------------- POLL FROM STDIN. -------------------------- //
		// ---------------------------------------------------------------------- //

		// Poll stdin (input from the terminal) and handle admnistrative command
		memset(buf, 0, sizeof(buf));
    memset(buf_err, 0, sizeof(buf_err));
		int status = read(0, buf, MAX_MSG);
    strcpy(buf_err, buf);
		if ( (status < 0) && (errno == EAGAIN) )
		{
			// No message to be read from STDIN. Pass.
		}
		else if (status != 0)
		{
      if (strcmp(strtok(buf_err, " "), "\n") == 0)
      {
        // SERVER sent an 'empty' message.
      }
      else
      {
        char user_name[MAX_USER_ID];
  			// Message received from STDIN.
  			switch(get_command_type(strtok(buf, "\n")))
        {
  				case LIST:
  					// Server requests listing of all users.
  					list_users(-1, user_list);
  					break;
  				case KICK:
  					// Kick user.
  					extract_name(strtok(buf, "\n"), user_name);
  					kick_user(find_user_index(user_list, user_name), user_list);
  					break;
  				case EXIT:
  					// Kick all users and terminate the server.
  					for (int i = 0; i < MAX_USER; i++)
  					{
              if (user_list[i].m_status == SLOT_FULL)
              {
                kick_user(i, user_list);
              }
  					}
  					// Kill self.
  					return 0;
  				default:
  					// Send a notice to all users.
  					broadcast_msg(user_list, strtok(buf, "\n"), "");
  					break;
          }
			}
			print_prompt("admin"); // Prints admin prompt to screen again.
		}
		else
		{
			// ERROR occured.
			printf("ERROR: Failed to read from STDIN.\n");
      cleanup_error(user_list);
			return -1;
		}


		// --------------------------------------------------------------------------------- //
		// -------------------------- POLL FROM CHILD PROCRESSES. -------------------------- //
		// --------------------------------------------------------------------------------- //

		for (int i = 0; i<MAX_USER; i++)
		{
			// poll child processes and handle user commands
			if (user_list[i].m_status != SLOT_EMPTY)
			{
				memset(buf, 0, sizeof(buf));
				int status = read(user_list[i].m_fd_to_server, buf, MAX_MSG);
				if ( (status < 0) && (errno == EAGAIN) )
				{
					// No message to be read from CHILD. Pass.
				}
				else if (status != -1)
				{
          // Message read from CHILD.
					// Process user message/command.
					switch(get_command_type(buf))
					{
						case LIST:
							// Server requests listing of all users.
							list_users(i, user_list);
              print_prompt("admin");
							break;
						case P2P:
							// Communicate with another user that the receiving user requested for.
							// Extract name.
							send_p2p_msg(i, user_list, buf);
              print_prompt("admin");
							break;
						case EXIT:
							// Kick the user, because they want to exit.
							kick_user(i, user_list);
              print_prompt("admin");
							break;
						default:
              // FORMATTING TO LOOK NICE.
              // BEWARE YOUR EYES. THIS IS BRUTE FORCE CODING.
        			for (int i = 0; i < 10; i++)
        			{
        				printf("\b");
        			}
        			for (int i = 0; i < 10; i++)
        			{
        				printf(" ");
        			}
        			for (int i = 0; i < 10; i++)
        			{
        				printf("\b");
        			}
              // END OF FORMATTING.

							printf("%s: %s\n", user_list[i].m_user_id, buf); // Print message.
							print_prompt("admin"); // Prints admin prompt to screen again.

							broadcast_msg(user_list, buf, user_list[i].m_user_id);
							break;
					}
				}
				else
				{
					// ERROR occured.
					printf("ERROR: Failed to read from CHILD: %s.\n", user_id);
          cleanup_error(user_list);
          return -1;
				}
			}
		}

		usleep(waitTime);
		/* ------------------------YOUR CODE FOR MAIN--------------------------------*/
	}
}

/* --------------------End of the main function ----------------------------------------*/
