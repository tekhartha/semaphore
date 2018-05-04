/* shmc1.cpp */

#include "registration.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <sys/sem.h>

using namespace std;

CLASS *class_ptr;
void *memptr;
char *pname;
int shmid, ret;
void rpterror(char *), srand(), perror(), sleep();
void sell_seats();

key_t semkey;
int semid;
struct sembuf sbuf = {0, -1, 0}; // {semaphore number, operation, flag}

main(int argc, char* argv[])
{
	
	semid = semget(ftok(".", 'u'), 1, IPC_CREAT);
	
	if (argc < 2) {
		fprintf (stderr, "Usage:, %s shmid\n", argv[0]);
		exit(1);
	}
	
	pname = argv[0];
	sscanf (argv[1], "%d", &shmid); // read data from argv[1] w format %d, put into shmid
	/*
		shmat(id, address, flag)
		Attach shared memory segment identified by ID to shared memory address.
	*/
	memptr = shmat (shmid, (void *)0, 0);
	if (memptr == (char *)-1 ) {
		rpterror ("shmat failed");
		exit(2);
	}

	class_ptr = (struct CLASS *)memptr; // create pointer of type CLASS to memory segment

	sell_seats();
	
	/*
		shmdt(address)
		Detach the shared memory segment at given address
		Should return 0 on success
	*/
	ret = shmdt(memptr);
	exit(0);
}

void sell_seats()
{
	int all_out = 0;
	srand ( (unsigned) getpid() ); // initialize random number generator with a seed
	while ( !all_out) { // loop to sell all seats
		/***/
		sbuf.sem_op = -1; // semaphore wait/decrement (since semval = 1)
		semop(semid, &sbuf, 1); // perform operation
		/***/
		if (class_ptr->seats_left > 0) {
			sleep ( (unsigned)rand()%5 + 1); // thread sleeps for given number of seconds
			class_ptr->seats_left--; // decrement number of seats ("selling")
			sleep ( (unsigned)rand()%5 + 1);
			cout << pname << " SOLD SEAT -- " << class_ptr->seats_left << " left" << endl;
		}
		else {
			all_out++; // increment all_out to break out of loop
			cout << pname << " sees no seats left" << endl;
		}
		/***/
		sbuf.sem_op = 1; // semaphore signal/increment
		semop(semid, &sbuf, 1); // perform operation
		/***/
		sleep ( (unsigned)rand()%10 + 1);
	}
}

/*
	Print formatted error message.
*/
void rpterror(char* string)
{
	char errline[50];

	sprintf (errline, "%s %s", string, pname);
	perror (errline);
}

