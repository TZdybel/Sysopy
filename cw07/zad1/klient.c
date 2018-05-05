//
// Created by tzdybel on 03.05.18.
//

#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdlib.h>
#include <zconf.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/wait.h>

int N;
int *data;
int semid;
int shmid;

void gettime() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    printf("Czas: %ds %lims\n", (int)time.tv_sec, time.tv_nsec);
}

void init() {
    key_t key = ftok("plik.txt", 4);
    if (key == -1) {
        printf("Cannot create valid key, does file with this name exists?");
        exit(1);
    }

    semid = semget(key, 0, 0);
    if (semid == -1) {
        printf("Cannot access semaphores\n");
        exit(1);
    }

    shmid = shmget(key, 0, 0);
    if (shmid == -1) {
        printf("Cannot access shared memory\n");
        exit(1);
    }

    data = (int *)shmat(shmid, NULL, 0);
    if (data == (int *)(-1)) {
        printf("Problem with connecting to shared memory\n");
        exit(1);
    }

    for (int i = 0;; ++i) {   //przeszukuje data dopóki nie znajdzie czegoś innego niż 0 - czyli ilości krzeseł w poczekalni
        if(data[i] != 0) {    //można w bardziej przejrzysty sposób ale nie chciałem zmieniać
            N = data[i];
            break;
        }
    }
}

int sprawdzCzySpi(struct sembuf *buf) {
    buf[0].sem_num = 1;
    buf[0].sem_op = 0;
    buf[1].sem_num = 1;
    buf[1].sem_op = 1;
    buf[2].sem_num = 0;
    buf[2].sem_op = 0;
    buf[3].sem_num = 0;
    buf[3].sem_op = 1;
    semop(semid, buf, 4);   //uzyskanie dostępu do stanu golibrody i poczekalni
    printf("Sprawdzam czy golibroda śpi - ID: %d\n", (int)getpid());
    gettime();
    return data[N+3];
}

void obudzGolibrod(struct sembuf *buf) {
    //jeśli tak, budzę go
    buf[0].sem_num = 2;
    buf[0].sem_op = 0;
    buf[1].sem_num = 2;
    buf[1].sem_op = 1;
    semop(semid, buf, 2);   //uzyskuję dostęp do fotela
    printf("Budzę golibrodę - ID: %d\n", (int)getpid());
    data[N+3] = 0;          //flaga stanu na obudzonego
    buf[0].sem_num = 1;
    buf[0].sem_op = -1;
    buf[1].sem_num = 0;
    buf[1].sem_op = -1;
    gettime();
    semop(semid, buf, 2);   //oddanie dostępu do stanu golibrody i do poczekalni
}

void ostrzyzSie(struct sembuf *buf, int version) {
    printf("Siadam na fotelu do strzyżenia - ID: %d\n", (int)getpid());
    if (version == 0) {
        data[0] = (int)getpid(); //siada na fotelu
        buf[0].sem_num = 2;
        buf[0].sem_op = -1;
        gettime();
        semop(semid, buf, 1);    //zwolnienie dostępu do fotela
    } else {
        gettime();
    }
    while(data[0] != 0) {}   //czeka aż zostanie ostrzyżony
    data[N+4] = 1;           //wysyła potwierdzenie
    printf("Zostałem ostrzyżony - ID: %d\n", (int)getpid());
    gettime();
}

void wyjdz() {
    printf("Wychodzę - ID: %d\n", (int)getpid());
    gettime();
}

int idzDoPoczekalni(struct sembuf *buf) {
    buf[0].sem_num = 1;
    buf[0].sem_op = -1;
    semop(semid, buf, 1);   //zwolnienie dostępu do stanu golibrody
    if(data[N+2] < data[N+1]) {   //czy jest miejsce w poczekalni
        //jeśli tak
        data[1+data[N+2]] = (int)getpid();  //ustawia się na pierwsze wolne miejsce
        data[N+2]++;
        printf("Zająłem miejsce nr %d w poczekalni - ID: %d\n", data[N+2], (int)getpid());
        gettime();
        buf[0].sem_num = 0;
        buf[0].sem_op = -1;
        semop(semid, buf, 1);  //zwolnienie dostęp do kolejki
        int pid = (int)getpid();
        while(data[0] != pid) {}  //czeka, dopóki nie zostaniesz poproszony przez golibrodę
        data[N+4] = 1;            //ustawia flagę zrozumienia
        return 0;
    } else {
        buf[0].sem_num = 0;
        buf[0].sem_op = -1;
        semop(semid, buf, 1);    //zwolnienie dostępu do kolejki
        printf("Brak miejsc. ");
        wyjdz();
        return -1;
    }
}

void wejdzDoSalonu(struct sembuf *buf) {
    if(sprawdzCzySpi(buf) == 1) {
        obudzGolibrod(buf);
        ostrzyzSie(buf, 0);
        wyjdz();
    } else {
        if (idzDoPoczekalni(buf) == 0) {
            ostrzyzSie(buf, 1);
            wyjdz();
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    int numOfProcesses = (int)strtol(argv[1], NULL, 10);
    int numOfLoops = (int)strtol(argv[2], NULL, 10);

    init();

    struct sembuf *buf = (struct sembuf *)calloc(4, sizeof(struct sembuf));
    for (int i = 0; i < 4; ++i) buf[i].sem_flg = 0;

    for (int i = 0; i < numOfProcesses; ++i) {
        pid_t child = fork();
        if(child == 0) {
            for (int j = 0; j < numOfLoops; ++j) {
                wejdzDoSalonu(buf);
            }
            if(shmdt(data) == -1) {
                printf("Cannot detach shared memory\n");
            }
            exit(0);
        } else continue;
    }

    for (int k = 0; k < numOfProcesses; ++k) {  //czeka aż wszystkie procesy się zakończą
        wait(NULL);
    }

    if(shmdt(data) == -1) {
        printf("Cannot detach shared memory\n");
    }

    return 0;
}