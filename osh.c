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
		invoke_error("Command exceeds maximum command length");
		num = -1;
	} else {
		a[num] = 0;
	}
	while (p != '\n') p = getchar();
	return num;
}

void print_history(char history[][MAX_LINE + 1], int history_count) {
	if (history_count == 0) {
		invoke_error("No history yet");
		return;
	}

	int i, j = 10;
	for (i = history_count; i > 0 && j > 0; i --, j--) {
		printf("%4d\t%s\n", i, history[i % 10]);
	}
}

int parse(char *buffer, int length, char* args[]) {
	int args_num = 0, last = -1, i;
	args[0] = NULL;
	for (i = 0; i <= length; ++i) {
		/* The last one must be zero so use <=*/
		if (buffer[i] != ' ' && buffer[i] != '\t' && buffer[i]) {
			continue;
		} else {
			if (last != i-1) {
				//store the parse result of [last+1, i-1]
				args[args_num] = (char*)malloc(i-last);
				if (args[args_num] == NULL) {
					invoke_error("Unable to allocate memory");
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
	return args_num;
}

int to_number(char *a) {
	int length = strlen(a), i, answer = 0;
	for (i = 0; i < length; i++) {
		if (a[i] > '9' || a[i] < '0') return -1;
		answer = answer * 10 + a[i] - '0';
	}
	return answer;
}

int main(void) {
	char *args[MAX_LINE/2 + 1]; /* command line arguments */
	int should_run = 1; /* flag to determine when to exit program */

	char history[10][MAX_LINE + 1];
	int history_count = 0;

	char buffer[MAX_LINE + 1];
	memset(buffer, 0, sizeof(buffer));
	int length, args_num;

	while (should_run) {
		printf("osh>");
		fflush(stdout);

		length = fetch_input(buffer);
		if (length == -1) continue;

		if (strcmp(buffer, "!!") == 0) {
			if (history_count > 0) {
				memcpy(buffer, history[history_count % 10], MAX_LINE + 1);
				length = strlen(buffer);
			} else {
				invoke_error("No commands in history");
				continue;
			}
		}

		args_num = parse(buffer, length, args);

		if (args_num == 0) continue;
		//No input message

		if (strcmp(args[0], "!") == 0) {
			int temp = to_number(args[1]);
			if (temp <= 0 || temp < history_count - 9 || temp > history_count) {
				invoke_error("No such command in history");
				continue;
			} else {
				memcpy(buffer, history[temp % 10], MAX_LINE + 1);
				length = strlen(buffer);
				args_num = parse(buffer, length, args);
			}
		}

		if (strcmp(args[0], "exit") == 0) {
			should_run = 0;
			continue;
		}

		if (strcmp(args[0], "history") == 0) {
			print_history(history, history_count);
			continue;
		}

		history_count ++;
		memcpy(history[history_count% 10], buffer, MAX_LINE + 1);

		//printf("%s\n", args[0]);
		//here to implement the new order

		int background = 0;
		if (strcmp(args[args_num-1], "&") == 0) {
			background = 1;
			args[--args_num] = NULL;
		}

		pid_t pid = fork();
		if (pid < 0) {
			invoke_error("Fork process error");
			return 1;
		}

		int status;

		if (pid == 0) {
			execvp(args[0], args);
			return 0;
		} else {
			if (background) {
				printf("pid #%d running in background %s\n", pid, buffer);
			} else {
				wait(&status);
			}
		}


		/**
		 * After reading user input, the steps are:
		 * (1) fork a child process using fork()
		 * (2) the child process will invoke execvp()
		 * (3) if command included &, parent will invoke wait()
		 */
	}

	return 0;
}