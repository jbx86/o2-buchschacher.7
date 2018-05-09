#include "proj7.h"

main(int argc, char *argv[]) {
	srand((time(NULL) + getpid()) % INT_MAX);

	printf("User %ld is running\n", (long)getpid());
	sleep((rand() % 10) + 1);
	printf("User %ld is done\n", (long)getpid());
	return 1;
}
