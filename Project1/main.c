/* CSci4061 F2018 Assignment 1
* login: cselabs login name (login used to submit)
* date: 10/05/2018
* name: Cassandra Chanthamontry, Joey Vang, Ounngy Ing
* id: id for first name, id for second name, ingxx006 */

// This is the main file for the code
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"
/*-------------------------------------------------------HELPER FUNCTIONS PROTOTYPES---------------------------------*/
void show_error_message(char * ExecName);
//Write your functions prototypes here
void show_targets(target_t targets[], int nTargetCount);

int build(target_t * target, target_t targets[], int nTargetCount);

/*-------------------------------------------------------END OF HELPER FUNCTIONS PROTOTYPES--------------------------*/


/*-------------------------------------------------------HELPER FUNCTIONS--------------------------------------------*/

//This is the function for writing an error to the stream
//It prints the same message in all the cases
void show_error_message(char * ExecName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", ExecName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	exit(0);
}

//Write your functions here

//Phase1: Warmup phase for parsing the structure here. Do it as per the PDF (Writeup)
void show_targets(target_t targets[], int nTargetCount)
{
	for (int i=0; i<nTargetCount; i++) {
		//Extract the fields.
		char* targetName = targets[i].TargetName;
		int dependencyCount = targets[i].DependencyCount;
		char* command = targets[i].Command;

		printf("TargetName: %s \n", targetName);
		printf("  DependencyCount: %d \n", dependencyCount);
		printf("  DependencyNames: ");
		printf("%s", targets[i].DependencyNames[0]);
		for (int j=1; j<dependencyCount; j++) {
			char* dependencyName = targets[i].DependencyNames[j];
			printf(", ");
			printf("%s", dependencyName);
		}
		printf("\n  Command: %s \n", command);
	}
}

int build(target_t * target, target_t targets[], int nTargetCount) {
	// Recursion loop to build the dependencies of the target.
	int numOfDependencies = target->DependencyCount;

	// ---------------------------------------------------------------------------------
	// TARGET HAS DEPENDENCIES.
	// ---------------------------------------------------------------------------------
	if (numOfDependencies != 0) {
		for (int i=0; i<numOfDependencies; i++) {
			int idx = find_target(target->DependencyNames[i], targets, nTargetCount);
			if (idx == -1) {
	// The target could not be found inside the targets array.
	// Check to see if it is a file within the current directory that can be built.

				if (does_file_exist(target->DependencyNames[i]) == -1) {
	// If the dependency does not exist in the array and is a file that does not exist.
	// Then we return -1 to indicate failure.
					 printf("make: *** No rule to make target'%s', needed by %s.   Stop.\n", target->DependencyNames[i], target->TargetName); //edit later
					return -1;
				}
			}

			else if (targets[idx].Status != FINISHED) {
	// Dependency is found and MUST be built. Call build() recursively to reach the bottom of the "tree".
				int result = build(&targets[idx], targets, nTargetCount);

				if (result == -1) {
	// Recursively return -1 if any errors occur in building or executing the target's dependencies.
					return -1;
				}

	// AFTER RECURSION. We are now at the 2nd-to-last target at bottom of the "tree".
	// If the dependencies need to be built, then so does the current target.
	// This ensures that the current target is updated, if its dependencies are updated.
				if (targets[idx].Status == NEEDS_BUILDING) {
					target->Status = NEEDS_BUILDING;
				}
				targets[idx].Status = FINISHED;
			}

	// Now, we check timestamps.
			int modificationTime = compare_modification_time(target->TargetName, target->DependencyNames[i]);
			if (modificationTime == 2 || modificationTime == -1) {
	// If the dependency has been updated (2) or the target has not yet been built (-1), then set flag for building.
	// Do not that although (-1) indicates that either the target or dependency do not exist, we've already done
	// checking for the dependency beforehand at line 82.
				target->Status = NEEDS_BUILDING;
			}
		}
	}

	// ---------------------------------------------------------------------------------
	// TARGET HAS NO DEPENDENCIES.
	// ---------------------------------------------------------------------------------
	else {
		target->Status = NEEDS_BUILDING;
	}

  // ---------------------------------------------------------------------------------

	// End of building the "tree". Now we build targets below.

	if (target->Status == NEEDS_BUILDING) {
	// ---------------------------------------------------------------------------------
	// TARGET NEEDS TO BE BUILT.
	// ---------------------------------------------------------------------------------
		char *commandTokens[256];
		int numTokens;

		printf("%s\n", target->Command);
		numTokens = parse_into_tokens(target->Command, commandTokens, " ");
	// Parse the command string into tokens that can be read by execvp().
	// Also check if it parses anything at all.

		if (numTokens != 0) {
	// If there are tokens...
	// Then we fork(). The child process will execute the command line, while the parent
	// will wait on the child to complete.
			int pid = fork();

			if (pid<0) {
	// Forking failed!
				exit(-1);
			}

			else if (pid==0) {
	// Child process.
	// Executes the line and checks for error.
	// If process execution failed. Report the error back to the parent.
				if(execvp(commandTokens[0], commandTokens)<0)
				{
					printf("make: %s: Command not found.\n", commandTokens[0]);
					printf("Makefile: recipe for target '%s' failed.\n", target->TargetName);
					exit(errno);
				}
			}

			else {
	// Parent process.
	// Check to see what kind of status is returned from the child after waiting.
	// If returned status is not a success (0), then print failure and stop program by returning -1.
				int status;
				wait(&status);
				if (WEXITSTATUS(status) != 0) {
					printf("Error executing command: %s\n", target->Command);
					return -1;
				}
				return 1;
			}
		}
	}

	// ---------------------------------------------------------------------------------
	// TARGET DOES NOT NEED TO BE BUILT. THUS IS SUCCESSFUL.
	// ---------------------------------------------------------------------------------
	return 1;
}

