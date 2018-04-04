//
// Created by tzdybel on 04.04.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <zconf.h>
#include <sys/wait.h>

int childSignalsReceived = 0;
int parentsignalsReceived = 0;

void intReact(int signum) {
    printf("Ending...\n");
    kill(0, SIGUSR2);
    exit(1);
}

void sigusr1React_type1(int signum) {
    kill(getppid(), SIGUSR1);
    childSignalsReceived++;
    printf("Number of signals received from parent: %d\n", childSignalsReceived);
}

void sigusr1React_type2(int signum) {
    childSignalsReceived++;
    printf("Number of signals received from parent: %d\n", childSignalsReceived);
    kill(getppid(), SIGUSR1);
}

void sigrtminReact(int signum) {
    kill(getppid(), SIGRTMIN);
    childSignalsReceived++;
    printf("Number of signals received from parent: %d\n", childSignalsReceived);
}

void sigusr2React(int signum) {
    printf("Child process ends work...\n");
    exit(0);
}

void parentSigusrReact(int signum) {
    parentsignalsReceived++;
    printf("Number of signals received from child: %d\n", parentsignalsReceived);
}

void setSigset(sigset_t* set, int type, int ifChild) {
    sigfillset(set);
    if (ifChild == 0) sigdelset(set, SIGINT);
    if (type == 3) {
        sigdelset(set, SIGRTMIN);
        sigdelset(set, SIGRTMAX);
        sigdelset(set, SIGUSR2);
    } else {
        sigdelset(set, SIGUSR1);
        sigdelset(set, SIGUSR2);
    }
    sigprocmask(SIG_SETMASK, set, NULL);
}

void type12(int numOfSignals, int type) {
    pid_t child_pid = fork();
    if (child_pid == 0) {
        sigset_t set;
        setSigset(&set, 1, 1);
        if (type == 1) signal(SIGUSR1, sigusr1React_type1);
        else signal(SIGUSR1, sigusr1React_type2);
        signal(SIGUSR2, sigusr2React);
        while(1) {};
    } else {
        sleep(1);
        signal(SIGUSR1, parentSigusrReact);
        for (int i = 0; i < numOfSignals; ++i) {
            kill(child_pid, SIGUSR1);
            printf("Number of signals send to child: %d\n", i+1);
            if(type == 2) pause();
        }
        kill(child_pid, SIGUSR2);
        wait(NULL);
    }
}

void type3(int numOfSignals) {
    pid_t child_pid = fork();
    if (child_pid == 0) {
        sigset_t set;
        setSigset(&set, 3, 1);
//        signal(SIGRTMIN, sigrtminReact);
        struct sigaction act;
        act.sa_handler = sigrtminReact;
        sigfillset(&act.sa_mask);
        sigdelset(&act.sa_mask, SIGINT);
        act.sa_flags = 0;
        sigaction(SIGRTMIN, &act, NULL);
        signal(SIGRTMAX, sigusr2React);
        signal(SIGUSR2, sigusr2React);
        while(1) {};
    } else {
        sleep(1);
        signal(SIGRTMIN, parentSigusrReact);
        for (int i = 0; i < numOfSignals; ++i) {
            kill(child_pid, SIGRTMIN);
            printf("Number of signals send to child: %d\n", i+1);
        }
        kill(child_pid, SIGRTMAX);
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    int numOfSignals = (int)strtol(argv[1], NULL, 10);
    int type = (int)strtol(argv[2], NULL, 10);
    if (type < 1 || type > 3) {
        printf("Wrong type number, it must be 1, 2 or 3\n");
        exit(1);
    }

    sigset_t set;
    setSigset(&set, type, 0);
//    signal(SIGINT, intReact);

    struct sigaction act;
    act.sa_handler = intReact;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    if (type == 1 || type == 2) type12(numOfSignals, type);
    else type3(numOfSignals);

    printf("Ending...\n");
    return 0;
}