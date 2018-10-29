#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "semun.h"

#include <time.h>
#include <sys/shm.h>

#include <sys/wait.h>
#include <sys/timeb.h>

#include <signal.h>
#include <string.h>

#define TEXT_SZ 128

/* ================================= SEMAPHORES ==================================== */

int semCreate(key_t key, int nsems, int val)
{
    int semid, i;
    union semun arg;
    int error;

    if ((key < 0) || (nsems <= 0))
        return -1;

    semid = semget(key, nsems, 0666 | IPC_CREAT);

    if (semid < 0)
        return -1;

    arg.val = val;

    for (i = 0; i < nsems; i++)
    {

        error = semctl(semid, i, SETVAL, arg);
        if (error < 0)
            return -1;
    }

    return semid;
}

int semDown(int semid, int sem_num)
{
    struct sembuf sb;

    if ((semid < 0) || (sem_num < 0))
        return -1;

    sb.sem_num = sem_num;
    sb.sem_op = -1;
    sb.sem_flg = 0;

    return semop(semid, &sb, 1);
}

int semUp(int semid, int sem_num)
{
    struct sembuf sb;

    if ((semid < 0) || (sem_num < 0))
        return -1;

    sb.sem_num = sem_num;
    sb.sem_op = 1;
    sb.sem_flg = 0;

    return semop(semid, &sb, 1);
}

int semDel(int semid)
{

    if (semid < 0)
        return -1;

    return semctl(semid, 0, IPC_RMID);
}

int Set(int semid, int n, int value)
{
    union semun arg;

    if (semid < 0 || n < 0)
        return -1;

    arg.val = value;

    return semctl(semid, n, SETVAL, arg);
}

int Get(int semid, int n)
{
    union semun arg;

    if (semid < 0 || n < 0)
        return -1;

    return semctl(semid, n, GETVAL, arg);
}

/* ====================== SHARED MEMORY =============================== */

typedef struct data
{ //shared memory storages a struct
    char val[TEXT_SZ];
} MemData;

int ShMCreate(key_t key)
{

    if (key < 0)
    {
        printf("*** key error ***");
        return -1;
    }

    return shmget(key, sizeof(MemData), IPC_CREAT | 0666);
}

MemData *ShMAttach(int ShmID)
{
    return shmat(ShmID, (void *)0, 0);
}

int ShMDetach(MemData *ShmPTR)
{
    return shmdt(ShmPTR);
}

int ShMDelete(int ShmID)
{
    return shmctl(ShmID, IPC_RMID, 0);
}

/* =============================== MAIN =============================================== */
// To use in finally method 
int ShmID;
int Mutex_ID;
MemData *ShmPTR;

void finally()
{
    printf("\n\n Finally...");  

    if (ShMDetach(ShmPTR) == -1)
    { //Detaching and Deleting SharedMem
        printf(" ***shmdt failed***\n");
        exit(1);
    }
    if (ShMDelete(ShmID) == -1)
    {
        printf(" ***shmctl(IPC_RMID) failed***\n");
        exit(1);
    }

    semDel(Mutex_ID); //Deleting semaphores

    sleep(5);
    exit(0); //parent exits
}

int main(int argc, char *argv[])
{
    //Captura o Ctrl+C
    struct sigaction act;
    act.sa_handler = finally;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);                            

    srand(time(NULL));

    //VARIABLES
    key_t key;
    int p, i, j, o;
    char c;

    if (argc < 1)
    {
        printf("Error of argc");
    }
    else if (argc < 2)
    {
        printf("Error of argc(1)");
        exit;
    }
    else
    {
        o = atoi(argv[1]);
    }

    //create the key
    if ((key = ftok("main.c", 'R')) == -1)
    { //producing a random key for shared memory
        printf("*** ftok error ***\n");
        exit(1);
    }

    ShmID = ShMCreate(key); //Creating and Attaching shared memory segment
    if (ShmID < 0)
    {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }
    ShmPTR = ShMAttach(ShmID);
    if (ShmPTR == (MemData *)(-1))
    {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }

    Mutex_ID = semCreate((key_t)1234, 1, 1); //Create a binary semaphore
    if (Mutex_ID < 0)
    {
        printf("***semget error***\n");
        exit(0);
    }

    if (o == 1) // Escritor
    {
        while (strlen(ShmPTR->val) < TEXT_SZ)
        {
            printf("Writing... \n"); //Random char
            c = (char)((rand() % 23) + 68);
            printf("%c ", c);
            printf("\n");

            // Copy char to shared mem
            semDown(Mutex_ID, 0); //Block Writing

            char temp[1]; 
            temp[0] = c ; 
            strcat(ShmPTR->val, temp); //Writing on Shared Memory segment 

            semUp(Mutex_ID, 0);

            sleep(rand() % 10);
        }
    }
    else
    {
        while (1)
        {
            printf("Reading... \n");

            semDown(Mutex_ID, i); //Block i Process from reading the same data twice

            printf("%s \n", ShmPTR->val); //Reading

            semUp(Mutex_ID, 0); //Increase counting semaphore

            sleep(rand() % 10);
        }
    }
}
