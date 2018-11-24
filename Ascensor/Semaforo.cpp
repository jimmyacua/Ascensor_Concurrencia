//
// Created by jimmy on 11/09/18.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include "Semaforo.h"

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

/*struct my_sembuf{
unsigned short sem_num;  // semaphore number
short          sem_op;   // semaphore operation
short          sem_flg;  // operation flags
};*/

Semaforo::Semaforo(int v){
    id = semget(KEY, 1, IPC_CREAT|0600); // 0600 para dar permisos de lectura y escritura
    if(-1 == id){ //hubo error
        perror("semaforo::constructor");
        exit(2);
    }

    union semun a;
    a.val = v;
    int st = semctl(id, 0, SETVAL, a);
    if(-1 == st){ //hubo error
        perror("semaforo::union");
        exit(2);
    }
}

Semaforo::~Semaforo(){
    int st = semctl(id, 0, IPC_RMID);
    if(-1 == st){ //hubo error
        perror("Semaforo::destructor");
        exit(2);
    }
}

int Semaforo::Signal(){
    struct sembuf s;
    s.sem_num = 0;
    s.sem_op = 1;
    s.sem_flg = 0;

    int st = semop(id, &s, 1);
    if(-1 == st){
      perror("Semaforo::Signal");
      exit(0);
    }
    return st;
}

int Semaforo::Wait(){
    struct sembuf s;
    s.sem_num = 0;
    s.sem_op = -1;
    s.sem_flg = 0;
    int st = semop(id, &s, 1);
    if(-1 == st){
      perror("Semaforo::Wait");
      exit(0);
    }
    return st;
}

