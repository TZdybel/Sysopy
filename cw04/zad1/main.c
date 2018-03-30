#include <stdio.h>
#include <time.h>
#include <zconf.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

pid_t child_pid;

void stpReact2(int signum) ;

void stpReact(int signum) {
    kill(child_pid, SIGKILL);
    printf("\nOczekuję na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");
    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, SIGTSTP);
    sigprocmask(SIG_UNBLOCK, &set, &oldset);
    signal(SIGTSTP, stpReact2);
    pause();
}

void stpReact2(int signum) {
    printf("\n");
    raise(SIGALRM);
    signal(SIGTSTP, stpReact);
}

void intReact(int signum) {
    if (child_pid != 0) {
        kill(child_pid, SIGKILL);
        printf("\nOdebrano sygnał SIGINT\n");
        printf("Kończę...\n");
    }
    exit(0);
}

int main() {
    sigset_t set, oldset;
    sigfillset(&set);
    sigdelset(&set, SIGTSTP);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, &oldset);

    signal(SIGTSTP, stpReact);
    struct sigaction sigaction1;
    sigaction1.sa_handler = intReact;
    sigaction1.sa_mask = set;
    sigaction1.sa_flags = 0;
    sigaction(SIGINT, &sigaction1, NULL);

    while(1) {
        child_pid = fork();
        if (child_pid == 0) {
            execl("./date", "date", NULL);
            exit(0);
        } else {
            wait(NULL);
        }
    }
    return 0;
}