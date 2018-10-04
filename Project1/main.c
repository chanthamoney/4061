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
	if (target->DependencyCount != 0) {
		for (int i=0; i<target->DependencyCount; i++) {
			int idx = find_target(target->DependencyNames[i], targets, nTargetCount);

			if (idx==-1) { // Failed to find dependency in array.
				if (does_file_exist(target->DependencyNames[i])!=-1) { // Could it be a file that needs to be processed?
					int condition = compare_modification_time(target->TargetName, target->DependencyNames[i]);
					printf("		timeStamp: %d\n", condition);

					if (condition==0 || condition==1) {
						target->Status = 1;
					}
					break;
				}
				else { // No. Dependency does not exist in array or directory.
					return -1;
				}
			}

			// Check modification time stamps if files exist.
			int condition = compare_modification_time(target->TargetName, target->DependencyNames[i]);
			printf("		timeStamp: %d\n", condition);

			if (condition==0 || condition==1) {
				target->Status = 1;
			}

			// Recursively build downward and recursively update upwards parents if children need to be built.
			 int result = build(&targets[idx], targets, nTargetCount);

			 if (result==-1) {
				 return -1;
			 }
			 else if (result==0) {
				 target->Status = 0;
			 }
		 }
	 }

	// The target has no dependencies to build.
	// This will happen at the children/leaves and work its way upward.
	if (target->Status!=1) { // 1 = complete.
		printf("		Target Built: %s\n", target->TargetName);
		target->Status = 1;
		char command[256]; strcpy(command, target->Command); // Preserve command, because parse_into_tokens destroys it.
		char * args[64];
		parse_into_tokens(target->Command, args, " ");

		//Fork here.
		int pid;
		pid = fork();

		if (pid<0) {
			exit(-1);
		}
		else if (pid==0) {
			if (strcmp(args[0],"echo")!=0 && strcmp(args[0],"rm")!=0) { // Special cases that should not be executed.
				execvp(args[0], args);
			}
			exit(0);
		}
		else {
			wait(&pid);
			printf("%s\n", command);
			printf("%s\n", target->TargetName);
			// ERROR CHECKING should go here.
		}

		return 0;
	}
	return 1;
}

/*-------------------------------------------------------END OF HELPER FUNCTIONS-------------------------------------*/


/*-------------------------------------------------------MAIN PROGRAM------------------------------------------------*/
//Main commencement
int main(int argc, char *argv[])
{
  target_t targets[MAX_NODES];
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

	  	  case 'h':
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


	for (int i=0; i<nTargetCount; i++) { // Loop through t
		printf("	----------\n	Target #%d\n	----------\n", i+1);
		if (build(&targets[i], targets, nTargetCount)==-1) {
			printf("Error in makefile.\n");
			break;
		}
	}

  /*End of your code*/
  //End of Phase2------------------------------------------------------------------------------------------------------

  return 0;
}
/*-------------------------------------------------------END OF MAIN PROGRAM------------------------------------------*/
