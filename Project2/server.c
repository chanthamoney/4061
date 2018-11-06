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
		printf(buf);
		printf("\n");
	} else {
		/* write to the given pipe fd */
		if (write(user_list[idx].m_fd_to_user, buf, strlen(buf) + 1) < 0)
			perror("writing to server shell");
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

  /*

  char * killCommand = "\exit";
  write(user_list[idx].m_fd_to_user, killCommand, MAX_MSG);
  int ret;
  while(1)
  { //find user pid.....
    ret =waitpid(pid, status, WNOHANG); //WNOHANG wait with nonblocking?
    if(pid==0) // means killing the child
    {
      kill(user_list[idx].m_pid, SIGINT);
      exit(1);
      //todo find pid of user
    }
    sleep(1000);
  }

  */

}

/*
 * Perform cleanup actions after the used has been killed
 */
void cleanup_user(int idx, USER * user_list)
{
	// m_pid should be set back to -1
	// m_user_id should be set to zero, using memset()
	// close all the fd
	// set the value of all fd back to -1
	// set the status back to empty
}

/*
 * Kills the user and performs cleanup
 */
void kick_user(int idx, USER * user_list) {
	// should kill_user()
	// then perform cleanup_user()
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
  strcpy(message, sender);
  strcat(message, ": ");
  strcat(message, buf);

  for (int i = 0; i<MAX_USER; i++)
  {
    if (user_list[i].m_status != SLOT_EMPTY && user_list[i].m_user_id != sender)
    {
      if (write(user_list[i].m_fd_to_user, message, MAX_MSG) < 0)
      {
        printf("ERROR: Failed to write to USER: %s.\n", user_list[i].m_user_id);
        return -1;
      }
    }
  }

	return 0;
}

/*
 * Cleanup user chat boxes
 */
void cleanup_users(USER * user_list)
{
	// go over the user list and check for any empty slots
	// call cleanup user for each of those users.
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
    strcpy(inbuf, buf);

    int token_cnt = parse_line(buf, tokens, " ");

    if(token_cnt >= 3) {
        strcpy(text, tokens[2]);
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
	// find the user id using find_user_index()
	// if user not found, write back to the original user "User not found", using the write()function on pipes.
	// if the user is found then write the message that the user wants to send to that user.
}

//takes in the filename of the file being executed, and prints an error message stating the commands and their usage
void show_error_message(char *filename)
{

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

/* ---------------------End of the functions that implementServer functionality -----------------*/


/* ---------------------Start of the Main function ----------------------------------------------*/
int main(int argc, char * argv[])
{
  int waitTime = 1000;
  char * YOUR_UNIQUE_ID = "CSCI_39";
	int nbytes;
	setup_connection(YOUR_UNIQUE_ID); // Specifies the connection point as argument.

	USER user_list[MAX_USER];
	init_user_list(user_list);   // Initialize user list

	char buf[MAX_MSG];
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
      printf("\nUSER CONNECTED: %s\n", user_id);
      // Check max user and same user id
      int idx = find_empty_slot(user_list);
      if ( (idx != -1) && (find_user_index(user_list, user_id) < 0) )
      // Insert new user if there is space available the user does not already exist.
      {
        // Create pipes from child to USER.
        // Error checking is done on every system call.
        // Error check when creating pipes.
        if ( (pipe(pipe_SERVER_reading_from_child) < 0) || (pipe(pipe_SERVER_writing_to_child) < 0) ) {
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
          return -1;
        }

      // -------------------------- Child process. -------------------------- //
        else if (pid == 0)
        {
          // Child process: poll users and SERVER
          char serverBuffer[MAX_MSG];
          char userBuffer[MAX_MSG];
          while (1)
          {
          // -------------------------- Poll on SERVER. -------------------------- //
            memset(serverBuffer, 0, sizeof(serverBuffer));
            int serverStatus = read(pipe_SERVER_writing_to_child[0], serverBuffer, MAX_MSG);
            if ( (serverStatus < 0) && (errno == EAGAIN) )
            {
              // No message to be read from SERVER. Pass.
            }
            else if (serverStatus != -1)
            {
              // Message read from SERVER.
              if (write(pipe_child_writing_to_user[1], serverBuffer, MAX_MSG) < 0)
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
            memset(userBuffer, 0, sizeof(userBuffer));
            int userStatus = read(pipe_child_reading_from_user[0], userBuffer, MAX_MSG);
            if ( (userStatus < 0) && (errno == EAGAIN) )
            {
              // No message to be read from USER. Pass.
            }
            else if (userStatus != -1)
            {
              // Message read from USER.
              // Forward to SERVER for procressing.
              if (write(pipe_SERVER_reading_from_child[1], userBuffer, MAX_MSG) < 0)
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

      // -------------------------- Parent/SERVER process. -------------------------- //
        else
        {
          // Server process: Add a new user information into an empty slot
          add_user(idx, user_list, pid, user_id, pipe_SERVER_writing_to_child[1], pipe_SERVER_reading_from_child[0]);
        }
      }
      // -------------------------- Finished establishing new user connection. -------------------------- //
    }

    // ---------------------------------------------------------------------- //
    // -------------------------- POLL FROM STDIN. -------------------------- //
    // ---------------------------------------------------------------------- //
		// Poll stdin (input from the terminal) and handle admnistrative command
    char stdinBuffer[MAX_MSG];
    char message[MAX_MSG];
    memset(stdinBuffer, 0, sizeof(stdinBuffer));
    memset(message, 0, sizeof(message));
    int status = read(0, stdinBuffer, MAX_MSG);
    //printf("status of STDIN: %d\n", status);
    if ( (status < 0) && (errno == EAGAIN) )
    {
      // No message to be read from STDIN. Pass.
    }
    else if (status != 0)
    {
      // Message received from STDIN.
      // Send message to CHILD.
      for (int i = 0; i<MAX_USER; i++)
      {
        // poll child processes and handle user commands
        if (user_list[i].m_status != SLOT_EMPTY)
        {
          strcpy(message, "Notice: ");
          strcat(message, stdinBuffer);
          if (write(user_list[i].m_fd_to_user, message, MAX_MSG) < 0)
          {
            printf("ERROR: Failed to write to USER: %s.\n", user_list[i].m_user_id);
            return -1;
          }
        }
      }
      print_prompt("admin");
    }
    else
    {
      // ERROR occured.
      printf("ERROR: Failed to read from STDIN.\n");
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
          char buffer[MAX_MSG];
          memset(buffer, 0, sizeof(buffer));
          int status = read(user_list[i].m_fd_to_server, buffer, MAX_MSG);
          if ( (status < 0) && (errno == EAGAIN) )
          {
            // No message to be read from CHILD. Pass.
          }
          else if (status != -1)
          {
            // Message read from CHILD.
            // Process user message/command.

            // FORMATTING TO LOOK NICE.
            for (int i = 0; i < 10; i++)
            {
              printf("\b"); // Removes and replaces the admin prompt with user message.
            }
            printf("%s: %s", user_list[i].m_user_id, buffer);
            print_prompt("admin"); // Prints admin prompt to screen again.

            broadcast_msg(user_list, buffer, user_list[i].m_user_id);

          }
          else
          {
            // ERROR occured.
            printf("ERROR: Failed to read from CHILD: %s.\n", user_id);
          }
        }
    }

    usleep(waitTime);

		/* ------------------------YOUR CODE FOR MAIN--------------------------------*/
	}
}

/* --------------------End of the main function ----------------------------------------*/
