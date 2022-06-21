//Amiram Yassif 314985474
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void calc();

void sigHandle(int sig) {
    alarm(0);
    alarm(60);
    //printf("Received signal!");
    //fflush(stdout);
    pid_t p = fork();
    if (p == 0)
        calc();
    else
        signal(SIGCHLD, SIG_IGN);
}

void timeout_handler(int sig) {
    printf("The server was closed because no service request was received for the last 60 seconds\n");
    fflush(stdout);
    exit(0);
}

int main() {
    alarm(60);
    signal(SIGALRM,timeout_handler);
    signal(SIGUSR1, sigHandle);
    //printf("%d", getpid());
    //fflush(stdout);
    while (true) {
        pause();
    }
}

void calc() {
    char outpid[16];
    FILE *file = fopen("to_srv", "r");
    char line[256];
    int args[3];
    int i = 0;
    fgets(outpid, sizeof (outpid), file);
    //printf("%s", outpid);
    //fflush(stdout);
    while (fgets(line, sizeof(line), file)) {
        args[i] = atoi(line);
    //    puts(line);
    //    fflush(stdout);
        ++i;
    }
    remove("to_srv");
    int result = 0;
    char resMsg[64];
    switch (args[1]) {
        case 1:
            result = args[0] + args[2];
            sprintf(resMsg, "%d", result);
            break;
        case 2:
            result = args[0] - args[2];
            sprintf(resMsg, "%d", result);
            break;
        case 3:
            result = args[0] * args[2];
            sprintf(resMsg, "%d", result);
            break;
        case 4:
            if (args[2] == 0)
                strcpy(resMsg, "CANNOT_DIVIDE_BY_ZERO\n");
            else {
                result = args[0] / args[2];
                sprintf(resMsg, "%d", result);
            }
            break;
    }
    char output_file_name[32];
    strcpy(output_file_name, "to_client_");
    outpid[strlen(outpid) - 1] = '\0';
    strcat(output_file_name, outpid);


    FILE *outfile = fopen(output_file_name, "w");
    fputs(resMsg, outfile);
    fclose(outfile);
    //printf("\n%s\n", outpid);
    //fflush(stdout);
    kill(atoi(outpid), SIGUSR1);
}
