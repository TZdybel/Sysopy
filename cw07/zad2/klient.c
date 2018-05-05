//
// Created by tzdybel on 03.05.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

int N;
int *data;
sem_t *sem1;
sem_t *sem2;
sem_t *sem3;
int fd;

void gettime() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    printf("Czas: %ds %lims\n", (int)time.tv_sec, time.tv_nsec);
}

void init() {
    sem1 = sem_open("/sem1", O_RDWR);
    sem2 = sem_open("/sem2", O_RDWR);
    sem3 = sem_open("/sem3", O_RDWR);
    if(sem1 == SEM_FAILED || sem2 == SEM_FAILED || sem3 == SEM_FAILED) {
        printf("Cannot access semaphores\n");
        exit(1);
    }

    fd = shm_open("/sharmem", O_RDWR, 0);
    if (fd == -1) {
        printf("Cannot access shared memory\n");
        exit(1);
    }

    data = (int *)mmap(NULL, (N+4)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);   //przypisanie pamięci współdzielonej do wskaźnika data
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

int sprawdzCzySpi() {
    sem_wait(sem2);     //uzyskanie dostępu do stanu golibrody i poczekalni
    sem_wait(sem1);
    printf("Sprawdzam czy golibroda śpi - ID: %d\n", (int)getpid());
    gettime();
    return data[N+3];
}

void obudzGolibrod() {
    //jeśli tak, budzę go
    sem_wait(sem3);   //uzyskuję dostęp do fotela
    printf("Budzę golibrodę - ID: %d\n", (int)getpid());
    data[N+3] = 0;          //flaga stanu na obudzonego
    gettime();
    sem_post(sem2);
    sem_post(sem1);   //oddanie dostępu do stanu golibrody i do poczekalni
}

void ostrzyzSie(int version) {
    printf("Siadam na fotelu do strzyżenia - ID: %d\n", (int)getpid());
    if (version == 0) {
        data[0] = (int)getpid(); //siada na fotelu
        gettime();
        sem_post(sem3);   //zwolnienie dostępu do fotela
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

int idzDoPoczekalni() {
    sem_post(sem2);    //zwolnienie dostępu do stanu golibrody
    if(data[N+2] < data[N+1]) {   //czy jest miejsce w poczekalni
        //jeśli tak
        data[1+data[N+2]] = (int)getpid();  //ustawia się na pierwsze wolne miejsce
        data[N+2]++;
        printf("Zająłem miejsce nr %d w poczekalni - ID: %d\n", data[N+2], (int)getpid());
        gettime();
        sem_post(sem1);     //zwolnienie dostęp do kolejki
        int pid = (int)getpid();
        while(data[0] != pid) {}  //czeka, dopóki nie zostaniesz poproszony przez golibrodę
        data[N+4] = 1;            //ustawia flagę zrozumienia
        return 0;
    } else {
        sem_post(sem1);    //zwolnienie dostępu do kolejki
        printf("Brak miejsc. ");
        wyjdz();
        return -1;
    }
}

void wejdzDoSalonu() {
    if(sprawdzCzySpi() == 1) {
        obudzGolibrod();
        ostrzyzSie(0);
        wyjdz();
    } else {
        if (idzDoPoczekalni() == 0) {
            ostrzyzSie(1);
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

    for (int i = 0; i < numOfProcesses; ++i) {
        pid_t child = fork();
        if(child == 0) {
            for (int j = 0; j < numOfLoops; ++j) {
                wejdzDoSalonu();
            }
            if(munmap(data, (N+4)*sizeof(int)) == -1) {
                printf("Cannot detach shared memory\n");
            }
            exit(0);
        } else continue;
    }

    for (int k = 0; k < numOfProcesses; ++k) {  //czeka aż wszystkie procesy się zakończą
        wait(NULL);
    }

    if(munmap(data, (N+4)*sizeof(int)) == -1) {
        printf("Cannot detach shared memory\n");
    }

    return 0;
}