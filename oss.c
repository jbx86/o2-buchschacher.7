#include "proj7.h"

#define MAXCHILD 18
#define MAXFORKS 100

int clkid;	// Shared memory idea of system clock
int frmid;
int pgtid;
int msgid;
FILE *fp;

message_buf buf;        // Message content
size_t buf_length;      // Message length

void handler(int signo) {
	printf("OSS: Terminating by signal\n");
	shmctl(clkid, IPC_RMID, NULL);
	shmctl(frmid, IPC_RMID, NULL);
	shmctl(pgtid, IPC_RMID, NULL);
	msgctl(msgid, IPC_RMID, &buf);
	fclose(fp);
	exit(1);
}


int main(int argc, char *argv[]) {

	int i;			// for loop var
	int forkCount = 0;	// User processes that have been forked by OSS
	int termCount = 0;	// User processes that have successfully terminated
	int userCount = 0;	// User processes currently running
	int maxUsers;
	int nextUser = 0;
	int framePtr = 0;
	long int osspid = (long)getpid();


	simtime_t nextFork;	// Time of next user fork

	signal(SIGALRM, handler);
	alarm(5);

	fp = fopen("data.log", "w");

	if (argc != 2) {
		printf("Usage: %s processes \n", argv[0]);
		exit(1);
	}
	else {
		/* Verify argument value */
		maxUsers = atoi(argv[1]);
		if (maxUsers < 1) {
			printf("n should be a user greater than 0\n");
			exit(1);
		}
		else if (maxUsers > MAXCHILD) {
			maxUsers = MAXCHILD;
		}
	}

	//frame_t frameTable[MEMSIZE];
	pid_t userPid;
	pid_t userTable [maxUsers];

//-------------------------
        //mem_page *memPage;
        //sem_t sem;
        //pid_t userPid;
        //struct sembuf myop[2];

	// Shared clock setup
	simtime_t *simclock;
	if ((clkid = shmget(CLKKEY, sizeof(simtime_t), IPC_CREAT | 0666)) < 0) {
		perror("oss: shmget clkid");
		exit(1);
	}
	simclock = shmat(clkid, NULL, 0);
	simclock->sec = 0;
	simclock->nano = 0;

	// Main memory frames in shared memory
	frame_t *frameTable;
	if ((frmid = shmget(FRMKEY, sizeof(frame_t)*MEMSIZE, IPC_CREAT | 0666)) < 0) {
		perror("oss: shmget frmid");
		exit(1);
	}
	frameTable = shmat(frmid, NULL, 0);

	// Page tables in shared memory
	int (*pageTable)[32];
	if ((pgtid = shmget(PGTKEY, sizeof(int)*maxUsers*32, IPC_CREAT | 0666)) < 0) {
		perror("oss: shmget pgtid");
		exit(1);
	}
	pageTable = shmat(pgtid, NULL, 0);
	int j;
	for (i = 0; i < maxUsers; i++) {
		for (j = 0; j < 32; j++) {
			pageTable[i][j] = i * 100 + j;
		}
	}

/*
	// Memory pages setup
	if ((mpgid = shmget(MPGKEY, MEMSIZE*sizeof(mem_page), IPC_CREAT | 0666)) < 0) {
		perror("oss: shmget mpgid");
		exit(1);
	}
	memPage = shmat(mpgid, NULL, 0);
	for (i = 0; i < MEMSIZE; i++) {
		memPage[i].dirty = -1;
		memPage[i].address = 1;
	}

	// Semaphore setup: One for each user and one for oss
	if ((semid = semget(SEMKEY, (maxUser + 1), IPC_CREAT | 0666)) < 0) {
		perror("oss: semget");
		exit(1);
	}
*/

	// Message queue in shared memeory
	if ((msgid = msgget(MSGKEY, IPC_CREAT | 0666)) < 0) {
		perror("oss: msgget");
		raise(SIGINT);
	}

//------------------------

	// Initialize userTable as empty
	for (i = 0; i < maxUsers; i++) {
		userTable[i] = 0;
	}

	// Initialize frameTable
	for (i = 0; i < MEMSIZE; i++) {
		frameTable[i].usebit = 0;
		frameTable[i].pagenum = -1;
		frameTable[i].dirtybit = 0;
	}

	// Randomly generate next fork time
	nextFork = *simclock;
	addTime(&nextFork, (rand() % 500) + 1);
	printf("\tNext fork at:\n\t");
	printTime(nextFork);

	// Main loop
	while (termCount < MAXFORKS) {

		// Child management:

		// Check if any users have terminated
		for (i = 0; i < maxUsers; i++) {
			if (userTable[i] > 0) {
				// Check if user has terminated
				if (waitpid(userTable[i], NULL, WNOHANG) > 0) {
					userCount--;
					termCount++;
					printf("%ld has terminated, term number %d\n", (long)userTable[i], termCount);
					userTable[i] = 0;
				}
			}
		}

		// Check if it's time to fork another user
		if (compTime(nextFork, *simclock) == 2) {
			if ((userCount < maxUsers) && (forkCount < MAXFORKS)) {
				userPid = forkUser(forkCount);
				userCount++;
				forkCount++;
				nextUser = emptyUser(userTable, maxUsers);
				userTable[nextUser] = userPid;
				printf("User pid %ld stored at location %d, fork number %d\n", (long)userPid, nextUser, forkCount);
			}
			nextFork = *simclock;
			addTime(&nextFork, (rand() % 500) + 1);
			//printf("\tNext fork at:\n\t");
			//printTime(nextFork);
		}
		addTime(simclock, 20);
		//printf("System time:\n");
		//printTime(*simclock);
/*
		// Recieve message from user processes
		if (msgrcv(msgid, &buf, MSGSIZE, osspid, IPC_NOWAIT) != (ssize_t)-1) {
			char *ptr;
			pid_t senderpid = (pid_t)strtol(buf.mtext, &ptr, 10);	// Get pid of user sending message
			int page = (int)strtol(buf.mtext, &ptr, 10);		// Get page user is trying to reference

			// Check if page is currently in a frame
			int frame = findPage(page, frameTable);
			// If page is not in a frame, find an emptry/unused frame and write it to that frame
			if (frame == -1) {
				framePtr = findFrame(frame, frameTable);
				framePtr = insertPage(page, framePtr, frameTable, simclock);
			}
		}
*/

	}
	shmctl(clkid, IPC_RMID, NULL);
	shmctl(frmid, IPC_RMID, NULL);
	shmctl(pgtid, IPC_RMID, NULL);
	msgctl(msgid, IPC_RMID, &buf);

	fclose(fp);
	exit(0);
}

