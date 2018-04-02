//
// Created by tzdybel on 02.04.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

int requests = 0;
int prints[5];

void requestCount(int signum) {
    requests++;
    if(prints[1] == 1) printf("Number of requests: %d\n", requests);
}

void sigReact(int signum) {
    requests++;
    if(prints[1] == 1) printf("Number of requests: %d\n", requests);
    kill(0, SIGALRM);
    if(prints[2] == 1) printf("Number of permissions send: %d\n", requests);
}

void closeAll(int signum) {
    kill(0, SIGINT);
    printf("Ending process...\n");
    exit(0);
}

void ignore(int signum) {
}

void RtSigReact(int signum) {
    if(prints[3] == 1) {
        if (signum - 34 == 0) printf("Received signal SIGRTMIN\n");
        else if (signum - 34 == 30) printf("Received signal SIGRTMAX\n");
        else printf("Received signal SIGRTMIN+%d\n", signum - 34);
    }
}

void setPrintTab(int *tab) {
    printf("What info you want to print?\n");
    printf("Write sequence of 5 ones and zeros for every following info which can be showed (e.g. 10010 or 11111): \n");
    printf("1.Tworzenie procesu potomnego (jego nr PID)\n");
    printf("2.Otrzymane prośby od procesów potomnych\n");
    printf("3.Wysłane pozwolenia na wysłanie sygnału rzeczywistego\n");
    printf("4.Otrzymane sygnały czasu rzeczywistego (wraz z numerem sygnału)\n");
    printf("5.Zakończenie procesu potomnego (wraz ze zwróconą wartością)\n");
    char *tmp1 = (char *)calloc(6, sizeof(char));
    scanf("%s", tmp1);
    int tmp2 = (int)strtol(tmp1, NULL, 10);
    for (int i = 4; i >= 0; --i) {
        int tmp3 = tmp2 % 10;
        tmp2 /= 10;
        if (tmp3 == 1) tab[i] = 1;
        else tab[i] = 0;
    }
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    signal(SIGINT, closeAll);
    signal(SIGUSR1, requestCount);
    signal(SIGALRM, ignore);

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigdelset(&set, SIGINT);
    sigdelset(&set, SIGALRM);
    for (int i = 0; i < 31; ++i) {
        sigdelset(&set, SIGRTMIN + i);
        signal(SIGRTMIN+i, RtSigReact);
    }
    sigprocmask(SIG_BLOCK, &set, NULL);

    setPrintTab(prints);
    int N = (int)strtol(argv[1], NULL, 10);
    int M = (int)strtol(argv[2], NULL, 10);

    for (int i = 0; i < N; ++i) {
        pid_t child_pid = fork();
        if (child_pid == 0) {
            srand(time(NULL) + getpid());
            int secs = rand()%11;

            sigset_t blockall = set;
            sigaddset(&blockall, SIGUSR1);
            sigaddset(&blockall, SIGALRM);
            sigprocmask(SIG_SETMASK, &blockall, NULL);
            sleep(secs);
            sigprocmask(SIG_SETMASK, &set, NULL);

            kill(getppid(), SIGUSR1);
            pause();
            kill(getppid(), SIGRTMIN+(rand()%31));
            exit(secs);
        } else {
            if (prints[0] == 1) printf("New process, PID: %d\n", (int) child_pid);
        }
    }
    while (requests < M-1) {}
    signal(SIGUSR1, sigReact);
    pause();
    for (int i = 0; i < N; ++i) {
        int status;
        pid_t pid = wait(&status);
        if(prints[4] == 1) printf("Process %d has ended - length of sleep: %d seconds\n", pid, WEXITSTATUS(status));
    }
    return 0;
}
