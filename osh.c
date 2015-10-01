#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LINE 80 /* The maximum length command */

void invoke_error(char *a) {
	printf("osh: %s >_< \n", a);
}

int fetch_input(char *a) {
	char p;
	int num  = 0;

	while (((p = getchar()) != '\n') && (num < MAX_LINE)) {
		a[num++] = p;
	}

	if (num == MAX_LINE && p != '\n') {
		invoke_error("command exceeds maximum command length");
		num = -1;
	} else {
		a[num] = 0;
	}
	while (p != '\n') p = getchar();
	return num;
}

int main(void) {
	char *args[MAX_LINE/2 + 1]; /* command line arguments */
	int should_run = 1; /* flag to determine when to exit program */

	char buffer[MAX_LINE + 1];
	memset(buffer, 0, sizeof(buffer));
	int length, args_num, i, last;

	while (should_run) {
		printf("osh>");
		fflush(stdout);

		length = fetch_input(buffer);
		if (length == -1) continue;

		args_num = 0; last = -1; args[0] = NULL;

		for (i = 0; i <= length; ++i) {
			/* The last one must be zero so use <=*/
			if (buffer[i] != ' ' && buffer[i] != '\t' && buffer[i]) {
				continue;
			} else {
				if (last != i-1) {
					//store the parse result of [last+1, i-1]
					args[args_num] = (char*)malloc(i-last);
					if (args[args_num] == NULL) {
						invoke_error("unable to allocate memory");
						return 1;
					}
					//malloc size+1
					memcpy(args[args_num], &buffer[last+1], i-last-1);
					//
					args[args_num][i-last] = 0;
					//set the last char zero

					args[++args_num] = NULL;
				}
				last = i;
			}
		}

		if (args_num == 0) continue;
		//No input message
		if (strcmp(args[0], "exit") == 0) {
			//Bye Bye
			should_run = 0;
			continue;
		}

		printf("%s\n", args[0]);


		/**
		 * After reading user input, the steps are:
		 * (1) fork a child process using fork()
		 * (2) the child process will invoke execvp()
		 * (3) if command included &, parent will invoke wait()
		 */
	}

	return 0;
}