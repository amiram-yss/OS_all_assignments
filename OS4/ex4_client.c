// Amiram Yassif 314985474
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <ctype.h>


#define ERR_MESSAGE         "ERROR_FROM_EX4\n"
#define OUTPUT_FILE_NAME    "to_srv"
#define OPEN_ERROR          -1
#define SEC_TO_MILLISEC     1000
#define TIMEOUT_SEC         30
#define EXPECTED_ARGC       5
#define SERVER_ID           argv[1]
#define VAR1                argv[2]
#define OPERATION           argv[3]
#define VAR2                argv[4]

char my_pid_str[7];

bool isnumber(char str[]) {
    int len = strlen(str);
    if (!isdigit(str[0]) && str[0] != '-')
        return false;

    for(int i = 1; i<len; i++) {
        if (!isdigit(str[i]))
            return false;
    }
    return true;
}

void throw_error() {
    printf(ERR_MESSAGE);
    fflush(stdout);
    exit(1);
}

void timeout_handler(int sig) {
    printf("Client closed because no response was received from the server for 30 seconds\n");
    fflush(stdout);
    remove(OUTPUT_FILE_NAME);
    exit(0);
}

void response_handler(int sig) {
    char response_file_name[20] = "to_client_";
    strcat(response_file_name, my_pid_str);
    //
    //system("ls");

    FILE *f_feedback = fopen(response_file_name, "r");
    while (f_feedback == NULL) {
        //exit(0);
        //printf("ATTEMPT\n");
        //fflush(stdout);
        sleep(1);
        f_feedback = fopen(response_file_name, "r");
    }

    char result[32];
    fgets(result, sizeof (result), f_feedback);
    puts(result);
    fflush(stdout);
    fclose(f_feedback);

    //DBG
    //printf("before remove");
    //fflush(stdout);
    remove(response_file_name);

    //printf("after remove");
    //fflush(stdout);

    exit(0);
}

int main(int argc, char *argv[]) {
    //printf("%d", getpid());
    //fflush(stdout);
    //sleep(1);
    signal(SIGALRM, timeout_handler);
    signal(SIGUSR1, response_handler);
    //srand(time(NULL));
    //check params correct
    if (argc != EXPECTED_ARGC)
        throw_error();

    if(!isnumber(SERVER_ID) ||
        !isnumber(VAR1) ||
        !isnumber(OPERATION) ||
        !isnumber(VAR2)) {
        throw_error();
    }

    if (atoi(OPERATION) != 1
        && atoi(OPERATION) != 2
        && atoi(OPERATION) != 3
        && atoi(OPERATION) != 4)
    throw_error();

    // set timer to 60 secs
    alarm(TIMEOUT_SEC);

    //open file. blocks until file doesn't exist
    int f_id = open(OUTPUT_FILE_NAME, O_WRONLY | O_CREAT | O_EXCL, 0777);
    while (f_id == OPEN_ERROR) {
        int sec = rand() % 5 + 1;
        sleep(sec);
        f_id = open(OUTPUT_FILE_NAME, O_WRONLY | O_CREAT | O_EXCL, 0777);
    }
    //write file
    pid_t my_pid = getpid();
    sprintf(my_pid_str, "%d", my_pid);
    write(f_id, my_pid_str, strlen(my_pid_str));
    write(f_id, "\n", 1);
    write(f_id, VAR1, strlen(VAR1));
    write(f_id, "\n", 1);
    write(f_id, OPERATION, strlen(OPERATION));
    write(f_id, "\n", 1);
    write(f_id, VAR2, strlen(VAR2));
    close(f_id);

    //wait for server response.
    kill(atoi(SERVER_ID), SIGUSR1);
    alarm(0);
    pause();


    return 0;
}
