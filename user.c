#include "proj7.h"

main(int argc, char *argv[]) {
	long int osspid = (long)getppid();
	long int userpid = (long)getpid();
	//int usernum = atoi(argv[1]);
	int usernum = 0;

	message_buf buf;        // Message content
	size_t buf_length;      // Message length

	int i;
	int clkid;
	int frmid;
	int pgtid;
	int msgid;
	int page;


	srand((time(NULL) + getpid()) % INT_MAX);

	// Shared clock setup
	simtime_t *simclock;
	if ((clkid = shmget(CLKKEY, sizeof(simtime_t), IPC_CREAT | 0666)) < 0) {
		perror("oss: shmget clkid");
		exit(1);
	}


	// Main memory frames in shared memory
	frame_t *frameTable;
	if ((frmid = shmget(FRMKEY, sizeof(frame_t)*MEMSIZE, 0666)) < 0) {
		perror("user: shmget frmid");
		exit(1);
	}
	frameTable = shmat(frmid, NULL, 0);

	// Page tables in shared memory
	int (*pageTable)[32];
	if ((pgtid = shmget(PGTKEY, sizeof(int)*18*32, 0666)) < 0) {
		perror("user: shmget pgtid");
		exit(1);
	}
	pageTable = shmat(pgtid, NULL, 0);
	for (i = 0; i < 32; i++) {
		pageTable[usernum][i] = usernum*100 + i;
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
	if ((msgid = msgget(MSGKEY, 0666)) < 0) {
		perror("user: msgget");
		exit(1);
	}

	// Chose a page to reference
	page = pageTable[usernum][rand() % 32];

	// Reference page
	buf.mtype = osspid;
	sprintf(buf.mtext, "%ld %d", userpid, page);
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
		perror("user: msgsnd");
		exit(1);
	}
	


	//printf("User %ld is running\n", (long)getpid());
	//sleep((rand() % 10) + 1);
	//printf("User %ld is done\n", (long)getpid());
	return 1;
}
