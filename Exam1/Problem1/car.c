// /  __ \           
// | /  \/ __ _ _ __
// | |    / _` | '__|
// | \__/\ (_| | |
//  \____/\__,_|_|

#include <stdio.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    static struct sembuf Decr = {0, -1, SEM_UNDO};
    static struct sembuf Incr = {0, 1, SEM_UNDO};

    static struct sembuf Flag_Decr = {1, -1, SEM_UNDO};
    static struct sembuf Flag_Incr = {1, 1, SEM_UNDO};

    key_t my_key;
    int semid;
    my_key = ftok("key", 'A');

    printf("This is my key: %d \n", my_key);

    // CREATION TRICK
    // whoever does the create does the initialization
    // request 1 semaphor for only my program
    if ((semid = semget(my_key, 2, IPC_CREAT | IPC_EXCL | 0600)) != -1)
    {
        // creator only
        semctl(semid, 0, SETVAL, 1);
        semctl(semid, 1, SETVAL, 0); // initialize flag to zero
    }
    else
        // non creators failed
        semid = semget(my_key, 2, 0600);

    pid_t pid = fork();

    if (pid == 0) // child
    {
        printf("value1 : %d \n \n", semctl(semid, 0, GETVAL, 1));

        printf("value2: %d \n \n", semctl(semid, 1, GETVAL, 1));

        // ----- Locked Section Below -----
        semop(semid, &Decr, 1);

        if (semctl(semid, 1, GETVAL, 1) == 1) // if flag is set them just stall
        {
            semop(semid, &Incr, 1);
        }
        else
        {

            printf("Car is entering the intersection \n");
            sleep(4);
            printf("Car is exiting the intersection \n");
            semop(semid, &Incr, 1);
            sleep(4);
        }
        // --------------------------------

        // if (semctl(semid, 1, GETVAL, 1) == 0) // check if the flag is set
        // {
        // }
    }
    else // parent
    {
        wait(NULL);
    }

    return 0;
}
