/***********************************************************************
name:
	assignment4 -- acts as a pipe using ":" to seperate programs.
description:	
	See CS 360 Processes and Exec/Pipes lecture for helpful tips.
***********************************************************************/

/* Includes and definitions */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

/**********************************************************************
./assignment4 <arg1> : <arg2>

    Where: <arg1> and <arg2> are optional parameters that specify the programs
    to be run. If <arg1> is specified but <arg2> is not, then <arg1> should be
    run as though there was not a colon. Same for if <arg2> is specified but
    <arg1> is not.
**********************************************************************/


int main(int argc, char *argv[]){
	
	if (argc <= 1) {
		return 0;
	}

	// var init
	int fd[2];
	int rdr, wtr = 0; 
	int coli = 1;

	// find ':' input
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], ":") == 0) {
			coli = i;
			break;
		}
	}

	// get left...
	char *leftArgs[coli];
	for (int i = 0; i < coli-1; i++) {
		leftArgs[i] = argv[i+1];
	}
	leftArgs[coli-1] = NULL;
	
	// ... and right arguments (for exec)
	int rSize = argc - coli;
	char *rightArgs[rSize];
	for (int i = coli+1; i < argc; i++) {
		rightArgs[i-coli-1] = argv[i];
	}
	rightArgs[rSize-1] = NULL;
	
	/*
	// for printing/debugging input
	printf("Left:\n");
	for (int i = 0; i < coli; i++) {
		printf(" %s", leftArgs[i]);
	}
	printf("\nRight:\n");
	for (int i = 0; i < argc - coli; i++) {
		printf(" %s", rightArgs[i]);
	}
	printf("\n");
	*/

	if (rightArgs[0] == NULL) {
		if (execvp(argv[0], leftArgs) == -1) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
		exit(0);
	}

	if (pipe(fd) != 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit(0);
	}

	rdr = fd[0];
	wtr = fd[1];
	
	int f = fork();

	if (f < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}

	if (f) {
		// parent
		close(wtr);

		// make the file descriptor rdr the new stdin
		close(0);
		dup(rdr);
		close(rdr);

		wait(NULL);

		// execute stuff after ':' input
		// why does this exec not show output to stdout?
		if (execvp(argv[0], rightArgs) == -1) { 
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}

		// end process
		exit(0);
	} else {
		// child
		close(rdr);

		// make the file descriptor wtr the new stout
		close(1);
		dup(wtr);
		close(wtr);

		// execute stuff before ':' input
		if (execvp(argv[0], leftArgs) == -1) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}

		// end process
		close(wtr);
		exit(0);
	}

	return 0;
}