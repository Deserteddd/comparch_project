#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/shm.h>

// Will only be executed by child processes P1-P4
int childishBehaviour(int fd[2]);

int parenting(int read_ends[], char *argv[]);

int main(int argc, char* argv[])
{
	printf("\n\n---------------------INIT-------------------------\n");
	
	if (argc < 2)
	{
		printf("Missing arguments\n");
		return 1;
	}

	// Create 4 child processes and store addresses of their pipe read ends
	int read_ends[4];
	int id = 1;
	int i;
	for(i = 0; i<4; i++) 
	{
		if (id != 0)
		{
			int fd[2];
			if (pipe(fd) == -1) {
				printf("Plumbing error\n");
				return 1;
			}

			read_ends[i] = fd[0]; 
			id = fork();
			if (id == 0)
			{
				return childishBehaviour(fd);
			} else close(fd[1]);
		}
	}

	// Only parenting from here on out
	usleep(1000);


	return parenting(read_ends, argv);
}

int childishBehaviour(int fd[2])
{
	// Close read end of pipe since it won't be used
	close(fd[0]);
	
	// Init rand with process id
	srand(getpid());
	
	// Generate random number in range 0..20
	int r = rand() % 20;

	printf("Child chose %d and will send it to their parent.\n", r);
	
	// Write random number to pipe
	write(fd[1], &r, sizeof(int));
	
	// Close the pipe
	close(fd[1]);
	return 0;
}

int parenting(int read_ends[], char *argv[])
{
	printf("\nParenting time:\n");

	int key;
	if (sscanf(argv[1], "%d", &key) != 1)
	{
		printf("Argument must be an integer\n");
		return 1;
	}
	
	// Generate a key for ipc
	key_t ipc_key; 
	ipc_key = ftok(argv[2], key);

	// Create shared memory segment
	int segment_id = shmget(ipc_key, 4*sizeof(int), IPC_CREAT | 0775);
	if (segment_id == -1) 
	{
		printf("Segment creation failed\n");
		return 1;
	}

	printf("Attaching with key: %i\n", ipc_key);
	
	// Attach to shared memory
	int* block;
	block = shmat(segment_id, 0, 0);
	if (*block == -1)
	{
		printf("Failed to attach\n");
		return 1;
	}

	// Read messages passed by child processes and write them shared memory
	int j;
	for (j = 0; j<4; j++)
	{
		int message;
		read(read_ends[j], &message, sizeof(int));
		printf("Received number %d from child. Writing to shared memory...\n", message);
		block[j] = message;
		close(read_ends[j]);
	}

	// Detach from shared memory
	if (shmdt(block) == -1)
		printf("Failed to detach from shared memory\n");

	return 0;
}
