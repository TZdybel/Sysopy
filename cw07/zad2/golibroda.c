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

int N;            //N miejsc w poczekalni + 1 fotel do strzyżenia

int *data;        //0 - fotel, 1 do N - miejsca w poczekalni,
                  // N+1 - ilość miejsc w poczekalni, N+2 - ilość zajętych miejsc,
                  // N+3 - flaga czy śpi, N+4 - flaga zrozumienia z klientem
sem_t *sem1;
sem_t *sem2;
sem_t *sem3;
int fd;

void gettime() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    printf("Czas: %ds %lims\n", (int)time.tv_sec, time.tv_nsec);
}

void strzyz() {
    sem_wait(sem3);      //"uzyskuję dostęp" do danych fotela (data[0])
    printf("Zabieram się za strzyżenie klienta o ID: %d\n", data[0]);
    gettime();
    printf("Obstrzygłem klienta o ID: %d\n", data[0]);
    data[N+4] = 0;      //ustawienie flagi potwierdzenia na 0
    data[0] = 0;        //zakonczenie strzyżenia
    gettime();
    sem_post(sem3);     //zwolnienie dostępu
    while (data[N+4] != 1) {}  //czeka na potwierdzenie przez klienta
    data[N+4] = 0;   //ustawienie flagi na 0
}

void spanko() {
    sem_post(sem1);      //zwolnienie dostępu do poczekalni
    printf("Idę spać\n");
    gettime();
    data[N+3] = 1;          //ustawienie flagi spania na 1
    sem_post(sem2);         //zwolnienie dostępu do informacji o stanie golibrody
    while (data[N+3] == 1) {} //zasypia
    printf("Budzę się\n");
    gettime();
}

int sprawdzPoczekalnie() {
    sem_wait(sem2);      //uzyskanie dostępu do danych kolejki i stanu golibrody
    sem_wait(sem1);
    printf("Sprawdzam czy jest ktoś w poczekalni\n");
    if (data[1] != 0) {     //jeśli jest ktoś w poczekalni
        //zaproś do strzyżenia
        printf("Zaprosiłem do strzyżenia nastepnego klienta o ID: %d\n", data[1]);
        for (int i = 0; i < N; ++i) {      //kolejka przesuwa się o 1, ostatnie miejsce się zwalnia
            data[i] = data[i+1];
        }
        data[N] = 0;
        data[N+2]--;
        gettime();
        while (data[N+4] != 1) {}          //czeka na potwierdzenie od klienta, na którego przyszła kolej na strzyżenie
        return 1;
    } else return 0;
}

void otworzSalon() {
    while (1) {
        if (sprawdzPoczekalnie() == 1) {
            sem_post(sem1);
            sem_post(sem2);   //zwolnienie dostępu do danych poczekalni i stanie golibrody
            strzyz();
        } else {
            spanko();
            strzyz();
        }
    }
}

void sigReact(int signum) {
    //usuwanie semaforów i pamięci wspólnej
    printf("\nReceived signal no.%d\nZamykanie salonu...\n", signum);
    if(munmap(data, (N+4)*sizeof(int)) == -1) {
        printf("Cannot detach shared memory\n");
    }
    if(shm_unlink("/sharmem") == -1) {
        printf("Cannot delete shared memory segment\n");
    }
    if(sem_close(sem1) == -1 || sem_close(sem2) == -1 || sem_close(sem3) == -1) {
        printf("Cannot close semaphores\n");
    }
    exit(0);
}

void init() {
    //utworzenie 3 semaforów
    sem1 = sem_open("/sem1", O_RDWR | O_CREAT, 0600, 1);    //dostęp do kolejki
    sem2 = sem_open("/sem2", O_RDWR | O_CREAT, 0600, 1);    //dostęp do stanu golibrody
    sem3 = sem_open("/sem3", O_RDWR | O_CREAT, 0600, 1);    //dostęp do fotela
    if(sem1 == SEM_FAILED || sem2 == SEM_FAILED || sem3 == SEM_FAILED) {
        printf("Cannot create semaphores\n");
        exit(1);
    }

    fd = shm_open("/sharmem", O_RDWR | O_CREAT, 0600);  //utworzenie segmentu pamięci wspólnej
    if (fd == -1) {
        printf("Cannot create shared memory\n");
        exit(1);
    }

    if (ftruncate(fd, (N+4)*sizeof(int)) == -1) {       //określenie rozmiaru segmentu (N+4)*int
        printf("Cannot allocate data for shared memory\n");
        exit(1);
    }

    data = (int *)mmap(NULL, (N+4)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);   //przypisanie pamięci współdzielonej do wskaźnika data
    if (data == (int *)(-1)) {
        printf("Problem with connecting to shared memory\n");
        exit(1);
    }

    for (int i = 0; i < N+5; ++i) data[i] = 0;         //inicjalizacja danych w data
    data[N+1] = N;
}

void setSignals() {
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);
    signal(SIGTERM, sigReact);
    signal(SIGINT, sigReact);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    setSignals();

    N = (int)strtol(argv[1], NULL, 10);

    init();

    otworzSalon();

    return 0;
}