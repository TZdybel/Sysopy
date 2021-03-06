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
#include <signal.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int N;            //N miejsc w poczekalni + 1 fotel do strzyżenia

int *data;        //0 - fotel, 1 do N - miejsca w poczekalni,
                  // N+1 - ilość miejsc w poczekalni, N+2 - ilość zajętych miejsc,
                  // N+3 - flaga czy śpi, N+4 - flaga zrozumienia z klientem
int semid;
int shmid;

void gettime() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    printf("Czas: %ds %lims\n", (int)time.tv_sec, time.tv_nsec);
}

void strzyz(struct sembuf *buf) {
    buf[0].sem_num = 2;
    buf[0].sem_op = 0;
    buf[1].sem_num = 2;
    buf[1].sem_op = 1;
    semop(semid, buf, 2);       //"uzyskuję dostęp" do danych fotela (data[0])
    printf("Zabieram się za strzyżenie klienta o ID: %d\n", data[0]);
    gettime();
    printf("Obstrzygłem klienta o ID: %d\n", data[0]);
    buf[0].sem_num = 2;
    buf[0].sem_op = -1;
    data[N+4] = 0;      //ustawienie flagi potwierdzenia na 0
    data[0] = 0;        //zakonczenie strzyżenia
    gettime();
    semop(semid, buf, 1);   //zwolnienie dostępu
    while (data[N+4] != 1) {}  //czeka na potwierdzenie przez klienta
    data[N+4] = 0;   //ustawienie flagi na 0
}

void spanko(struct sembuf *buf) {
    buf[0].sem_num = 0;
    buf[0].sem_op = -1;
    semop(semid, buf, 1);   //zwolnienie dostępu do poczekalni
    printf("Idę spać\n");
    gettime();
    data[N+3] = 1;          //ustawienie flagi spania na 1
    buf[0].sem_num = 1;
    buf[0].sem_op = -1;
    semop(semid, buf, 1);   //zwolnienie dostępu do informacji o stanie golibrody
    while (data[N+3] == 1) {} //zasypia
    printf("Budzę się\n");
    gettime();
}

int sprawdzPoczekalnie(struct sembuf *buf) {
    buf[0].sem_num = 1;
    buf[0].sem_op = 0;
    buf[1].sem_num = 1;
    buf[1].sem_op = 1;
    buf[2].sem_num = 0;
    buf[2].sem_op = 0;
    buf[3].sem_num = 0;
    buf[3].sem_op = 1;
    semop(semid, buf, 4);   //uzyskanie dostępu do danych kolejki i stanu golibrody
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

void otworzSalon(struct sembuf *buf) {
    while (1) {
        if (sprawdzPoczekalnie(buf) == 1) {
            buf[0].sem_num = 0;
            buf[0].sem_op = -1;
            buf[1].sem_num = 1;
            buf[1].sem_op = -1;
            semop(semid, buf, 2);  //zwolnienie dostępu do danych poczekalni i stanie golibrody
            strzyz(buf);
        } else {
            spanko(buf);
            strzyz(buf);
        }
    }
}

void sigReact(int signum) {
    //usuwanie semaforów i pamięci wspólnej
    printf("\nReceived signal no.%d\nZamykanie salonu...\n", signum);
    if(semctl(semid, 0, IPC_RMID) == -1) {
        printf("Cannot remove set of semaphores\n");
    }
    if(shmdt(data) == -1) {
        printf("Cannot detach shared memory\n");
    }
    if(shmctl(shmid, IPC_RMID, 0) == -1) {
        printf("Cannot remove shared memory\n");
    }
    exit(0);
}

void init() {
    key_t key = ftok("plik.txt", 4);   //utworzenie klucza
    if (key == -1) {
        printf("Cannot create valid key, does file with this name exists?");
        exit(1);
    }

    semid = semget(key, 3, 0600 | IPC_CREAT);  //utworzenie zbioru 3 semaforów
    if (semid == -1) {
        printf("Cannot create semaphores\n");
        exit(1);
    }

    union semun initsem;
    initsem.array = (unsigned short *)calloc(3, sizeof(unsigned short));
    initsem.array[0] = 0;  //semafor dający dostęp do kolejki
    initsem.array[1] = 0;  //semafor dający dostęp do info, czy golibroda śpi
    initsem.array[2] = 0;  //semafor dający dostęp do info na temat procesu strzyżenia
    semctl(semid, 0, SETALL, initsem);

    shmid = shmget(key, (N+4)*sizeof(int), 0600 | IPC_CREAT);  //wydzielenie obszaru na pamięć wspólną o wielkości (N+4)*int
    if (shmid == -1) {
        printf("Cannot create shared memory\n");
        exit(1);
    }

    data = (int *)shmat(shmid, NULL, 0);                        //przypisanie pamięci współdzielonej do wskaźnika data
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

    struct sembuf *buf = (struct sembuf *)calloc(4, sizeof(struct sembuf));  //tablica struktur odpowiedzialnych za wykonywanie operacji na semaforach
    for (int i = 0; i < 4; ++i) buf[i].sem_flg = 0;                          //ustawia flagi na 0

    otworzSalon(buf);

    return 0;
}