/*-------------------------------------------------------END OF HELPER FUNCTIONS-------------------------------------*/


/*-------------------------------------------------------MAIN PROGRAM------------------------------------------------*/
//Main commencement
int main(int argc, char *argv[])
{
  target_t targets[MAX_NODES];
	for(int i = 0; i < MAX_NODES; i++) {
		targets[i].Status = UNFINISHED;
		//SET ALL THE TARGETS TO UNFINISHED
	}

  int nTargetCount = 0;

  /* Variables you'll want to use */
  char Makefile[64] = "Makefile";
  char TargetName[64];

  /* Declarations for getopt. For better understanding of the function use the man command i.e. "man getopt" */
  extern int optind;   		// It is the index of the next element of the argv[] that is going to be processed
  extern char * optarg;		// It points to the option argument
  int ch;
  char *format = "f:h";
  char *temp;

  //Getopt function is used to access the command line arguments. However there can be arguments which may or may not need the parameters after the command
  //Example -f <filename> needs a finename, and therefore we need to give a colon after that sort of argument
  //Ex. f: for h there won't be any argument hence we are not going to do the same for h, hence "f:h"
  while((ch = getopt(argc, argv, format)) != -1)
  {
	  switch(ch)
	  {
	  	  case 'f':
	  		  temp = strdup(optarg);
	  		  strcpy(Makefile, temp);  // here the strdup returns a string and that is later copied using the strcpy
	  		  free(temp);	//need to manually free the pointer
	  		  break;

	  	  case 'h': //TODO: WRITE THE -H
	  	  default:
	  		  show_error_message(argv[0]);
	  		  exit(1);
	  }

  }

  argc -= optind;
  if(argc > 1)   //Means that we are giving more than 1 target which is not accepted
  {
	  show_error_message(argv[0]);
	  return -1;   //This line is not needed
  }

  /* Init Targets */
  memset(targets, 0, sizeof(targets));   //initialize all the nodes first, just to avoid the valgrind checks

  /* Parse graph file or die, This is the main function to perform the toplogical sort and hence populate the structure */
  if((nTargetCount = parse(Makefile, targets)) == -1)  //here the parser returns the starting address of the array of the structure. Here we gave the makefile and then it just does the parsing of the makefile and then it has created array of the nodes
	  return -1;


  //Phase1: Warmup-----------------------------------------------------------------------------------------------------
  //Parse the structure elements and print them as mentioned in the Project Writeup
  /* Comment out the following line before Phase2 */



	//UNCOMMENT THE LINE BELOW FOR PHASE 1
  //show_targets(targets, nTargetCount);



  //End of Warmup------------------------------------------------------------------------------------------------------

  /*
   * Set Targetname
   * If target is not set, set it to default (first target from makefile)
   */
  if(argc == 1)
	strcpy(TargetName, argv[optind]);    // here we have the given target, this acts as a method to begin the building
  else
  	strcpy(TargetName, targets[0].TargetName);  // default part is the first target

  /*
   * Now, the file has been parsed and the targets have been named.
   * You'll now want to check all dependencies (whether they are
   * available targets or files) and then execute the target that
   * was specified on the command line, along with their dependencies,
   * etc. Else if no target is mentioned then build the first target
   * found in Makefile.
   */

  //Phase2: Begins ----------------------------------------------------------------------------------------------------
  /*Your code begins here*/

  for (int i=0; i<nTargetCount; i++) {
	// If a target name is passed, we do a linear search to find it in the makefile to build.
	// Otherwise, it is defaulted to the first target, as done on line 264.
		if(strcmp(targets[i].TargetName, TargetName) == 0)
		{
			build(&targets[i], targets, nTargetCount);
		}
	}

  /*End of your code*/
  //End of Phase2------------------------------------------------------------------------------------------------------

  return 0;
}
/*-------------------------------------------------------END OF MAIN PROGRAM------------------------------------------*/
