/* CSci4061 F2018 Assignment 1
* login: cselabs login name (login used to submit)
* date: mm/dd/yy
* name: full name1, full name2, full name3 (for partner(s))
* id: id for first name, id for second name, id for third name */

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
// //TODO:
// - MAKE THE -h which is the usage case so ppl know what to do
// - Check syntax of the makefile and if it is not the correct syntax we need to exit out.
// - Make sure ONLY errors are printed to screen (as well as commands that are run?)
// - Exit parent processes if the child process is unsuccessful (DONE)
// - makefile correct syntax?
// - Specific target works? Test for that
/*-------------------------------------------------------HELPER FUNCTIONS PROTOTYPES---------------------------------*/
void show_error_message(char * ExecName);
//TODO: WRITE THE DAMN USER PAGE look at make -h FOR EXAMPLE!! DO NOT FORGET !!!
//Write your functions prototypes here
void show_targets(target_t targets[], int nTargetCount);

int build(target_t * target, target_t targets[], int nTargetCount) {
	// Recursion loop to build the dependencies of the target.
	int numOfDependencies = target->DependencyCount;
	if (numOfDependencies != 0) {
		for (int i=0; i<numOfDependencies; i++) {
			printf("Starting child %s\n", target->DependencyNames[i]);
			int idx = find_target(target->DependencyNames[i], targets, nTargetCount);
			printf("Index found: %d\n", idx);
			if (idx!=-1) {
				int pid = fork(); // we are forking to build current dependencies;
				if (pid<0) {
					exit(-1);
				}
				else if (pid==0) {
					int result = build(&targets[idx], targets, nTargetCount);
					if (result==-1) {
						exit(-1);
					}
					printf("Finishing child:%s", targets[idx].TargetName);
					exit(0);
				}

				else {
					wait(&pid);
					if (WEXITSTATUS(pid) != 0) {
						printf("child exited with error code=%d\n", WEXITSTATUS(pid));
						exit(-1);
					}
					if (targets[idx].Status == NEEDS_BUILDING) {
						target->Status = NEEDS_BUILDING;
					}
					targets[idx].Status == FINISHED;
					// TODO: Write Error checking to see if other childs processes have failed
				}
			}
			else {
				if (does_file_exist(target->DependencyNames[i]) == -1) {
					printf("File does not exist:'%s'\n", target->DependencyNames[i]); //edit later
					exit(-1);
				}
			}

			int modificationTime = compare_modification_time(target->TargetName, target->DependencyNames[i]);
			printf("'%s':'%s'; mod time: '%d'\n",target->TargetName, target->DependencyNames[i], modificationTime);

			//TARGET.TARGETNAME is the child because we are now at the base case case
			/*
			This compares the Target that we are building with the depdency. Targets[idx] find thes depdency.
			/*

			/*
			if (modificationTime==-1) {
				int success = does_file_exist(targets[idx].DependencyNames[i]);
				if (success==-1) {
					printf("Input file does not exist: %s,.\n", targets[idx].DependencyNames[i]);
					return -1;
				}
			}
			//WHAT IF SAME TIME FOR TARGET AND DEPDENCIES */
			if (modificationTime == 2 || modificationTime == -1) {
				target->Status = NEEDS_BUILDING;
			}
		}
	} else {
		target->Status = NEEDS_BUILDING;
	}

	printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n'%s' - '%d'\n", target->TargetName, does_file_exist(target->TargetName));

	printf("Checking if baby needs to recompile.\n");
	printf("STATUS: '%d'\n", target->Status);

	if (target->Status == NEEDS_BUILDING) { //we have to recompile
		//This means parent is modified eariler than the child
		//This means parent has been recently adjusted and the
		// execvp(target->Command, )
		//
		// target->Status = FINISHED;
		// execvp()
		printf("\nCommand for %s is: '%s'", target->TargetName, target->Command);
		char *commandTokens[256];
		int numTokens;
		numTokens = parse_into_tokens(target->Command, commandTokens, " ");
		if (numTokens != 0) {
			execvp(commandTokens[0], commandTokens);
		}
		//Want to start the array of tokens on the second toker
	}

	printf("	End: %s\n", target->TargetName);
	printf("	End: %d\n", target->Status);


	return 1;
}
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


	for (int i=0; i<nTargetCount; i++) { // Loop through the targets and build them.
		//printf("index,: %i, Command: %s, Target Name: %s", i, targets[i].Command, targets[i].TargetName);
		if(strcmp(targets[i].TargetName, TargetName) == 0) //this means the current target name is the same as the what we are looking for
		{
			build(&targets[i], targets, nTargetCount); //pass in "parent"
			break;
		}
	}

  /*End of your code*/
  //End of Phase2------------------------------------------------------------------------------------------------------

  return 0;
}
/*-------------------------------------------------------END OF MAIN PROGRAM------------------------------------------*/
