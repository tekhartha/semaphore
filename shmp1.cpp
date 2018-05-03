/* shmp1.cpp */

#include "registration.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <sys/sem.h>


using namespace std;

CLASS myclass = { "1001", "120186", "Operating Systems", 15 }; // Class Number, Date, Title, Seats left

#define NCHILD	3 // Define constant with global file scope

int	shm_init( void * );
void	wait_and_wrap_up( int [], void *, int );
void	rpterror( char *, char * );

main(int argc, char *argv[])
{
		
	int	child[NCHILD], i, shmid, semid; // declare ints
	void	*shm_ptr; // pointer to shared memory location
	char	ascshmid[10], ascsemid[10], pname[14];
	
	/***SEMAPHORE CODE***/
	semid = semget(ftok(".", 'u'), 1, IPC_CREAT | 0600); // create semaphore identifier
	semctl(semid, 0, SETVAL, 1); // set semaphore value of the first semaphore (semaphore 0) to 1
	/***SEMAPHORE CODE***/

	strcpy (pname, argv[0]); // strcpy(destination, source)
	shmid = shm_init(shm_ptr); // create shared memory segment ID
	sprintf (ascshmid, "%d", shmid);


	for (i = 0; i < NCHILD; i++) { // create 3 child processes
		child[i] = fork(); // put PIDS of child processes into child array
		switch (child[i]) {
		case -1:
			rpterror ("fork failure", pname);
			exit(1);
		case 0:
			sprintf (pname, "shmc%d", i+1); // print number of child process
			execl("shmc1", pname, ascshmid, (char *)0); // execl(path, args...) execute shmc1
			perror ("execl failed");
			exit (2);
		}
	}
	wait_and_wrap_up (child, shm_ptr, shmid); // Remove shared memory segments
}

/*
	Create identifier for shared memory segment
*/	
int shm_init(void *shm_ptr)
{
	int	shmid;
	/*
		shmget(key, size, flag)
		Allocate System V shared memory segment.
		-ftok: return key based on (path, id)
		-sizeof(CLASS): size of memory segment
		-0600: octal permission
		-IPC_CREAT: create new segment
	*/
	shmid = shmget(ftok(".",'u'), sizeof(CLASS), 0600 | IPC_CREAT);
	if (shmid == -1) {
		perror ("shmget failed");
		exit(3);
	}
	/*
		shmat(id, address, flag)
		Attach shared memory segment identified by ID to shared memory address.
	*/
	shm_ptr = shmat(shmid, (void * ) 0, 0);
	if (shm_ptr == (void *) -1) {
		perror ("shmat failed");
		exit(4);
	}
	/*
		memcpy(source ptr, destination ptr, count)
		Copy [count] bytes from source mem location to destination mem location. 
		
	*/
	memcpy (shm_ptr, (void *) &myclass, sizeof(CLASS) );
	return (shmid);
}


/*
	Remove the shared memory segment (children) associated with the shmid.
*/
void wait_and_wrap_up(int child[], void *shm_ptr, int shmid)
{
	int wait_rtn, w, ch_active = NCHILD;

	while (ch_active > 0) { // wait for all children to terminate
		wait_rtn = wait( (int *)0 );
		for (w = 0; w < NCHILD; w++)
			if (child[w] == wait_rtn) {
				ch_active--;
				break;
			}
	}
	cout << "Parent removing shm" << endl;
	/*
		shmdt(address)
		Detach the shared memory segment at given address
	*/
	shmdt (shm_ptr);
	/*
		shmcl(id, control operation, shmid_ds structure)
		Perform control operation on the shared mem segment that id refers to.
		-IPC_RMID: destroy segment
	*/
	shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0);
	exit (0);
}


/*
	Print formatted error message.
*/
void rpterror(char *string, char *pname)
{
	char errline[50];
	sprintf (errline, "%s %s", string, pname);
	perror (errline);
}

