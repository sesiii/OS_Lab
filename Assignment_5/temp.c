#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

#define SHMSZ     27

main()
{
    char c;
    int shmid;
    key_t key;
    printf("I am the leader\n");
    char *shm, *s;

    /*
     * We'll name our shared memory segment
     * "5678".
     */
    key = 5678;

    /*
     * Create the segment.
     */
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        
        perror("shmget");
        exit(1);
    }
    printf("shmid: %d\n", shmid);
    printf("Enter no of followers: ");
    int n;
    scanf("%d", n);
    int segment[n+3];
    printf("Waiting for followers to join...\n");

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    /*
     * Now put some things into the memory for the
     * other process to read.
     */
    s = shm;

    segment[0] = n;
    segment[1] = 0;
    segment[2]=0;
    segment[3]=0;
    for (int i = 0; i < n; i++)
    {
        segment[i + 4] = 0;
    }
    // for (c = 'a'; c <= 'z'; c++)
    //     *s++ = c;
    // *s = NULL;

    /*
     * Finally, we wait until the other process 
     * changes the first character of our memory
     * to '*', indicating that it has read what 
     * we put there.
     */
    while (*shm != '*')
        sleep(1);

    exit(0);
}