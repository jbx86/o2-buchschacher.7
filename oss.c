#include "proj7.h"

#define MAXUSERS 18
#define MAXFORKS 100

int main(int argc, char *argv[]) {

	int i;
	int forkCount = 0;	// User processes that have been forked by OSS
	int termCount = 0;	// User processes that have successfully terminated
	int userCount = 0;	// User processes currently running
	int nextUser = 0;

	pid_t userPid, x;
	pid_t userTable [MAXUSERS];

	// Initialize userTable as empty
	for (i = 0; i < MAXUSERS; i++) {
		userTable[i] = 0;
	}

	// Main loop
	while (termCount < MAXFORKS) {

		// Fork a user process and store in table if MAXFORKS and MAXUSERS haven't been reached yet
		while ((forkCount < MAXFORKS) && (userCount < MAXUSERS)) {
			userPid = forkUser();
			if (userPid > 0) {
				userCount++;
				forkCount++;
				nextUser = emptyUser(userTable);
				userTable[nextUser] = userPid;
				printf("User pid %ld stored at location %d, fork number %d\n", (long)userPid, nextUser, forkCount);
			}
		}



		// Critcal section



		// Check if any users have terminated
		for (i = 0; i < MAXUSERS; i++) {
			if (userTable[i] > 0) {
				x = waitpid(userTable[i], NULL, WNOHANG);
				if (x > 0) {
					userTable[i] = 0;
					userCount--;
					termCount++;
					printf("%ld has terminated, term number %d\n", (long)x, termCount);
				}
			}
		}
	}

	exit(0);
}

pid_t forkUser() {
	pid_t pid = fork();
	if (pid == 0) {
		execl("./user", "./user", NULL);
	}
	else if (pid > 0) {
		return pid;
	}
	else {
		perror("oss: fork");
		exit(1);
	}	
}

int emptyUser(pid_t * elem) {
	int i;
	for (i = 0; i < MAXUSERS; i++) {
		if (elem[i] == 0) {
			return i;
		}
	}
}
