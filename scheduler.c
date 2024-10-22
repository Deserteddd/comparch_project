#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <threads.h>

void insertionSort(int* arr)
{
    for (int i = 1; i < 4; ++i) {
        int key = arr[i];
        int j = i - 1;

        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

int main(int argc, char *argv[])
{
	printf("\n\n-------------------SCHEDULER----------------------\n");
	if (argc < 2)
	{
		printf("Missing arguments\n");
		return 1;
	}

	int key;
	if (sscanf(argv[1], "%d", &key) != 1)
	{
		printf("Frist argument must be an integer\n");
		return 1;
	}

	key_t ipc_key = ftok(argv[2], key);

	int segment_id;
	segment_id = shmget(ipc_key, 4*sizeof(int), IPC_CREAT | 0775);
	
	if (segment_id == -1) 
	{
		printf("Segment creation failed\n");
		return 1;
	}

	printf("Attaching with key: %i\n", ipc_key);
	int* block;
	block = shmat(segment_id, 0, 0);
	if (*block == -1)
	{
		printf("Failed to attach\n");
		return 1;
	}
	
	for (int i = 0; i<4; i++)
	{
		printf("Read %i from shared memory\n", block[i]);
	}
	
	// Sort the numbers and print them out
	insertionSort(block);
	printf("Sorted:\n[");
	for (int i = 0; i<4; i++)
	{
		printf((i < 3) ? "%i, " : "%i]\n", block[i]);
	}

	// Detach from shared memory
	shmdt(block);

	// Mark shared segment to be destroyed
	shmctl(segment_id, IPC_RMID, NULL);
	return 0;
}
