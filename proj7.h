#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NPS 1000000000
#define CLKKEY 1212
#define FRMKEY 2323
#define PGTKEY 3434
#define MSGKEY 4545
#define MSGSIZE 256
#define MEMSIZE 256

// Structures

typedef struct {
	int usebit;
	int dirtybit;
	int pagenum;
} frame_t;

typedef struct {
	unsigned int sec;
	unsigned int nano;
} simtime_t;

typedef struct msgbuf {
	long mtype;
	char mtext[MSGSIZE];
} message_buf;

// Functions

void addTime(simtime_t *t, int n) {
	t->sec += (t->nano + n) / NPS;
	t->nano = (t->nano + n) % NPS;
}

void printTime(simtime_t time) {
	printf("%ld:%09ld\n", time.sec, time.nano);
}

int compTime(simtime_t x, simtime_t y) {
	if (x.sec > y.sec)
		return 1;
	else if (x.sec < y.sec)
		return 2;
	else {
		if (x.nano > y.nano)
			return 1;
		else if (x.nano < y.nano)
			return 2;
		else
			return 0;
	}
}