pid_t forkUser(int usernum) {
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

int emptyUser(pid_t *elem, int maxUsers) {
	int i;
	for (i = 0; i < maxUsers; i++) {
		if (elem[i] == 0) {
			return i;
		}
	}
}

// Seach frames for a page number
int findPage(int page, frame_t *f) {
	int i;
	// Check memory for page being referenced
	for (i = 0; i < MEMSIZE; i++) {
		// If page is found, set usebit
		if (f[i].pagenum == page) {
			//if[i].usebit = 1;
			printf("page %d found in frame %d\n", page, i);
			return i;
		}
	}
	// If page is not found, return -1 signifying segfault
	printf("page %d is not in the table\n", page);
	return -1;
}

// Search for an open/unused frame
int findFrame(int f, frame_t *table) {
	// Find next frame with unset usebit
	while (table[f].usebit != 0) {
		table[f].usebit = 0;
		f = ++f % MEMSIZE;
	}
	return f;	// Return frame number
}

// 
int insertPage(int page, int frame, frame_t *table, simtime_t simclock) {
	if (table[frame].pagenum != -1) {
		printf("Page fault\n");
	}
	else {
		simclock.nano += 10;
		printTime(simclock);
	}
	table[frame].usebit = 1;
	table[frame].dirtybit = 0;
	table[frame].pagenum = page;
	return ++frame % MEMSIZE;
}
