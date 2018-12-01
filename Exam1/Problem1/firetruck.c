// |  ___(_)        | |                 | |
// | |_   _ _ __ ___| |_ _ __ _   _  ___| | __
// |  _| | | '__/ _ \ __| '__| | | |/ __| |/ /
// | |   | | | |  __/ |_| |  | |_| | (__|   <
// \_|   |_|_|  \___|\__|_|   \__,_|\___|_|\_\


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
        semid = semget(my_key, 1, 0600);

    pid_t pid = fork();

    if (pid == 0) // child
    {

        semop(semid, &Flag_Incr, 1); // set flag

        printf("value1 : %d \n \n", semctl(semid, 0, GETVAL, 1));

        printf("value2: %d \n \n", semctl(semid, 1, GETVAL, 1));

        // ----- Locked Section Below -----
        semop(semid, &Decr, 1);

        printf("Fire Engine is entering the intersection \n");
        sleep(4);
        // --------------------------------

        semop(semid, &Incr, 1);
        semop(semid, &Flag_Decr, 1); // clear flag

        printf("Fire Engine is exiting the intersection \n");
        sleep(4);
        // ...
    }
    else // parent
    {
        wait(NULL);
    }

    return 0;
}
